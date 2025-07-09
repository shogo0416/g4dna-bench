#!/bin/bash
#===============================================================================
#  BSD 2-Clause License
#
#  Copyright (c) 2021-2025 Shogo OKADA (shogo.okada@kek.jp)
#  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are met:
#
#  1. Redistributions of source code must retain the above copyright notice,
#     this list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce the above copyright notice,
#     this list of conditions and the following disclaimer in the documentation
#     and/or other materials provided with the distribution.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
#  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
#  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
#  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
#  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
#  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
#  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
#  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
#  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
#  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#===============================================================================

# path to top directory
TOPDIR="../../"

# binary name
BIN="bin/chem-bench"

# Geant4 version
G4VERSION="11.3.2"

# physics and chemistry lists
PHYSLIST="G4EmDNAPhysics_option8"
CHEMLIST="DNAChemistryOpt3"

# event number
EVENT_NUM=10000

# thread number
THREAD_NUM=32

# incident energy in MeV
declare -A ENERGY=(
  ["Proton"]="0.2 0.5 1.0 2.0 4.0 10.0 25.0 50.0 75.0 99.9"
  ["Alpha"]="0.5 1.0 3.0 6.0 10.0 20.0 48.0 100.0 200.0 399.9"
  ["Carbon"]="6.5 8.0 10.0 12.0 18.0 24.0 32.0 40.0 48.0 96.0 120.0 240.0 480.0 960.0 1920.0 3840.0 7680.0"
  #["Carbon"]="48.0"
)

# particle label
declare -A PARTICLE_LABEL=(
  ["Proton"]="p"
  ["Alpha"]="Alpha"
  ["Carbon"]="C"
)

# beam particle label
declare -A BEAM_PARTICLE=(
  ["Proton"]="proton"
  ["Alpha"]="alpha"
  ["Carbon"]="ion"
)

# output files
OUTPUT_FILES=("result_gval.csv" "result_LET.csv" "benchmark.json")

# python scripts
PYSCRIPTS_DIR="scripts/"
PYSCRIPTS=("makeplots.py" "compplots.py")

# log files
RUNLOG="run.log"
ERRLOG="err.log"

#-------------------------------------------------------------------------------
make_config_file() {

local config_filename=${1}
local event_num=${2}
local thread_num=${3}
local particle_kind=${4}
local energy=$(echo "${5} * 1000.0" | bc)
local multiple_ioni=${6}

# generate seed for random number generator
local rand_min=1
local rand_max=999999999
local rand_seed=$(python -c \
  "import random, time; random.seed(time.time_ns()); print(random.randint($rand_min, $rand_max))")

local ion_Z=0
local ion_A=0
if [ ${particle_kind} == "Carbon" ]; then
  ion_Z=6
  ion_A=12
fi

cat << EOF > ${config_filename}
{
  "random_seed"     : ${rand_seed},
  "event_number"    : ${event_num},
  "thread_number"   : ${thread_num},
  "cpu_affinity"    : false,
  "beam_particle"   : "${BEAM_PARTICLE[${particle_kind}]}",
  "beam_ion_Z"      : ${ion_Z},
  "beam_ion_A"      : ${ion_A},
  "beam_energy"     : ${energy},
  "beam_source_pos" : [0.0, 0.0, 0.0],
  "beam_direction"  : [0.0, 0.0, 1.0],
  "target_size"     : [2000.0, 2000.0, 2000.0],
  "physics_configs" : {
    "physics_option"           : "${PHYSLIST}",
    "electron_solvation_model" : "Meesungnoen2002",
    "use_multiple_ionisation"  : ${multiple_ioni},
    "primary_removal_configs"  : {
      "enabled"     : true,
      "kill_energy" : [10.0, 10.1]
    }
  },
  "chemistry_configs": {
    "chemistry_option" : "${CHEMLIST}",
    "time_step_model"  : "IRT",
    "use_alternative_B1A1_decay"   : false,
    "use_alternative_decay_vibH2O" : false,
    "use_g4_molecule_counter"      : true,
    "simulation_end_time"          : 1.0E+06
  },
  "check_boundary"        : false,
  "output_gval"           : "result_gval.csv",
  "output_LET"            : "result_LET.csv",
  "benchmark_file"        : "benchmark.json",
  "benchmark_for_threads" : false,
  "term_frequency"        : 1000
}
EOF

}

