#!/usr/bin/env python3
import os

EVENT_NUM_BASE = 10000
THREAD_CONF = [1, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32]
BINARY = '../../../bin/chem-bench-10.7.2'

#-------------------------------------------------------------------------------
template_conf = '''\
{{
  "random_seed"          : 123456789,
  "event_number"         : {EVENT_NUM},
  "thread_number"        : {THREAD_NUM},
  "beam_particle"        : "e-",
  "beam_ion_Z"           : 0,
  "beam_ion_A"           : 0,
  "beam_energy"          : 750,
  "beam_source_pos"      : [0.0, 0.0, 0.0],
  "beam_direction"       : [0.0, 0.0, 1.0],
  "target_size"          : [20.0, 20.0, 20.0],
  "primary_removal"      : true,
  "kill_energy"          : [75.0, 75.1],
  "phys_list"            : "G4EmDNAPhysics_option8",
  "chem_list"            : "G4EmDNAChemistry_option3",
  "ele_solvation_model"  : "Meesungnoen2002",
  "use_molecule_counter" : false,
  "check_boundary"       : false,
  "output_file"          : "{GVAL_FILE}",
  "benchmark_file"       : "{BENCH_FILE}"
}}'''

#-------------------------------------------------------------------------------
def run_sim(event_num, thread_num):

    gval_file  = 'gval_' + str(thread_num) + 'threads.csv'
    bench_file = 'benchmark_' + str(thread_num) + 'threads.json'

    td = dict()
    td['EVENT_NUM']  = event_num
    td['THREAD_NUM'] = thread_num
    td['GVAL_FILE']  = gval_file
    td['BENCH_FILE'] = bench_file

    conf_file = 'conf_bench.json'
    open(conf_file, 'w').write(template_conf.format(**td))

    log_file = 'run_' + str(thread_num) + 'threads.log'

    command = BINARY
    command += ' -c ' + conf_file
    command += ' > ' + log_file
    print(command)

    # run simulation
    os.system(command)


#-------------------------------------------------------------------------------
def main():
    for x in THREAD_CONF:
        run_sim(x * EVENT_NUM_BASE, x)


#===============================================================================
if __name__ == '__main__':
    main()
