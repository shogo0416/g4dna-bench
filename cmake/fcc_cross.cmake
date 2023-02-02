#-------------------------------------------------------------------------------
# fcc_cross.cmake: a cmake toolchain file with the Fujitsu Compiler
#-------------------------------------------------------------------------------
set(FCC_TRAD False CACHE BOOL "FCC Trad Mode")
set(FCC_FAST_MODE False CACHE BOOL "FCC Fast Mode")

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_INCLUDE_PATH /opt/FJSVstclanga/v1.1.0/include)
set(CMAKE_LIBRARY_PATH /opt/FJSVstclanga/v1.1.0/lib64)
set(CMAKE_FIND_ROOT_PATH /opt/FJSVstclanga/v1.1.0)
set(CMAKE_C_COMPILER /opt/FJSVstclanga/v1.1.0/bin/fccpx)
set(CMAKE_CXX_COMPILER /opt/FJSVstclanga/v1.1.0/bin/FCCpx)

#-------------------------------------------------------------------------------
# compile mode
if(FCC_TRAD)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Ntrad")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ntrad")
else()
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Nclang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Nclang")
endif()

#-------------------------------------------------------------------------------
# fast options
if(FCC_FAST_MODE)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Ofast")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")
endif()

#-------------------------------------------------------------------------------
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=a64fx+sve -ffj-ocl -ffj-optlib-string")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mcpu=a64fx+sve -ffj-ocl -ffj-optlib-string")

#-------------------------------------------------------------------------------
set(CMAKE_CXX17_STANDARD_COMPILE_OPTION  "-std=c++17")
set(CMAKE_CXX17_EXTENSION_COMPILE_OPTION "-std=gnu++17")

#-------------------------------------------------------------------------------
list(APPEND CMAKE_CXX_COMPILE_FEATURES
  cxx_std_17
  cxx_alias_templates
  cxx_auto_type
  cxx_delegating_constructors
  cxx_enum_forward_declarations
  cxx_explicit_conversions
  cxx_final
  cxx_lambdas
  cxx_nullptr
  cxx_override
  cxx_range_for
  cxx_strong_enums
  cxx_uniform_initialization
)

#-------------------------------------------------------------------------------
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
