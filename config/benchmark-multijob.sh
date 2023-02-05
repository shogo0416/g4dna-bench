#!/bin/sh

# event number
event_num=10000

# job number
job_num=5

# job start id
job_start=0

# seed for random number generator
seed=123456789

# compiler (@FX700)
compiler="gcc-native"

# Geant4 version number
g4version="10.7.4"

binary="../../../bin/chem-bench"

#-------------------------------------------------------------------------------
# function to make a configuration file with json format
#-------------------------------------------------------------------------------
make_config_file() {
cat << EOF > conf_bench_no$1.json
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
  "chem_list"             : "G4EmDNAChemistry_option3",
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

RANDOM=$seed
job_end=$((job_start + job_num))
for ((i=$job_start; i<$job_end; i++)); do

  job_id=$((i + 1))

  rng_seed=$RANDOM
  output_file="gval_no${job_id}.csv"
  benchmark_file="benchmark_no${job_id}.json"
  term_freq=$((event_num / 10))

  # make a configuration file
  make_config_file ${job_id} $rng_seed $event_num $output_file $benchmark_file $term_freq

  # set a simulation log file
  log_file="run_no${job_id}.log"

  # run simulation
  command="$binary -c conf_bench_no${job_id}.json"
  echo -e "\n[JOB#${job_id}]"$command
  $command > $log_file &

done

# for Mac
if [ "$(uname)" == "Darwin" ]; then
  unset DYLD_LIBRARY_PATH
fi
