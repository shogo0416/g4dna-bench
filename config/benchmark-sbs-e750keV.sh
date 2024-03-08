#!/bin/sh
#===============================================================================
# BSD 2-Clause License
#
# Copyright (c) 2020-2023 Shogo OKADA (shogo.okada@kek.jp)
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
# OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
# EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#===============================================================================

# event number
event_num=500

# job number
job_num=1

# job start id
job_start=0

# seed for random number generator
seed=123456789

# compiler (@FX700)
compiler=""

# Geant4 version number
#g4version="11.1.1"
g4version="10.7.4-mt"

binary="../../bin/chem-bench"

# label for output files
label="e750keV"

# directory name
dirname="e750keV"

# multi-job
multijob=false

#-------------------------------------------------------------------------------
# function to make a configuration file with json format
#-------------------------------------------------------------------------------
make_config_file() {
cat << EOF > config_bench_${label}_no$1.json
{
  "random_seed"           : $2,
  "event_number"          : $3,
  "thread_number"         : 1,
  "cpu_affinity"          : false,
  "beam_particle"         : "e-",
  "beam_ion_Z"            : 0,
  "beam_ion_A"            : 0,
  "beam_energy"           : 750,
  "beam_source_pos"       : [0.0, 0.0, 0.0],
  "beam_direction"        : [0.0, 0.0, 1.0],
  "target_size"           : [20.0, 20.0, 20.0],
  "primary_removal"       : true,
  "kill_energy"           : [75.0, 75.1],
  "phys_list"             : "G4EmDNAPhysics_option8",
  "chem_list"             : "G4EmDNAChemistry_option1",
  "ele_solvation_model"   : "Meesungnoen2002",
  "use_molecule_counter"  : false,
  "check_boundary"        : false,
  "output_file"           : "$4",
  "benchmark_file"        : "$5",
  "benchmark_for_threads" : false,
  "term_frequency"        : $6
}
EOF
}

#-------------------------------------------------------------------------------
# main
#-------------------------------------------------------------------------------
# check the binary setting
if [ ! -f "$binary" ]; then
  echo "[ERROR] Could not find the binary. Check the 'binary' setting."
  exit 1
fi

# set Geant4 environment
G4DIR=/opt/geant4/${compiler}/${g4version}
source $G4DIR/bin/geant4.sh
G4DATA=$(env | grep G4)

# for Mac
if [ "$(uname)" == "Darwin" ]; then
  export DYLD_LIBRARY_PATH=$G4DIR/lib:$DYLD_LIBRARY_PATH
fi

echo "
[MESSAGE] Set environment for Geant4 Version $G4VERSION

* Path to Geant4 Libraries:
$G4DIR

* Data Path:
$G4DATA"

# make directory
if ! "$multijob"; then
  if [ ! -d $dirname ]; then
    mkdir $dirname
  fi
fi

RANDOM=$seed
job_end=$((job_start + job_num))
for ((i=$job_start; i<$job_end; i++)); do

  job_id=$((i + 1))

  rng_seed=$RANDOM
  output_file="result_gval_${label}_no${job_id}.csv"
  benchmark_file="benchmark_${label}_no${job_id}.json"
  term_freq=$((event_num / 10))

  # make a configuration file
  make_config_file ${job_id} $rng_seed $event_num $output_file $benchmark_file $term_freq

  # set a simulation log file
  log_file="run_${label}_no${job_id}.log"

  # run simulation
  command="$binary -c config_bench_${label}_no${job_id}.json"
  echo -e "\n[JOB#${job_id}]"$command
  if "$multijob"; then
    $command > $log_file &
    continue
  else
    $command > $log_file
  fi

  if [ -f $output_file ]; then
    echo "--> Succeeded to run the simulation"
    outputs=($output_file $benchmark_file $log_file)
    for x in ${outputs[@]}
      do
        if [ -f $x ]; then
          mv $x $dirname
        fi
      done
  fi

done

# for Mac
if [ "$(uname)" == "Darwin" ]; then
  unset DYLD_LIBRARY_PATH
fi
