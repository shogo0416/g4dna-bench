# g4dna-bench

[![geant4](https://img.shields.io/badge/geant4-10.5-blue.svg)](http://www.geant4.org/)
[![geant4](https://img.shields.io/badge/geant4-10.6-green.svg)](http://www.geant4.org/)

## Before starting...
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
2. Move to `build` directory. Then, do `cmake`and `make install`.

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
2. Copy `con.json` from `script` directory.

```
$ mkdir work
$ cd work
$ cp ../scripts/conf.json .
$ ../bin/g4dna-bench
```
