# Copyright (C) 2012-2015  (see AUTHORS file for a list of contributors)
#
# This file is part of GNSS-SDR.
#
# GNSS-SDR is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# GNSS-SDR is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNSS-SDR. If not, see <http://www.gnu.org/licenses/>.
#

# Optional drivers


if(ENABLE_OSMOSDR)
    ################################################################################
    # OsmoSDR - http://sdr.osmocom.org/trac/
    ################################################################################
    find_package(GrOsmoSDR REQUIRED)
    if(NOT GROSMOSDR_FOUND)
        message(FATAL_ERROR "gr-osmosdr required to build gnss-sdr with the optional OSMOSDR driver")
    endif(NOT GROSMOSDR_FOUND)

    # set OSMO include dirs
    set(OSMO_DRIVER_INCLUDE_DIRS
            ${OPT_DRIVER_INCLUDE_DIRS}
            ${GROSMOSDR_INCLUDE_DIR}/osmosdr
            )
    set(OPT_DRIVER_SOURCES ${OPT_DRIVER_SOURCES} osmosdr_signal_source.cc)
    set(OPT_LIBRARIES ${OPT_LIBRARIES} ${GROSMOSDR_LIBRARIES})
    set(OPT_DRIVER_INCLUDE_DIRS ${OPT_DRIVER_INCLUDE_DIRS} ${OSMO_DRIVER_INCLUDE_DIRS})
endif(ENABLE_OSMOSDR)

if(ENABLE_UHD)
    set(OPT_DRIVER_SOURCES ${OPT_DRIVER_SOURCES} uhd_signal_source.cc)
    set(OPT_LIBRARIES ${OPT_LIBRARIES} ${UHD_LIBRARIES} ${GNURADIO_UHD_LIBRARIES})
    set(OPT_DRIVER_INCLUDE_DIRS ${OPT_DRIVER_INCLUDE_DIRS} ${UHD_INCLUDE_DIRS})
endif(ENABLE_UHD)


#set(SIGNAL_SOURCE_ADAPTER_SOURCES file_signal_source.cc
#        ${OPT_DRIVER_SOURCES}
#        )

set(SIGNAL_SOURCE_ADAPTER_SOURCES udp_signal_source.cc file_signal_source.cc
        ${OPT_DRIVER_SOURCES}
        )

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${OPT_DRIVER_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/src/core/system_parameters
        ${CMAKE_SOURCE_DIR}/src/core/interfaces
        ${CMAKE_SOURCE_DIR}/src/algorithms/libs
        ${CMAKE_SOURCE_DIR}/src/algorithms/signal_source/libs
        ${CMAKE_SOURCE_DIR}/src/algorithms/signal_source/gnuradio_blocks
        ${GLOG_INCLUDE_DIRS}
        ${GFlags_INCLUDE_DIRS}
        ${Boost_INCLUDE_DIRS}
        ${GNURADIO_RUNTIME_INCLUDE_DIRS}
        ${VOLK_INCLUDE_DIRS}
)

if(ARCH_64BITS)
    add_definitions(-DARCH_64BITS=1)
endif(ARCH_64BITS)

add_definitions(-DGNSSSDR_INSTALL_DIR="${CMAKE_INSTALL_PREFIX}")

file(GLOB SIGNAL_SOURCE_ADAPTER_HEADERS "*.h")
list(SORT SIGNAL_SOURCE_ADAPTER_HEADERS)
add_library(signal_source_adapters ${SIGNAL_SOURCE_ADAPTER_SOURCES} ${SIGNAL_SOURCE_ADAPTER_HEADERS})
source_group(Headers FILES ${SIGNAL_SOURCE_ADAPTER_HEADERS})
target_link_libraries(signal_source_adapters
        ${Boost_LIBRARIES}
        ${GNURADIO_PMT_LIBRARIES}
        ${GNURADIO_BLOCKS_LIBRARIES}
        ${OPT_LIBRARIES}
        gnss_sp_libs
        )

