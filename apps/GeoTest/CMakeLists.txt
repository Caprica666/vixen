SET(USE_INTEL_COMPILER 1 CACHE BOOL "Set to 1 to use the Intel Compiler")
SET(CMAKE_VERBOSE_MAKEFILE false)

IF(COMMAND cmake_policy)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND cmake_policy)

ADD_DEFINITIONS(-DVIXEN_OGL)

##############################################################
# Compiler
##############################################################

IF (USE_INTEL_COMPILER)

SET (CMAKE_CXX_COMPILER "/opt/intel/composerxe/bin/icpc")
SET (CMAKE_C_COMPILER "/opt/intel/composerxe/bin/icc")
SET (CMAKE_CXX_FLAGS "-fPIC -fstrict-aliasing -fp-model fast ${SSE_FLAGS}")
#SET (CMAKE_CXX_FLAGS "-fPIC -g -D_DEBUG -fstrict-aliasing -fp-model fast ${SSE_FLAGS}")
#-Wall
SET (CMAKE_CXX_FLAGS_DEBUG "-DDEBUG -g -O0")
SET (CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -g -O2")
SET (CMAKE_EXE_LINKER_FLAGS "")

ELSE (USE_INTEL_COMPILER)

SET (CMAKE_CXX_COMPILER "g++")
SET (CMAKE_C_COMPILER "gcc")
SET (CMAKE_CXX_FLAGS "-fPIC -fstrict-aliasing -ffast-math ${SSE_FLAGS}")
#-Wall
SET (CMAKE_CXX_FLAGS_DEBUG "-DDEBUG -g -O0 -ftree-ter")
SET (CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -g -O2")
SET (CMAKE_EXE_LINKER_FLAGS "")
ENDIF (USE_INTEL_COMPILER)

INCLUDE_DIRECTORIES(../../inc)

ADD_EXECUTABLE(geotest geotest)
LINK_DIRECTORIES(../../opt)
TARGET_LINK_LIBRARIES(geotest VixenGL GL GLU glib-2.0 freeimage)


