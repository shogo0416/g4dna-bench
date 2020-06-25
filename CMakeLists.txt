#-------------------------------------------------------------------------------
# CMakeLists.txt
#-------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

#-------------------------------------------------------------------------------
# Set the name of the main program
set(NAME_MAIN_PROGRAM g4dna-bench)

#-------------------------------------------------------------------------------
# Set the project name
project(${NAME_MAIN_PROGRAM} CXX)

#-------------------------------------------------------------------------------
# Find Geant4 package
find_package(Geant4 REQUIRED)

#-------------------------------------------------------------------------------
# Set build option
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE "Release")

#-------------------------------------------------------------------------------
# Set Geant4 include directories and compile definitions
include(${Geant4_USE_FILE})

#-------------------------------------------------------------------------------
# Set include directory for this project
include_directories(${PROJECT_SOURCE_DIR}/codes)

#-------------------------------------------------------------------------------
# Locate sources and headers for this project
# NB: headers are included so they will show up in IDEs
file(GLOB sources ${PROJECT_SOURCE_DIR}/codes/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/codes/*.h
                  ${PROJECT_SOURCE_DIR}/codes/*.hpp)

#-------------------------------------------------------------------------------
# Add the executable, and link it to the Geant4 libraries
add_executable(${NAME_MAIN_PROGRAM}
               ${PROJECT_SOURCE_DIR}/codes/main.cc
               ${sources} ${headers})

#-------------------------------------------------------------------------------
target_link_libraries(${NAME_MAIN_PROGRAM} ${Geant4_LIBRARIES})

#-------------------------------------------------------------------------------
# Install the executable to 'bin' directory under CMAKE_INSTALL_PREFIX
set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})
install(TARGETS ${NAME_MAIN_PROGRAM} DESTINATION bin)
