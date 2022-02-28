#!/bin/sh
base_event=10000
threads=(1 2 4 6 8 10 12)
g4version="10.7.2"
binary="chem-bench-"${g4version}

# set Geant4 environment
source $HOME/setenv-geant4.sh ${g4version} MT

for i in "${threads[@]}"
do

event_num=$((i * base_event))

cat << EOF > conf_bench.json
{
  "random_seed"           : 123456789,
  "event_number"          : ${event_num},
  "thread_number"         : ${i},
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
  "output_file"           : "gval_${i}mt.csv",
  "benchmark_file"        : "benchmark_${i}mt.json",
  "benchmark_for_threads" : false
}
EOF

log_file="run_"${i}"mt.log"

command="../../../bin/${binary} -c conf_bench.json"
echo "[MT${i}]"${command}

$command >> ${log_file}

done
