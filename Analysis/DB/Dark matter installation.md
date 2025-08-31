```c

[snuintern1@cms2 DMG4]$ source /cvmfs/sft.cern.ch/lcg/views/LCG_102/x86_64-centos7-gcc8-opt/setup.sh

[snuintern1@cms2 DMG4]$ ./runconfigure_lcg104_lxplus

The package will be installed in PREFIX = .

The compilation mode (see CMakeLists.txt for definitions) is: Release

  

[snuintern1@cms2 DMG4]$ make

Scanning dependencies of target UtilsDM

[  2%] Building CXX object CMakeFiles/UtilsDM.dir/src/UtilsDM/DarkMatterParametersRegistry.cc.o

  

[snuintern1@cms2 DMG4]$ make install

[  9%] Built target UtilsDM

  

[snuintern1@cms2 DMG4]$ cd examples/

[snuintern1@cms2 examples]$ ls

CMakeLists.txt  configure  example1022  testAnnihilationCS

clean           example1   example11    testCS

[snuintern1@cms2 examples]$ cd example1

[snuintern1@cms2 example1]$ ls

CMakeCache.txt  Makefile  cmake_install.cmake  config.sh      mkgeant4.cc

CMakeFiles      README    config.csh           histresults.C  runconfigure

[snuintern1@cms2 example1]$ make clean

[snuintern1@cms2 example1]$ ./runconfigure

  

configuring with the following package locations taken from ../../config.txt :

  

  

[snuintern1@cms2 example1]$ ls

CMakeCache.txt  README               config.sh      mkgeant4.cc

CMakeFiles      cmake_install.cmake  histresults.C  runconfigure

Makefile        config.csh           mkgeant4

[snuintern1@cms2 example1]$ ./mkgeant4

Initialized DarkPhotons off electrons and positrons for material density = 11.35

  

Total CS calc, E = 200  M = 0.017  KFactor = 1.14704 CS = 3551.46

 E0 = 200  Max cross section = 7.63055e+08

  

Test of the DarkMatter package: DM emission simulation, energy = 100 GeV, mass = 0.017 GeV

  

  

Emission simulated, X = 0.978526 Theta = 2.3562e-05

  

Cross section in pb for eps=0.0001 cs = 3501.27
```