#-------------------------------------------------------------------------------
filesearch() {
shopt -s nullglob
local pattern="$1"
filelist=($pattern)
echo ${#filelist[@]}
}

#-------------------------------------------------------------------------------
make_plots() {
cd ${1}

local gval=$(filesearch "result_gval*.csv")
local LET=$(filesearch "result_LET*.csv")

if [ "${gval}" -ge 1 ] && [ "${LET}" -ge 1 ]; then
  local cmd="./makeplots.py"
  ${cmd}
fi

cd ../
}

#-------------------------------------------------------------------------------
run_simulation() {

local workdir=${1}
local event_num=${2}
local thread_num=${3}
local particle_kind=${4}
local multiple_ioni=${5}
local show_progress=${6}
local particle_label=${PARTICLE_LABEL[${particle_kind}]}
local energy_list=${ENERGY[${particle_kind}]}

for ekin in ${energy_list[@]}; do

  local outputs=()
  for x in ${OUTPUT_FILES[@]}; do
    outputs+=(${x})
  done

  # make analysis directory
  local anadir="${particle_label}${ekin}MeV"
  if [ ! -d ${anadir} ]; then
    mkdir ${anadir}
  fi

  # copy analysis scripts
  for x in ${PYSCRIPTS[@]}; do
    local fname="${TOPDIR}${PYSCRIPTS_DIR}${x}"
    cp ${fname} ${anadir}
  done

  # make config file
  local config_filename="config_${particle_label}${ekin}MeV.json"
  make_config_file ${config_filename} ${event_num} ${thread_num} \
                   ${particle_kind} ${ekin} ${multiple_ioni}
  outputs+=(${config_filename})

  # run simulation
  local cmd="${TOPDIR}${BIN} -c ${config_filename} "
  echo ${cmd}
  if "${show_progress}"; then
    ${cmd} 2>&1 | tee ${RUNLOG}
  else
    ${cmd} > ${RUNLOG} 2> ${ERRLOG}
  fi
  exit_code=$?

  # move simulation results
  for x in ${outputs[@]}; do
    if [ -f ${x} ]; then
      mv ${x} ${anadir}
    fi
  done

  if [ $exit_code -eq 139 ]; then
    echo "[ERROR] Segmentation fault detected (core dumped)"
  elif [ $exit_code -eq 128 ]; then
    echo "[ERROR] Process terminated by signal: $((exit_code - 128))"
  else
    echo "[MESSAGE] Program exited normally with status $exit_code"
    # make plot
    make_plots ${anadir}
  fi

  mv ${anadir} ${workdir}

done

}

#-------------------------------------------------------------------------------
show_help() {
cat << EOF

Usage: $0 [-h] [-e INTVAL] [-t INTVAL] [-p Proton/Alpha/Carbon]
          [-m true/false] [-s]

Options:
  -h                      show this help

  -e INTVAL               set event number
                          [default: ${EVENT_NUM}]

  -t INTVAL               set thread number
                          [default: ${THREAD_NUM}]

  -p Proton/Alpha/Carbon  set incident particle
                          [default: Carbon]

  -m true/false           enable/disable multiple-ionization process
                          [default: true]

  -s                      show simulation progress
                          [default: false]

Example:
  * run proton simulation without multiple-ionization process
    $0 -p Proton -m false

  * run alpha simulation with multiple-ionization process
    $0 -p Alpha -m true

EOF

}

#===============================================================================
# Main Function
#===============================================================================

particle_kind="Carbon"
event_num=${EVENT_NUM}
thread_num=${THREAD_NUM}
multiple_ioni=true
show_progress=false

while getopts "he:t:p:m:s" opt; do
  case $opt in
    h)
      show_help
      exit 0
      ;;
    e)
      event_num=${OPTARG}
      ;;
    t)
      thread_num=${OPTARG}
      ;;
    p)
      particle_kind="${OPTARG}"
      ;;
    m)
      multiple_ioni=${OPTARG}
      ;;
    s)
      show_progress=true
      ;;
    *)
      show_help
      exit 1
      ;;
  esac
done

# set Geant4 environment
g4major_version=($(echo $G4VERSION | cut -d '.' -f 1))
if [ $g4major_version == 11 ]; then
  source $HOME/setenv-geant4.sh $G4VERSION
else
  source $HOME/setenv-geant4.sh $G4VERSION MT
fi

dir_suffix=""
if "${multiple_ioni}"; then
  echo "[MESSAGE] Multiple-ionization process is enabled."
else
  echo "[MESSAGE] Multiple-ionization process is disabled."
  dir_suffix="_woMIoni"
fi

# create work directory
workdir="${particle_kind}Sim${dir_suffix}"
if [ ! -d ${workdir} ]; then
  mkdir ${workdir}
else
  counter=1
  while true; do
    tmp_workdir="${workdir}_${counter}"
    if [ ! -d ${tmp_workdir} ]; then
      mkdir ${tmp_workdir}
      workdir=${tmp_workdir}
      break
    fi
    counter=$((counter + 1))
  done
fi

echo "[MESSAGE] Make a work directory '${workdir}.'"

echo ${thread_num}

# run simulation
run_simulation ${workdir} ${event_num} ${thread_num} ${particle_kind} \
               ${multiple_ioni} ${show_progress}
