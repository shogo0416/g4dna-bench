#!/bin/sh
base_event=10000

#threads=(1 2 4 8 16 32 64 128 160 192 224 256)
#threads=(1 2 4 8 16 20 24 28 32 40 48 56 64)
#threads=(1 2 4 6 8 10 12 14 16 18 20 22 24 26 28 30 32)
#threads=(1 2 4 8 16 20 24 28 32)
threads=(1 2 4 8 14 20 24 28)
#threads=(1 2 4 6 8 10 12)

g4version="11.1.0"

binary="../bin/chem-bench"

#-------------------------------------------------------------------------------
# function to make a configuration file with json format
#-------------------------------------------------------------------------------
make_config_file() {
cat << EOF > conf_bench.json
{
  "random_seed"           : 123456789,
  "event_number"          : $1,
  "thread_number"         : $2,
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
  "output_file"           : "$3",
  "benchmark_file"        : "$4",
  "benchmark_for_threads" : false
}
EOF
}

#-------------------------------------------------------------------------------
# function to show help
#-------------------------------------------------------------------------------
show_help() {
cat << EOF
Usage: benchmark.sh <option>

Options:
  help       show this help
  strong     run simulation with strong-scaling
  weak       run simulation with weak-scaling

EOF
}

#-------------------------------------------------------------------------------
# main
#-------------------------------------------------------------------------------
# set strong-scaling or weak-scaling
sim_mode=""
if [ ${#1} -gt 0 ]; then
  if [ $1 == "help" ]; then
    show_help
    exit 0
  elif [ $1 == "strong" ]; then
    sim_mode="strong"
  elif [ $1 == "weak" ]; then
    sim_mode="weak"
  else
    echo "[ERROR] Unknown option ($1) was set..."
    exit 1
  fi
else
  # set default value
  sim_mode="strong"
fi

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

  output_file="gval_${it}mt.csv"
  benchmark_file="benchmark_${it}mt.json"

  # make a configuration file
  make_config_file $event_num $it $output_file $benchmark_file

  # set a simulation log file
  log_file="run_${it}mt.log"

  counter=0
  while :
  do

    # run simulation
    command="$binary -c conf_bench.json"
    echo -e "\n[MT$it]"$command
    $command > $log_file

    if [ -f $output_file ]; then
      echo "--> Succeeded to run the simulation"

      # make a directory to store simulation results
      dirname="sim_${it}mt"
      if [ ! -d $dirname ]; then
        mkdir $dirname
      fi

      #put simulation results to the directory
      outputs=($output_file $benchmark_file $log_file)
      for x in ${outputs[@]}
      do
        if [ -f $x ]; then
          mv $x $dirname
        fi
      done

      break

    fi

    $((counter++))

    if [ $counter -gt 20 ]; then
      counter=0
      echo "--> Stop the simulation."
      break
    fi

    echo "--> Failed to run the simulation.. Try again."

  done

done
