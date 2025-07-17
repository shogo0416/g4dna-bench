# g4dna-bench

A benchmark application for Geant4-DNA water radiolysis simulation

[![geant4](https://img.shields.io/badge/geant4-10.5-blue.svg)](https://gitlab.cern.ch/geant4/geant4/-/releases/v10.5.1)
[![geant4](https://img.shields.io/badge/geant4-10.6-blue.svg)](https://gitlab.cern.ch/geant4/geant4/-/releases/v10.6.3)
[![geant4](https://img.shields.io/badge/geant4-10.7-blue.svg)](https://geant4.org/download/10.7.4.html)
[![geant4](https://img.shields.io/badge/geant4-11.0-green.svg)](https://geant4.org/download/11.0.4.html)
[![geant4](https://img.shields.io/badge/geant4-11.1-green.svg)](https://geant4.org/download/11.1.3.html)
[![geant4](https://img.shields.io/badge/geant4-11.2-green.svg)](https://geant4.org/download/11.2.2.html)
[![geant4](https://img.shields.io/badge/geant4-11.3-green.svg)](https://geant4.org/download/11.3.2.html)
[![geant4](https://img.shields.io/badge/geant4-11.4.beta-red.svg)](https://geant4.org/download/11.4.0.beta.html)
[![geant4](https://img.shields.io/badge/geant4-dev-red.svg)](https://gitlab.cern.ch/geant4/geant4-dev)

## Setup Geant4 enviroment variables
Set Geant4 environment variables to specify the paths for data tables before compiling and running the application.

```
$ env grep | G4
G4INCLDATA=/home/shogo/opt/Geant4/data/G4INCL1.0
G4LEVELGAMMADATA=/home/shogo/opt/Geant4/data/PhotonEvaporation5.5
G4RADIOACTIVEDATA=/home/shogo/opt/Geant4/data/RadioactiveDecay5.4
G4PIIDATA=/home/shogo/opt/Geant4/data/G4PII1.3
G4SAIDXSDATA=/home/shogo/opt/Geant4/data/G4SAIDDATA2.0
G4ABLADATA=/home/shogo/opt/Geant4/data/G4ABLA3.1
G4REALSURFACEDATA=/home/shogo/opt/Geant4/data/RealSurface2.1.1
G4NEUTRONHPDATA=/home/shogo/opt/Geant4/data/G4NDL4.6
G4PARTICLEXSDATA=/home/shogo/opt/Geant4/data/G4PARTICLEXS2.1
G4ENSDFSTATEDATA=/home/shogo/opt/Geant4/data/G4ENSDFSTATE2.2
G4LEDATA=/home/shogo/opt/Geant4/data/G4EMLOW7.1
```

## How to install the application
1. Create a `build` directory to compile the application.
2. Move to `build` directory. Then, do `cmake`and `make install`. A binary file `g4dna-bench` will be installed in `bin` directory.

```
$ mkdir build
$ cd build
$ cmake ../
...
$ make install
...
```

## How to run simulation
1. Create `work` directory. Then, move there.
2. Copy `config-IRT.json` from `config` directory.
3. Run simulation.
```
$ mkdir work
$ cd work
$ cp ../config/config-IRT.json .
$ ../bin/chem-bench -c config-IRT.json 2>&1 | tee run.log
```
4. Then, make G-value plot
```
$ cp ../scripts/makeplots.py .
$ ./makeplots.py
$ ls
gval.pdf ...
$ evince gval.pdf
```

### Simulaion option

```
$ ../bin/g4dna-bench -h

    [Usage] chem-bench <options>
    [Options]
      -h, --help              print this information

      -c, --conf   <filename> set configuration file

      -s, --seed   <val>      set seed for random number generation

      -m, --macro  <filename> set macro file

```

## Configuration file
The configuration of the appilcation is described in `json` file:
```
{
  "random_seed"     : 123456789,                            // seed for RNG
  "event_number"    : 10000,                                // event number
  "thread_number"   : 32,                                   // thread number
  "cpu_affinity"    : false,
  "beam_particle"   : "e-",                                 // beam particle kind (e-/proton/alpha/ion)
  "beam_ion_Z"      : 0,                                    // atomic number for ions
  "beam_ion_A"      : 0,                                    // mass number for ions
  "beam_energy"     : 750.0,                                // kinetic energy for beam particle (in keV)
  "beam_source_pos" : [0.0, 0.0, 0.0],                      // source position (in um)
  "beam_direction"  : [0.0, 0.0, 1.0],                      // momentum direction
  "target_size"     : [20.0, 20.0, 20.0],                   // target size (in um)
  "physics_configs" : {
    "physics_option"           : "G4EmDNAPhysics_option8",  // set physics list
    "electron_solvation_model" : "Meesungnoen2002",         // set electron solvation model
    "use_multiple_ionisation"  : false,                     // if true, enable multiple-ionization processes (11.3 or later)
    "primary_removal_configs"  : {
      "enabled"     : true,                                 // if true, enable primary removal
      "kill_energy" : [75.0, 75.1]                          // set kill energy for primary particle (in keV)
    }
  },
  "chemistry_configs": {
    "chemistry_option" : "G4EmDNAChemistry_option3",        // set chemistry list
    "time_step_model"  : "IRT",                             // set tims step model (SBS/IRT/IRT_syn, 11.3 or later)
    "use_alternative_B1A1_decay"   : false,                 // if true, use alternative B1A1 decay chain for excited water molecules (11.3 or later)
    "use_alternative_decay_vibH2O" : false,                 // if true, use alternative B1A1 decay chain for vibrational excited water molecules (11.3 or later)
    "use_g4_molecule_counter"      : false,                 // if true, use G4MoleculeCounter
    "simulation_end_time"          : 1.0E+06                // set simulation end time for chemistry phase (in ps)
  },
  "check_boundary"        : false,
  "output_gval"           : "result_gval.csv",              // time profiles of chemical yield for each molecular species
  "output_LET"            : "result_LET.csv",               // energy deposit (in keV) and LET (in keV/um)
  "benchmark_file"        : "benchmark.json",               // simulation performance
  "benchmark_for_threads" : false,
  "term_frequency"        : 1000
}
```

## Simulation geometry
Target is a cubic water phantom (20 x 20 x 20 um3). Electrons with kinetic energy of 750 keV are shot from the center of the phantom.

![geom](/misc/geom.png)

