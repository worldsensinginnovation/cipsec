# Copyright (C) 2016  (see AUTHORS file for a list of contributors)
#
# This file is part of SDRJD.
#
# SDRJD is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# SDRJD is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with SDRJD. If not, see <http://www.gnu.org/licenses/>.
#


########################################################################
# Project setup
########################################################################
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(WARNING "In-tree build is bad practice. Try 'cd build && cmake ../' ")
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
cmake_minimum_required(VERSION 2.8)
project(sdrjd CXX C)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)
file(RELATIVE_PATH RELATIVE_CMAKE_CALL ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})



########################################################################
# Determine optional blocks/libraries to be built (default: not built)
# Enable them here or at the command line by doing 'cmake -DENABLE_XXX=ON ../'
########################################################################

# Support of optional RF front-ends
set(ENABLE_OSMOSDR ON)
option(ENABLE_OSMOSDR "Enable the use of OsmoSDR and other front-ends (RTL-based dongles, HackRF, bladeRF, etc.) as signal source (experimental)" OFF)

# Building and packaging options
option(ENABLE_GENERIC_ARCH "Builds a portable binary" OFF)
option(ENABLE_PACKAGING "Enable software packaging" OFF)
option(ENABLE_OWN_GLOG "Download glog and link it to gflags" OFF)
option(ENABLE_LOG "Enable logging" ON)
if(ENABLE_PACKAGING)
    set(ENABLE_GENERIC_ARCH ON)
endif(ENABLE_PACKAGING)


