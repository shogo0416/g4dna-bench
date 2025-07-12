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

#threads=(1 2 4 8 16 32 64 128 160 192 224 256)
#threads=(1 2 4 8 16 20 24 28 32 40 48 56 64)
#threads=(1 2 4 8 16 24 32 40 48 56)
#threads=(1 2 4 8 12 16 20 24 28 32)
threads=(1 2 4 8 16 20 24 28 32)
#threads=(1 2 4 8 14 20 24 28)
#threads=(1 2 4 6 8 10 12)

# Geant4 version
#g4version="11.3.2"
g4version="11.4.0-beta"

# physics and chemistry lists
PHYSLIST="G4EmDNAPhysics_option8"
CHEMLIST="G4EmDNAChemistry_option3"

binary="../../bin/chem-bench"

config_file="config_bench.json"

#-------------------------------------------------------------------------------
# function to make a configuration file with json format
#-------------------------------------------------------------------------------
make_config_file() {

# generate seed for random number generator
local rand_min=1
local rand_max=999999999
local rand_seed=$(python -c \
  "import random, time; random.seed(time.time_ns()); print(random.randint($rand_min, $rand_max))")

cat << EOF > ${config_file}
{
  "random_seed"     : ${rand_seed},
  "event_number"    : ${1},
  "thread_number"   : ${2},
  "cpu_affinity"    : false,
  "beam_particle"   : "e-",
  "beam_ion_Z"      : 0,
  "beam_ion_A"      : 0,
  "beam_energy"     : 750.0,
  "beam_source_pos" : [0.0, 0.0, 0.0],
  "beam_direction"  : [0.0, 0.0, 1.0],
  "target_size"     : [20.0, 20.0, 20.0],
  "physics_configs" : {
    "physics_option"           : "${PHYSLIST}",
    "electron_solvation_model" : "Meesungnoen2002",
    "use_multiple_ionisation"  : false,
    "primary_removal_configs"  : {
      "enabled"     : true,
      "kill_energy" : [75.0, 75.1]
    }
  },
  "chemistry_configs": {
    "chemistry_option" : "${CHEMLIST}",
    "time_step_model"  : "IRT",
    "use_alternative_B1A1_decay"   : false,
    "use_alternative_decay_vibH2O" : false,
    "use_g4_molecule_counter"      : false,
    "simulation_end_time"          : 1.0E+06
  },
  "check_boundary"        : false,
  "output_gval"           : "${3}",
  "output_LET"            : "${4}",
  "benchmark_file"        : "${5}",
  "benchmark_for_threads" : false,
  "term_frequency"        : ${6}
}
EOF
}

#-------------------------------------------------------------------------------
# function to show help
#-------------------------------------------------------------------------------
show_help() {
cat << EOF

Usage: $0 [-h] [-s] [-w] [-f] [-s]

Options:
  -h        show this help

  -e <val>  set base event number
            [default: 10000]

  -w        run simulation with weak-scaling
            [default: strong-scaling]

  -f        fix CPU cores
            [default: false]

  -s        show simulation progress
            [default: false]

Examples:
  * run simulation with strong-scaling
  $0

  * run simulation with weak-scaling
  $0 -w

EOF
}

#===============================================================================
# main function
#===============================================================================
base_event=10000
sim_mode="strong"
fix_core=false
show_progress=false

while getopts "he:wfs" opt; do
  case $opt in
    h)
      show_help
      exit 0
      ;;
    e)
      base_event=$OPTARG
      ;;
    w)
      sim_mode="weak"
      ;;
    f)
      fix_core=true
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
g4major_version=($(echo $g4version | cut -d '.' -f 1))
if [ $g4major_version == 11 ]; then
  source $HOME/setenv-geant4.sh $g4version
else
  source $HOME/setenv-geant4.sh $g4version MT
fi

echo -e "\n[MESSAGE] Benchmark Mode: ${sim_mode}-scaling"

for it in ${threads[@]}; do

  event_num=0
  if [ $sim_mode == "strong" ]; then
    event_num=$base_event
  elif [ $sim_mode == "weak" ]; then
    event_num=$((it * base_event))
  fi

  output_gval="result_gval_${it}mt.csv"
  output_LET="result_LET_${it}mt.csv"
  benchmark_file="benchmark_${it}mt.json"
  term_freq=$((event_num / 10))

  # make a configuration file
  make_config_file ${event_num} ${it} ${output_gval} ${output_LET} ${benchmark_file} ${term_freq}

  # set a simulation log file
  log_file="run_${it}mt.log"

  # run simulation
  command=""
  if "${fix_core}"; then
    upp_core=$((it - 1))
    if [ ${upp_core} -gt 1 ]; then
      command="taskset -c 0-${upp_core} ${binary} -c ${config_file}"
    else
      command="taskset -c 0 ${binary} -c ${config_file}"
    fi
  else
    command="${binary} -c ${config_file}"
  fi
  echo -e "\n[MT${it}] ${command}"
  if "$show_progress"; then
    $command 2>&1 | tee $log_file
  else
    $command > $log_file
  fi

  if [ -f $output_gval ]; then
    echo "--> Succeeded to run the simulation"

    # make a directory to store simulation results
    dirname="sim_${it}mt"
    if "${fix_core}"; then
      dirname="${dirname}_fc"
    fi
    if [ ! -d $dirname ]; then
      mkdir $dirname
    else
      counter=1
      while true; do
        tmp_dirname="${dirname}_${counter}"
        if [ ! -d ${tmp_dirname} ]; then
          mkdir ${tmp_dirname}
          dirname=${tmp_dirname}
          break
        fi
        counter=$((counter + 1))
      done
    fi

    #put simulation results to the directory
    outputs=(${output_gval} ${output_LET} ${benchmark_file} ${log_file} ${config_file})
    for x in ${outputs[@]}
    do
      if [ -f $x ]; then
        mv $x $dirname
      fi
    done

    echo "--> Analysis directory: ${dirname}"

  else

    echo "--> Failed to run the simulation..."

  fi

done
