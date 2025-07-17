#-------------------------------------------------------------------------------
# CMakeLists.txt
#-------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

#-------------------------------------------------------------------------------
# Set the name of the main program
set(APP_NAME chem-bench)

#-------------------------------------------------------------------------------
# Set the project name
project(${APP_NAME} CXX)

#-------------------------------------------------------------------------------
# Find Geant4 package
find_package(Geant4 REQUIRED)

#-------------------------------------------------------------------------------
# Set build option
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE "Release")

#-------------------------------------------------------------------------------
# Set Geant4 include directories and compile definitions
include(${Geant4_USE_FILE})

#-------------------------------------------------------------------------------
# cross-compile
if(DEFINED CMAKE_TOOLCHAIN_FILE)
  message("-- Enabled cross-complie with ${CMAKE_CXX_COMPILER_ID}")
  message("-- CXX Flags : ${CMAKE_CXX_FLAGS}")
endif()

#-------------------------------------------------------------------------------
add_executable(${APP_NAME})

target_sources(${APP_NAME} PRIVATE
  ${PROJECT_SOURCE_DIR}/codes/application.cc
  ${PROJECT_SOURCE_DIR}/codes/event_action.cc
  ${PROJECT_SOURCE_DIR}/codes/geometry.cc
  ${PROJECT_SOURCE_DIR}/codes/main.cc
  ${PROJECT_SOURCE_DIR}/codes/molecule_counter.cc
  ${PROJECT_SOURCE_DIR}/codes/physics_list.cc
  ${PROJECT_SOURCE_DIR}/codes/primary_generator.cc
  ${PROJECT_SOURCE_DIR}/codes/primary_removal.cc
  ${PROJECT_SOURCE_DIR}/codes/run_action.cc
  ${PROJECT_SOURCE_DIR}/codes/simdata.cc
  ${PROJECT_SOURCE_DIR}/codes/stacking_action.cc
  ${PROJECT_SOURCE_DIR}/codes/step_action.cc
  ${PROJECT_SOURCE_DIR}/codes/stopwatch.cc
  ${PROJECT_SOURCE_DIR}/codes/timehistory.cc
  ${PROJECT_SOURCE_DIR}/codes/time_step_action.cc
  ${PROJECT_SOURCE_DIR}/codes/dna_chemistry.cc
  ${PROJECT_SOURCE_DIR}/codes/dna_dissociation_channel.cc
)

target_include_directories(${APP_NAME} PRIVATE
  ${PROJECT_SOURCE_DIR}/codes ${Geant4_INCLUDE_DIR})

target_link_libraries(${APP_NAME} PRIVATE ${Geant4_LIBRARIES})

#-------------------------------------------------------------------------------
# Install the executable to 'bin' directory under CMAKE_INSTALL_PREFIX
set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})
install(TARGETS ${APP_NAME} DESTINATION bin)
