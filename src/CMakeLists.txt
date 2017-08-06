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

ADD_DEFINITIONS(-DVIXEN_OGL)
#ADD_DEFINITIONS(-DVIXEN_OGL -DVX_NOTHREAD)

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

##############################################################
# search paths
##############################################################

LINK_DIRECTORIES(${EXECUTABLE_OUTPUT_PATH})
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

ADD_LIBRARY(VixenGL STATIC
./ogl/appgl.cpp
./ogl/camgl.cpp
./ogl/imagegl.cpp
./ogl/lightgl.cpp
./ogl/meshgl.cpp
./ogl/ocull.cpp
./ogl/scenegl.cpp
./base/array.cpp
./base/bitmap.cpp
./base/box.cpp
./base/bufmess.cpp
./base/event.cpp
./base/fileio.cpp
./base/group.cpp
./base/loadq.cpp
./base/matrix.cpp
./base/messenger.cpp
./base/Obj.cpp
./base/orientbox.cpp
./base/quat.cpp
./base/sphere.cpp
./base/world.cpp
./linux/scenegl_x.cpp
./linux/scene-x.cpp
./linux/world3d_x.cpp
./render/appearance.cpp
./render/fog.cpp
./render/geodraw.cpp
./render/geomesh.cpp
./render/geonorm.cpp
./render/geosort.cpp
./render/image.cpp
./render/loadfont.cpp
./render/material.cpp
./render/sampler.cpp
./render/shader.cpp
./render/textgeom.cpp
./render/trimesh.cpp
./render/vtxaos.cpp
./render/vtxcache.cpp
./render/vtxpool.cpp
./scene/cam.cpp
./scene/distscene.cpp
./scene/extmodel.cpp
./scene/light.cpp
./scene/model.cpp
./scene/octree.cpp
./scene/scene.cpp
./scene/scenethread.cpp
./scene/shape.cpp
./scene/simpleshape.cpp
./scene/sprite.cpp
./scene/world3d.cpp
./sim/deformer.cpp
./sim/computethread.cpp
./sim/engine.cpp
./sim/interpolator.cpp
./sim/keyframe.cpp
./sim/pose.cpp
./sim/skeleton.cpp
./sim/skin.cpp
./sim/trigger.cpp
./sim/xformer.cpp
./util/arcball.cpp
./util/flyer.cpp
./util/framestats.cpp
./util/navigator.cpp
./util/quadtree.cpp
./util/RayPicker.cpp
./util/terrain.cpp
./util/terrcoll.cpp
./util/Trackball.cpp
./util/VCursor.cpp
./vcore/valloc.cpp
./vcore/vbufq.cpp
./vcore/vfile.cpp
./vcore/vobj.cpp
./vcore/vsocket.cpp
./vcore/vstream.cpp
./vcore/vstring.cpp
./vcore/vthread-pt.cpp
./vcore/vtlsdata-pt.cpp
./vcore/vtree.cpp
./vcore/vthread.cpp
./vcore/linux/vdbg-x.cpp
./vcore/linux/vstring-x.cpp
./vcore/linux/vlock-x.cpp
./vcore/linux/vcore-x.cpp
)

TARGET_LINK_LIBRARIES(VixenGL
)

