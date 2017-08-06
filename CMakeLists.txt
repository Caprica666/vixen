## ======================================================================== ##
## Copyright 2009-2011 Intel Corporation                                    ##
##                                                                          ##
## Licensed under the Apache License, Version 2.0 (the "License");          ##
## you may not use this file except in compliance with the License.         ##
## You may obtain a copy of the License at                                  ##
##                                                                          ##
##     http://www.apache.org/licenses/LICENSE-2.0                           ##
##                                                                          ##
## Unless required by applicable law or agreed to in writing, software      ##
## distributed under the License is distributed on an "AS IS" BASIS,        ##
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. ##
## See the License for the specific language governing permissions and      ##
## limitations under the License.                                           ##
## ======================================================================== ##

PROJECT(VixenGL)

##############################################################
# configuration
##############################################################

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

IF(COMMAND cmake_policy)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND cmake_policy)

SET(USE_INTEL_COMPILER 1 CACHE BOOL "Set to 1 to use the Intel Compiler")
SET(CMAKE_VERBOSE_MAKEFILE false)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR} ../inc /usr/include/glib-2.0
/usr/lib64/glib-2.0/include)
##############################################################
# output paths
##############################################################

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

##############################################################
# directories to compile
##############################################################

ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(apps/GeoTest)

