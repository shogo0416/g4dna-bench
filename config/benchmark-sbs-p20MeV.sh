#!/bin/sh

# event number
event_num=5000

# job number
job_num=30

# job start id
job_start=0

# seed for random number generator
seed=123456789

# compiler (@FX700)
compiler=""

# Geant4 version number
g4version="11.1.1"

binary="../../bin/chem-bench"

# directory name
dirname="p20MeV"

# multi-job
multijob=false

#-------------------------------------------------------------------------------
# function to make a configuration file with json format
#-------------------------------------------------------------------------------
make_config_file() {
cat << EOF > config_p20MeV_no$1.json
{
  "random_seed"           : $2,
  "event_number"          : $3,
  "thread_number"         : 1,
  "cpu_affinity"          : false,
  "beam_particle"         : "proton",
  "beam_ion_Z"            : 0,
  "beam_ion_A"            : 0,
  "beam_energy"           : 20000,
  "beam_source_pos"       : [0.0, 0.0, 0.0],
  "beam_direction"        : [0.0, 0.0, 1.0],
  "target_size"           : [20.0, 20.0, 20.0],
  "primary_removal"       : true,
  "kill_energy"           : [10.0, 10.1],
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
if ! "$multijob" ; then
  if [ ! -d $dirname ]; then
    mkdir $dirname
  fi
fi

RANDOM=$seed
job_end=$((job_start + job_num))
for ((i=$job_start; i<$job_end; i++)); do

  job_id=$((i + 1))

  rng_seed=$RANDOM
  output_file="gval_p20MeV_no${job_id}.csv"
  benchmark_file="benchmark_p20MeV_no${job_id}.json"
  term_freq=$((event_num / 10))

  # make a configuration file
  make_config_file ${job_id} $rng_seed $event_num $output_file $benchmark_file $term_freq

  # set a simulation log file
  log_file="run_p20MeV_no${job_id}.log"

  # run simulation
  command="$binary -c config_p20MeV_no${job_id}.json"
  echo -e "\n[JOB#${job_id}]"$command
  if "$multijob" ; then
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