###############################
# SDRJD version information
###############################
set(THIS_IS_A_RELEASE OFF)   # only related to version name, no further implications.
if(NOT ${THIS_IS_A_RELEASE})
    # Get the current working branch
    execute_process(
        COMMAND git rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    # Get the latest abbreviated commit hash of the working branch
    execute_process(
        COMMAND git log -1 --format=%h
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_COMMIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
endif(NOT ${THIS_IS_A_RELEASE})

set(VERSION_INFO_MAJOR_VERSION 0)
set(VERSION_INFO_API_COMPAT    0)
if(${THIS_IS_A_RELEASE})
    set(VERSION_INFO_MINOR_VERSION 8)
else(${THIS_IS_A_RELEASE})
    set(VERSION_INFO_MINOR_VERSION 8.git-${GIT_BRANCH}-${GIT_COMMIT_HASH})
endif(${THIS_IS_A_RELEASE})

set(VERSION ${VERSION_INFO_MAJOR_VERSION}.${VERSION_INFO_API_COMPAT}.${VERSION_INFO_MINOR_VERSION})


########################################################################
# Environment setup
########################################################################
include(ExternalProject)

# Detect 64-bits machine
if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
     set(ARCH_64BITS TRUE)
endif( CMAKE_SIZEOF_VOID_P EQUAL 8 )

set(OS_IS_MACOSX "")
set(OS_IS_LINUX "")

# Detect Linux Distribution
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
     set(OperatingSystem "Linux")
     set(OS_IS_LINUX TRUE)
     if(ARCH_64BITS)
         set(ARCH_ "(64 bits)")
     else(ARCH_64BITS)
         set(ARCH_ "(32 bits)")
     endif(ARCH_64BITS)
     if(EXISTS "/etc/lsb-release")
         execute_process(COMMAND cat /etc/lsb-release
                         COMMAND grep DISTRIB_ID
                         COMMAND awk -F= "{ print $2 }"
                         COMMAND tr "\n" " "
                         COMMAND sed "s/ //"
                         OUTPUT_VARIABLE LINUX_DISTRIBUTION
                         RESULT_VARIABLE LINUX_ID_RESULT
         )
         execute_process(COMMAND cat /etc/lsb-release
                         COMMAND grep DISTRIB_RELEASE
                         COMMAND awk -F= "{ print $2 }"
                         COMMAND tr "\n" " "
                         COMMAND sed "s/ //"
                         OUTPUT_VARIABLE LINUX_VER
                         RESULT_VARIABLE LINUX_VER_RESULT
         )
     endif(EXISTS "/etc/lsb-release")
     if(NOT LINUX_DISTRIBUTION)
         if(EXISTS "/etc/linuxmint/info")
             set(LINUX_DISTRIBUTION "LinuxMint")
             execute_process(COMMAND cat /etc/linuxmint/info
                             COMMAND grep -m1 RELEASE
                             COMMAND awk -F= "{ print $2 }"
                             COMMAND tr "\n" " "
                             COMMAND sed "s/ //"
                             OUTPUT_VARIABLE LINUX_VER
                             RESULT_VARIABLE LINUX_VER_RESULT
             )
         endif(EXISTS "/etc/linuxmint/info")
     endif(NOT LINUX_DISTRIBUTION)
     if(NOT LINUX_DISTRIBUTION)
         if(EXISTS "/etc/os-release")
             execute_process(COMMAND cat /etc/os-release
                             COMMAND grep -m1 NAME
                             COMMAND awk -F= "{ print $2 }"
                             COMMAND tr "\n" " "
                             COMMAND sed "s/ //"
                             OUTPUT_VARIABLE LINUX_DISTRIBUTION
                             RESULT_VARIABLE LINUX_ID_RESULT
             )
             execute_process(COMMAND cat /etc/os-release
                             COMMAND grep VERSION_ID
                             COMMAND awk -F= "{ print $2 }"
                             COMMAND tr "\n" " "
                             COMMAND sed "s/ //"
                             OUTPUT_VARIABLE LINUX_VER
                             RESULT_VARIABLE LINUX_VER_RESULT
             )
             if(${LINUX_DISTRIBUTION} MATCHES "Debian")
                 set(LINUX_DISTRIBUTION "Debian")
                 file(READ /etc/debian_version LINUX_VER)
             endif(${LINUX_DISTRIBUTION} MATCHES "Debian")
         endif(EXISTS "/etc/os-release")
     endif(NOT LINUX_DISTRIBUTION)
     if(NOT LINUX_DISTRIBUTION)
         if(EXISTS "/etc/redhat-release")
             set(LINUX_DISTRIBUTION "Red Hat")
             file (READ /etc/redhat-release LINUX_VER)
         endif(EXISTS "/etc/redhat-release")
     endif(NOT LINUX_DISTRIBUTION)
     if(NOT LINUX_DISTRIBUTION)
         if(EXISTS "/etc/debian_version")
             set(LINUX_DISTRIBUTION "Debian")
             file (READ /etc/debian_version LINUX_VER)
         endif(EXISTS "/etc/debian_version")
     endif(NOT LINUX_DISTRIBUTION)
     if(NOT LINUX_DISTRIBUTION)
         set(LINUX_DISTRIBUTION "Generic")
         set(LINUX_VER "Unknown")
     endif(NOT LINUX_DISTRIBUTION)
     message(STATUS "Configuring SDRJD v${VERSION} to be built on ${LINUX_DISTRIBUTION} GNU/Linux Release ${LINUX_VER} ${ARCH_}")
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

# Detect Mac OS X Version
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
     set(OperatingSystem "Mac OS X")
     set(OS_IS_MACOSX TRUE)
     exec_program(uname ARGS -v  OUTPUT_VARIABLE DARWIN_VERSION)
     string(REGEX MATCH "[0-9]+" DARWIN_VERSION ${DARWIN_VERSION})
     if(${DARWIN_VERSION} MATCHES "15")
         set(MACOSX_EL_CAPITAN TRUE)
         set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")
         set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
         message(STATUS "Configuring SDRJD v${VERSION} to be built on Mac OS X 10.11 El Capitan")
     endif(${DARWIN_VERSION} MATCHES "15")
     if(${DARWIN_VERSION} MATCHES "14")
         set(MACOSX_YOSEMITE TRUE)
         set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")
         set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
         message(STATUS "Configuring SDRJD v${VERSION} to be built on Mac OS X 10.10 Yosemite")
     endif(${DARWIN_VERSION} MATCHES "14")
     if(${DARWIN_VERSION} MATCHES "13")
         set(MACOSX_MAVERICKS TRUE)
         set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")
         set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
         set(CMAKE_XCODE_ATTRIBUTE_GCC_VERSION="com.apple.compilers.llvm.clang.1_0")
         message(STATUS "Configuring SDRJD v${VERSION} to be built on Mac OS X 10.9 Mavericks")
     endif(${DARWIN_VERSION} MATCHES "13")
     if(${DARWIN_VERSION} MATCHES "12")
         message(STATUS "Configuring SDRJD v${VERSION} to be built on Mac OS X 10.8 Mountain Lion")
     endif(${DARWIN_VERSION} MATCHES "12")
     if(${DARWIN_VERSION} MATCHES "11")
         message(STATUS "Configuring SDRJD v${VERSION} to be built on Mac OS X 10.7 Lion")
     endif(${DARWIN_VERSION} MATCHES "11")
     if(${DARWIN_VERSION} MATCHES "10")
         message(STATUS "Configuring SDRJD v${VERSION} to be built on Mac OS X 10.6 Snow Leopard")
     endif(${DARWIN_VERSION} MATCHES "10")
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")


#select the release build type by default to get optimization flags
if(NOT CMAKE_BUILD_TYPE)
     if(ENABLE_GPERFTOOLS OR ENABLE_GPROF)
         set(CMAKE_BUILD_TYPE "RelWithDebInfo")
         message(STATUS "Build type not specified: defaulting to RelWithDebInfo.")
     else(ENABLE_GPERFTOOLS OR ENABLE_GPROF)
         set(CMAKE_BUILD_TYPE "Release")
         message(STATUS "Build type not specified: defaulting to Release.")
     endif(ENABLE_GPERFTOOLS OR ENABLE_GPROF)
else(NOT CMAKE_BUILD_TYPE)
     message(STATUS "Build type set to ${CMAKE_BUILD_TYPE}.")
endif(NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "")

# Append -O2 optimization flag for Debug builds
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O2")


################################################################################
# Checkout cmake version
################################################################################
if(CMAKE_VERSION VERSION_LESS 2.8.8)
      message(STATUS "Your CMake version is too old and does not support some features required by SDRJD. CMake version must be at least 2.8.8. For more information check https://github.com/joakimkarlsson/bandit/issues/40")
      message(FATAL_ERROR "Fatal error: CMake >= 2.8.8 required.")
endif(CMAKE_VERSION VERSION_LESS 2.8.8)


################################################################################
# Checkout compiler version
################################################################################

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
      if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.7)
          message(STATUS "Your GCC version is too old and does not support some C++11 features required by SDRJD. GCC version must be at least 4.7")
          if(${LINUX_DISTRIBUTION} MATCHES "Ubuntu")
              if(${LINUX_VER} MATCHES "12.04")
                  message(STATUS "For instructions on how to upgrade GCC, check http://askubuntu.com/a/271561")
              endif(${LINUX_VER} MATCHES "12.04")
          endif(${LINUX_DISTRIBUTION} MATCHES "Ubuntu")
          message(FATAL_ERROR "Fatal error: GCC >= 4.7 required.")
      endif(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.7)
endif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")


################################################################################
# Check if the compiler defines the architecture as ARM
################################################################################
if(NOT OS_IS_MACOSX)
     include(TestForARM)
endif(NOT OS_IS_MACOSX)


################################################################################
# Check for availability of SSE
################################################################################
if(NOT ENABLE_GENERIC_ARCH)
     include(TestForSSE)
endif(NOT ENABLE_GENERIC_ARCH)

################################################################################
# Boost - http://www.boost.org
################################################################################
if(UNIX AND EXISTS "/usr/lib64")
    list(APPEND BOOST_LIBRARYDIR "/usr/lib64") # Fedora 64-bit fix
endif(UNIX AND EXISTS "/usr/lib64")
set(Boost_ADDITIONAL_VERSIONS
    "1.45.0" "1.45" "1.46.0" "1.46"                 "1.48.0" "1.48" "1.49.0" "1.49"
    "1.50.0" "1.50" "1.51.0" "1.51"                 "1.53.0" "1.53" "1.54.0" "1.54"
    "1.55.0" "1.55" "1.56.0" "1.56" "1.57.0" "1.57" "1.58.0" "1.58" "1.59.0" "1.59"
    "1.60.0" "1.60" "1.61.0" "1.61" "1.62.0" "1.62" "1.63.0" "1.63" "1.64.0" "1.64"
    "1.65.0" "1.65" "1.66.0" "1.66" "1.67.0" "1.67" "1.68.0" "1.68" "1.69.0" "1.69"
    "1.70.0" "1.70" "1.71.0" "1.71" "1.72.0" "1.72" "1.73.0" "1.73" "1.74.0" "1.74"
)
set(Boost_USE_MULTITHREAD ON)
set(Boost_USE_STATIC_LIBS OFF)
find_package(Boost COMPONENTS date_time system filesystem thread serialization chrono REQUIRED)
if(NOT Boost_FOUND)
     message(FATAL_ERROR "Fatal error: Boost (version >=1.45.0) required.")
endif(NOT Boost_FOUND)


################################################################################
# GNU Radio - http://gnuradio.org
################################################################################
set(GR_REQUIRED_COMPONENTS RUNTIME ANALOG BLOCKS FFT FILTER PMT)
find_package(Gnuradio)
if(PC_GNURADIO_RUNTIME_VERSION)
   if(PC_GNURADIO_RUNTIME_VERSION VERSION_LESS 3.7.3)
       set(GNURADIO_RUNTIME_FOUND)
       message(STATUS "The GNU Radio version installed in your system is too old.")
   endif(PC_GNURADIO_RUNTIME_VERSION VERSION_LESS 3.7.3)
endif(PC_GNURADIO_RUNTIME_VERSION)
if(NOT GNURADIO_RUNTIME_FOUND)
   message(STATUS "CMake cannot find GNU Radio >= 3.7.3")
   if(OS_IS_LINUX)
       message("Go to https://github.com/gnuradio/pybombs")
       message("and follow the instructions to install GNU Radio in your system.")
       message(FATAL_ERROR "GNU Radio 3.7.3 or later is required to build sdrjd")
   endif(OS_IS_LINUX)
   if(OS_IS_MACOSX)
       message("You can install it easily via Macports:")
       message("  sudo port install gnuradio ")
       message("Alternatively, you can use homebrew:")
       message("  brew tap odrisci/gnuradio")
       message("  brew install gnuradio" )
       message(FATAL_ERROR "GNU Radio 3.7.3 or later is required to build sdrjd")
   endif(OS_IS_MACOSX)
endif(NOT GNURADIO_RUNTIME_FOUND)

if(NOT GNURADIO_ANALOG_FOUND)
    message(FATAL_ERROR "*** The gnuradio-analog library v3.7.3 or later is required to build sdrjd")
endif()
if(NOT GNURADIO_BLOCKS_FOUND)
    message(FATAL_ERROR "*** The gnuradio-blocks library v3.7.3 or later is required to build sdrjd")
endif()
if(NOT GNURADIO_FILTER_FOUND)
    message(FATAL_ERROR "*** The gnuradio-filter library v3.7.3 or later is required to build sdrjd")
endif()
if(NOT GNURADIO_FFT_FOUND)
    message(FATAL_ERROR "*** The gnuradio-fft library v3.7.3 or later is required to build sdrjd")
endif()
if(NOT GNURADIO_PMT_FOUND)
    message(FATAL_ERROR "*** The gnuradio-pmt library v3.7.3 or later is required to build sdrjd")
endif()



################################################################################
# VOLK - Vector-Optimized Library of Kernels
################################################################################
find_package(Volk)
if(NOT VOLK_FOUND)
    message(FATAL_ERROR "*** VOLK is required to build sdrjd")
endif()



################################################################################
# volk_gnsssdr module - GNSS-SDR's own VOLK library
################################################################################
find_package(VolkGnssSdr)
if(ENABLE_PACKAGING)
    if(CMAKE_COMPILER_IS_GNUCXX AND NOT WIN32)
        set(STRIP_VOLK_GNSSSDR_PROFILE "-DENABLE_STRIP=ON")
    endif(CMAKE_COMPILER_IS_GNUCXX AND NOT WIN32)
endif(ENABLE_PACKAGING)
set(VOLK_GNSSSDR_BUILD_COMMAND "make")
if(PYTHON_EXECUTABLE)
    set(USE_THIS_PYTHON "-DPYTHON_EXECUTABLE=${PYTHON_EXECUTABLE}")
endif(PYTHON_EXECUTABLE)
if(OS_IS_MACOSX)
    if(NOT PYTHON_EXECUTABLE)
        find_file(MAC_PYTHON python
                PATHS /opt/local/bin
                /usr/local/bin )
        if(MAC_PYTHON)
            set(USE_THIS_PYTHON "-DPYTHON_EXECUTABLE=${MAC_PYTHON}")
        endif(MAC_PYTHON)
    endif(NOT PYTHON_EXECUTABLE)
    if(CMAKE_GENERATOR STREQUAL Xcode)
        set(VOLK_GNSSSDR_BUILD_COMMAND "xcodebuild" "-configuration" "Debug" "-target")
    endif(CMAKE_GENERATOR STREQUAL Xcode)
endif(OS_IS_MACOSX)
if(NOT VOLK_GNSSSDR_FOUND)
    message(STATUS " volk_gnsssdr will be built along with sdrjd when doing 'make'")
    set(CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    set(C_FLAGS "${CMAKE_C_FLAGS} -std=c11")
    set(VOLK_GNSSSDR_CMAKE_ARGS -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
            -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
            -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/volk_gnsssdr_module/install
            -DENABLE_STATIC_LIBS=ON
            -DENABLE_PROFILING=${ENABLE_PROFILING}
            -DCMAKE_CXX_FLAGS=${CXX_FLAGS}
            -DCMAKE_C_FLAGS=${C_FLAGS}
            ${STRIP_VOLK_GNSSSDR_PROFILE}
            ${USE_THIS_PYTHON} )
    if(CMAKE_TOOLCHAIN_FILE)
        set(VOLK_GNSSSDR_CMAKE_ARGS ${VOLK_GNSSSDR_CMAKE_ARGS} -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE})
    endif(CMAKE_TOOLCHAIN_FILE)
    ExternalProject_Add(volk_gnsssdr_module
            PREFIX ${CMAKE_CURRENT_BINARY_DIR}/volk_gnsssdr_module
            SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/algorithms/libs/volk_gnsssdr_module/volk_gnsssdr
            BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/volk_gnsssdr_module/build
            CMAKE_ARGS ${VOLK_GNSSSDR_CMAKE_ARGS}
            DOWNLOAD_COMMAND ""
            UPDATE_COMMAND ""
            PATCH_COMMAND ""
            BUILD_COMMAND ${VOLK_GNSSSDR_BUILD_COMMAND} volk_gnsssdr_profile
            INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/volk_gnsssdr_module/install
            )
    find_package(ORC)
    if(NOT ORC_FOUND)
        set(ORC_LIBRARIES "")
        set(ORC_INCLUDE_DIRS "")
    endif(NOT ORC_FOUND)

    add_library(volk_gnsssdr UNKNOWN IMPORTED)
    set_property(TARGET volk_gnsssdr PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/volk_gnsssdr_module/install/lib/libvolk_gnsssdr.a)
    set(VOLK_GNSSSDR_INCLUDE_DIRS "${CMAKE_CURRENT_BINARY_DIR}/volk_gnsssdr_module/build/include/;${CMAKE_CURRENT_SOURCE_DIR}/src/algorithms/libs/volk_gnsssdr_module/volk_gnsssdr/include;${ORC_INCLUDE_DIRS}")
    set(VOLK_GNSSSDR_LIBRARIES volk_gnsssdr)

    add_custom_command(TARGET volk_gnsssdr_module POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/volk_gnsssdr_module/install/bin/volk_gnsssdr_profile
            ${CMAKE_SOURCE_DIR}/install/volk_gnsssdr_profile)

    add_custom_command(TARGET volk_gnsssdr_module POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/volk_gnsssdr_module/install/bin/volk_gnsssdr-config-info
            ${CMAKE_SOURCE_DIR}/install/volk_gnsssdr-config-info)
endif(NOT VOLK_GNSSSDR_FOUND)


################################################################################
# gflags - https://github.com/gflags/gflags
################################################################################
set(LOCAL_GFLAGS false)
set(gflags_RELEASE 2.1.2)
find_package(GFlags)
if (NOT GFlags_FOUND)
     message (STATUS " gflags library has not been found.")
     message (STATUS " gflags will be downloaded and built automatically ")
     message (STATUS " when doing 'make'. ")

     ExternalProject_Add(
          gflags-${gflags_RELEASE}
          PREFIX ${CMAKE_CURRENT_BINARY_DIR}/gflags-${gflags_RELEASE}
          GIT_REPOSITORY git://github.com/gflags/gflags.git
          GIT_TAG v${gflags_RELEASE}
          SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/gflags/gflags-${gflags_RELEASE}
          BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/gflags-${gflags_RELEASE}
          CMAKE_ARGS -DBUILD_SHARED_LIBS=ON -DBUILD_STATIC_LIBS=ON -DBUILD_gflags_nothreads_LIB=OFF -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
          BUILD_COMMAND make
          UPDATE_COMMAND ""
          PATCH_COMMAND ""
          INSTALL_COMMAND ""
     )

     set(GFlags_INCLUDE_DIRS
          ${CMAKE_CURRENT_BINARY_DIR}/gflags-${gflags_RELEASE}/include CACHE PATH "Local Gflags headers"
     )

     add_library(gflags UNKNOWN IMPORTED)
     set_property(TARGET gflags PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/gflags-${gflags_RELEASE}/lib/${CMAKE_FIND_LIBRARY_PREFIXES}gflags.a)
     add_dependencies(gflags gflags-${gflags_RELEASE})
     set(GFlags_LIBS gflags)
     file(GLOB GFlags_SHARED_LIBS "${CMAKE_CURRENT_BINARY_DIR}/gflags-${gflags_RELEASE}/lib/${CMAKE_FIND_LIBRARY_PREFIXES}gflags${CMAKE_SHARED_LIBRARY_SUFFIX}*")
     set(GFlags_LIBRARY gflags-${gflags_RELEASE})
     set(GFlags_LIBRARY_PATH ${CMAKE_CURRENT_BINARY_DIR}/gflags-${gflags_RELEASE}/lib )
     link_directories(${GFlags_LIBRARY_PATH})
     set(GFlags_lib ${GFlags_LIBS} CACHE FILEPATH "Local Gflags library")
     set(GFlags_LIBRARY_PATH ${GFlags_LIBS})
     set(LOCAL_GFLAGS true CACHE STRING "GFlags downloaded and built automatically" FORCE)
endif(NOT GFlags_FOUND)


################################################################################
# glog - https://github.com/google/glog
################################################################################
if(NOT ${ENABLE_OWN_GLOG})
     find_package(GLOG)
     if(GLOG_INCLUDE_DIRS)
        set(GLOG_FOUND ON)
     endif(GLOG_INCLUDE_DIRS)
endif(NOT ${ENABLE_OWN_GLOG})
set(glog_RELEASE 0.3.4)
if (NOT GLOG_FOUND OR ${LOCAL_GFLAGS})
     message (STATUS " glog library has not been found")
     if(NOT GFlags_FOUND)
          message(STATUS " or it is likely not linked to gflags.")
     endif(NOT GFlags_FOUND)
     message (STATUS " glog will be downloaded and built automatically ")
     message (STATUS " when doing 'make'. ")
     if(NOT ${LOCAL_GFLAGS})
         add_library(gflags-${gflags_RELEASE} UNKNOWN IMPORTED)
         set_property(TARGET gflags-${gflags_RELEASE} PROPERTY IMPORTED_LOCATION "${GFlags_LIBS}")
     endif(NOT ${LOCAL_GFLAGS})
     set(TARGET_GFLAGS gflags-${gflags_RELEASE})
     if(${LOCAL_GFLAGS})
         set(GFLAGS_LIBRARIES_TO_LINK ${GFlags_SHARED_LIBS})
         set(GFLAGS_LIBRARY_DIR_TO_LINK ${CMAKE_CURRENT_BINARY_DIR}/gflags-${gflags_RELEASE}/lib)
     else(${LOCAL_GFLAGS})
         set(GFLAGS_LIBRARIES_TO_LINK ${GFlags_LIBS})
         set(GFLAGS_LIBRARY_DIR_TO_LINK ${GFlags_LIBRARY_DIRS})
     endif(${LOCAL_GFLAGS})

     if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
         file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/glog-${glog_RELEASE}/tmp/configure_with_gflags
"#!/bin/sh
export CPPFLAGS=-I${GFlags_INCLUDE_DIRS}
export LDFLAGS=-L${GFLAGS_LIBRARY_DIR_TO_LINK}
export LIBS=\"${GFLAGS_LIBRARIES_TO_LINK} -lc++\"
export CXXFLAGS=\"-stdlib=libc++\"
export CC=clang
export CXX=clang++
cd ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glog/glog-${glog_RELEASE}/
aclocal
automake --add-missing
autoreconf -vfi
cd ${CMAKE_CURRENT_BINARY_DIR}/glog-${glog_RELEASE}
${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glog/glog-${glog_RELEASE}/configure")

      else("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
             file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/glog-${glog_RELEASE}/tmp/configure_with_gflags
"#!/bin/sh
export CPPFLAGS=-I${GFlags_INCLUDE_DIRS}
export LDFLAGS=-L${GFLAGS_LIBRARY_DIR_TO_LINK}
export LIBS=${GFLAGS_LIBRARIES_TO_LINK}
cd ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glog/glog-${glog_RELEASE}/
aclocal
automake --add-missing
autoreconf -vfi
cd ${CMAKE_CURRENT_BINARY_DIR}/glog-${glog_RELEASE}
${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glog/glog-${glog_RELEASE}/configure")

     endif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")

     file(COPY ${CMAKE_CURRENT_BINARY_DIR}/glog-${glog_RELEASE}/tmp/configure_with_gflags
          DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/glog-${glog_RELEASE}
          FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ
                           GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

     set(GLOG_CONFIGURE ${CMAKE_CURRENT_BINARY_DIR}/glog-${glog_RELEASE}/configure_with_gflags)

     ExternalProject_Add(
         glog-${glog_RELEASE}
         DEPENDS ${TARGET_GFLAGS}
         PREFIX ${CMAKE_CURRENT_BINARY_DIR}/glog-${glog_RELEASE}
         GIT_REPOSITORY https://github.com/google/glog/
         GIT_TAG v${glog_RELEASE}
         SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glog/glog-${glog_RELEASE}
         BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/glog-${glog_RELEASE}
         CONFIGURE_COMMAND ${GLOG_CONFIGURE} --prefix=<INSTALL_DIR>
         BUILD_COMMAND make
         UPDATE_COMMAND ""
         PATCH_COMMAND ""
         INSTALL_COMMAND ""
     )

     # Set up variables
     set(GLOG_INCLUDE_DIRS
          ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glog/glog-${glog_RELEASE}/src/
          ${CMAKE_CURRENT_BINARY_DIR}/glog-${glog_RELEASE}/src
     )
     set(GLOG_LIBRARIES
          ${CMAKE_CURRENT_BINARY_DIR}/glog-${glog_RELEASE}/.libs/${CMAKE_FIND_LIBRARY_PREFIXES}glog.a
     )
     set(LOCAL_GLOG true CACHE STRING "Glog downloaded and built automatically" FORCE)
else(NOT GLOG_FOUND OR ${LOCAL_GFLAGS})
     add_library(glog-${glog_RELEASE} UNKNOWN IMPORTED)
     set_property(TARGET glog-${glog_RELEASE} PROPERTY IMPORTED_LOCATION "${GLOG_LIBRARIES}")
endif(NOT GLOG_FOUND OR ${LOCAL_GFLAGS})

if(NOT ENABLE_LOG)
     message(STATUS "Logging is not enabled")
     add_definitions(-DGOOGLE_STRIP_LOG=1)
endif(NOT ENABLE_LOG)


################################################################################
# USRP Hardware Driver (UHD) - OPTIONAL
################################################################################
find_package(UHD)
if(NOT UHD_FOUND)
    set(ENABLE_UHD OFF)
    message(STATUS " The USRP Hardware Driver (UHD) signal source will not be built,")
    message(STATUS " so all USRP-based front-ends will not be usable.")
    message(STATUS " Please check http://files.ettus.com/manual/")
else(NOT UHD_FOUND)
    set(GR_REQUIRED_COMPONENTS UHD)
    find_package(Gnuradio)
    set(ENABLE_UHD ON)
endif(NOT UHD_FOUND)


################################################################################
# Doxygen - http://www.stack.nl/~dimitri/doxygen/index.html (OPTIONAL, used if found)
################################################################################
find_package(Doxygen)
if(DOXYGEN_FOUND)
     message(STATUS "Doxygen found.")
     message(STATUS "You can build the documentation with 'make doc'." )
     message(STATUS "When done, point your browser to ${CMAKE_BINARY_DIR}/html/index.html")
     set(HAVE_DOT ${DOXYGEN_DOT_FOUND})
     file(TO_NATIVE_PATH ${CMAKE_SOURCE_DIR} top_srcdir)
     file(TO_NATIVE_PATH ${CMAKE_BINARY_DIR} top_builddir)
     find_package(LATEX)
     if (PDFLATEX_COMPILER)
          set(GENERATE_PDF_DOCUMENTATION "YES")
          set(SDRJD_USE_MATHJAX "NO")
     else(PDFLATEX_COMPILER)
          set(GENERATE_PDF_DOCUMENTATION "NO")
          set(SDRJD_USE_MATHJAX "YES")
     endif(PDFLATEX_COMPILER)
     configure_file(${CMAKE_SOURCE_DIR}/docs/doxygen/Doxyfile.in
          ${CMAKE_BINARY_DIR}/docs/doxygen/Doxyfile
          @ONLY
     )
     add_custom_target(doc
          ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/docs/doxygen/Doxyfile
          WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
          COMMENT "Generating API documentation with Doxygen." VERBATIM
     )
     if(LATEX_COMPILER)
          message(STATUS "'make pdfmanual' will generate a manual at ${CMAKE_BINARY_DIR}/docs/SDRJD_manual.pdf")
          add_custom_target(pdfmanual
               COMMAND ${CMAKE_MAKE_PROGRAM}
               COMMAND ${CMAKE_COMMAND} -E copy refman.pdf ${CMAKE_BINARY_DIR}/docs/SDRJD_manual.pdf
               COMMAND ${CMAKE_MAKE_PROGRAM} clean
               DEPENDS doc
               WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/docs/latex
               COMMENT "Generating PDF manual with Doxygen." VERBATIM
          )
     endif(LATEX_COMPILER)
     message(STATUS "'make doc-clean' will clean the documentation.")
     add_custom_target(doc-clean
          COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/docs/html
          COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/docs/latex
          COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/docs/SDRJD_manual.pdf
          COMMENT "Cleaning documentation." VERBATIM
     )
else(DOXYGEN_FOUND)
     message(STATUS " Doxygen has not been found in your system.")
     message(STATUS " You can get nice code documentation by using it!")
     message(STATUS " Get it from http://www.stack.nl/~dimitri/doxygen/index.html")
     if(OS_IS_LINUX)
         if(${LINUX_DISTRIBUTION} MATCHES "Fedora" OR ${LINUX_DISTRIBUTION} MATCHES "Red Hat")
             message(" or simply by doing 'sudo yum install doxygen-latex'.")
         else(${LINUX_DISTRIBUTION} MATCHES "Fedora" OR ${LINUX_DISTRIBUTION} MATCHES "Red Hat")
             message(" or simply by doing 'sudo apt-get install doxygen-latex'.")
         endif(${LINUX_DISTRIBUTION} MATCHES "Fedora" OR ${LINUX_DISTRIBUTION} MATCHES "Red Hat")
     endif(OS_IS_LINUX)
     if(OS_IS_MACOSX)
         message(STATUS " or simply by doing 'sudo port install doxygen +latex'.")
     endif(OS_IS_MACOSX)
endif(DOXYGEN_FOUND)



################################################################################
# Setup of optional drivers
################################################################################
set(ENABLE_OSMOSDR ON)
if(ENABLE_OSMOSDR)
    message(STATUS "The driver for OsmoSDR and other front-ends (HackRF, bladeRF, Realtek's RTL2832U-based dongles, etc.) will be compiled." )
    message(STATUS "You can disable it with 'cmake -DENABLE_OSMOSDR=OFF ../'" )
else(ENABLE_OSMOSDR)
    message(STATUS "The (optional) driver for OsmoSDR and related front-ends is not enabled." )
    message(STATUS "Enable it with 'cmake -DENABLE_OSMOSDR=ON ../' to add support for OsmoSDR and other front-ends (HackRF, bladeRF, Realtek's RTL2832U-based USB dongles, etc.)" )
endif(ENABLE_OSMOSDR)

########################################################################
# Set compiler flags
########################################################################

# Enable C++11 support in GCC
# For "-std=c++0x"  GCC's support for C++11 see http://gcc.gnu.org/projects/cxx0x.html
if(CMAKE_COMPILER_IS_GNUCXX AND NOT WIN32)
     set(MY_CXX_FLAGS "${MY_CXX_FLAGS} -std=c++11 -Wall -Wextra")  #Add warning flags: For "-Wall" see http://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html
endif(CMAKE_COMPILER_IS_GNUCXX AND NOT WIN32)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(MY_CXX_FLAGS "${MY_CXX_FLAGS} -std=c++11 -stdlib=libc++ -Wno-c++11-narrowing")
    if(CMAKE_BUILD_TYPE MATCHES "Release")
        set(MY_CXX_FLAGS "${MY_CXX_FLAGS} -Wno-unused-private-field")
    endif(CMAKE_BUILD_TYPE MATCHES "Release")
endif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")

# Processor-architecture related flags
# See http://gcc.gnu.org/onlinedocs/gcc/i386-and-x86_002d64-Options.html#i386-and-x86_002d64-Options
if (NOT ARCH_COMPILER_FLAGS)
  if(CMAKE_COMPILER_IS_GNUCXX AND NOT WIN32)
    if(OS_IS_MACOSX)
      set(ARCH_COMPILER_FLAGS "-march=corei7 -mfpmath=sse")
    else(OS_IS_MACOSX)
      if(NOT ${ENABLE_GENERIC_ARCH})
         if(IS_ARM) # ARM-specific options (https://gcc.gnu.org/onlinedocs/gcc/ARM-Options.html)
            if(ARM_VERSION STREQUAL "arm")
               # Unknown arm version - try our best to detect
               set(ARCH_COMPILER_FLAGS "-mcpu=native")
            else(ARM_VERSION STREQUAL "arm")
               set(ARCH_COMPILER_FLAGS "-march=${ARM_VERSION}")
	        endif(ARM_VERSION STREQUAL "arm")
         else(IS_ARM)
            set(ARCH_COMPILER_FLAGS "-march=native -mfpmath=sse")
	     endif(IS_ARM)
      endif(NOT ${ENABLE_GENERIC_ARCH})
    endif(OS_IS_MACOSX)
  endif(CMAKE_COMPILER_IS_GNUCXX AND NOT WIN32)
endif (NOT ARCH_COMPILER_FLAGS)
set(MY_CXX_FLAGS "${MY_CXX_FLAGS} ${ARCH_COMPILER_FLAGS}")

if(CMAKE_COMPILER_IS_GNUCXX AND NOT WIN32)
    #http://gcc.gnu.org/wiki/Visibility
    add_definitions(-fvisibility=hidden)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MY_CXX_FLAGS}")

if(OS_IS_LINUX)
    if(${LINUX_DISTRIBUTION} MATCHES "Fedora" OR ${LINUX_DISTRIBUTION} MATCHES "openSUSE" OR ${LINUX_DISTRIBUTION} MATCHES "ArchLinux")
        link_libraries(pthread)
    endif(${LINUX_DISTRIBUTION} MATCHES "Fedora" OR ${LINUX_DISTRIBUTION} MATCHES "openSUSE" OR ${LINUX_DISTRIBUTION} MATCHES "ArchLinux")
endif(OS_IS_LINUX)


########################################################################
# Create uninstall target
########################################################################
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
    @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

########################################################################
# Add subdirectories (in order of deps)
########################################################################
add_subdirectory(src)
