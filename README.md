# g4dna-bench

A benchmark application for Geant4-DNA water radiolysis simulation

[![geant4](https://img.shields.io/badge/geant4-10.5-blue.svg)](http://www.geant4.org/)
[![geant4](https://img.shields.io/badge/geant4-10.6-green.svg)](http://www.geant4.org/)
[![geant4](https://img.shields.io/badge/geant4-10.7.beta-orange.svg)](http://www.geant4.org/)

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
2. Copy `con.json` from `config` directory.
3. Run simulation.
```
$ mkdir work
$ cd work
$ cp ../config/conf.json .
$ ../bin/g4dna-bench
```
### Simulaion option

```
$ ../bin/g4dna-bench -h

[Usage] g4dna-banch <options>
[Options]
  -h, --help             print this information
  -c, --conf <file_name> set configuration file [defualt: conf.json]
```

## Configuration file
The configuration of the appilcation is described in `json` file:
```
{
  "random_seed"          : 123456789,
  "event_number"         : 4,
  "thread_number"        : 1,
  "beam_particle"        : "e-",
  "beam_ion_Z"           : 0,                          // atomic number for ions
  "beam_ion_A"           : 0,                          // mass number for ions
  "beam_energy"          : 750,                        // in keV
  "beam_source_pos"      : [0.0, 0.0, 0.0],            // in um
  "beam_direction"       : [0.0, 0.0, 1.0],
  "target_size"          : [20.0, 20.0, 20.0],         // in um
  "primary_removal"      : true,
  "kill_energy"          : [75.0, 75.1],               // in keV
  "phys_list"            : "G4EmDNAPhysics_option8",
  "chem_list"            : "G4EmDNAChemistry_option1",
  "ele_solvation_model"  : "Meesungnoen2002",
  "use_molecule_counter" : false,                      // if true, use G4MoleculeCounter for calculating G-values
  "check_boundary"       : false,
  "output_file"          : "gval.csv",
  "benchmark_file"       : "benchmark.json"
}
```

## Simulation geometry
Target is a cubic water phantom (20 x 20 x 20 um3). Electrons with kinetic energy of 750 keV are shot from the center of the phantom.

![geom](/misc/geom.png)

