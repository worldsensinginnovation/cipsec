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


set(GNSS_RECEIVER_SOURCES
        control_message_factory.cc
        file_configuration.cc
        sdr_block_factory.cc
        in_memory_configuration.cc
        jammer_detector_msg.cc
        detector_msg_receiver.cc
        sdrjd.cc
        )


if(PC_GNURADIO_RUNTIME_VERSION VERSION_GREATER 3.7.3)
    add_definitions(-DMODERN_GNURADIO=1)
endif(PC_GNURADIO_RUNTIME_VERSION VERSION_GREATER 3.7.3)


include_directories(
        $(CMAKE_CURRENT_SOURCE_DIR)
        ${CMAKE_SOURCE_DIR}/src/core/interfaces
        ${CMAKE_SOURCE_DIR}/src/core/libs
        ${CMAKE_SOURCE_DIR}/src/algorithms/libs
        ${CMAKE_SOURCE_DIR}/src/algorithms/signal_source/adapters
        ${CMAKE_SOURCE_DIR}/src/algorithms/conditioner/adapters
        ${CMAKE_SOURCE_DIR}/src/algorithms/data_type_adapter/adapters
        ${CMAKE_SOURCE_DIR}/src/algorithms/data_type_adapter/gnuradio_blocks
        ${CMAKE_SOURCE_DIR}/src/algorithms/resampler/adapters
        ${CMAKE_SOURCE_DIR}/src/algorithms/input_filter/adapters
        ${CMAKE_SOURCE_DIR}/src/algorithms/jammer_detectors/adapters
        ${CMAKE_SOURCE_DIR}/src/algorithms/jammer_detectors/gnuradio_blocks
        ${GLOG_INCLUDE_DIRS}
        ${GFlags_INCLUDE_DIRS}
        ${Boost_INCLUDE_DIRS}
        ${GNURADIO_RUNTIME_INCLUDE_DIRS}
        ${OPT_RECEIVER_INCLUDE_DIRS}
        ${VOLK_GNSSSDR_INCLUDE_DIRS}
)

if(Boost_VERSION LESS 105000)
    add_definitions(-DOLD_BOOST=1)
endif(Boost_VERSION LESS 105000)

if(ENABLE_OSMOSDR)
    add_definitions(-DOSMOSDR_DRIVER=1)
endif(ENABLE_OSMOSDR)

if(ENABLE_UHD)
    add_definitions(-DUHD_DRIVER=1)
endif(ENABLE_UHD)


add_definitions(-DGNSSSDR_INSTALL_DIR="${CMAKE_INSTALL_PREFIX}")

file(GLOB GNSS_RECEIVER_HEADERS "*.h")
list(SORT GNSS_RECEIVER_HEADERS)
file(GLOB GNSS_RECEIVER_INTERFACE_HEADERS "../interfaces/*.h")
list(SORT GNSS_RECEIVER_INTERFACE_HEADERS)
add_library(gnss_rx ${GNSS_RECEIVER_SOURCES} ${GNSS_RECEIVER_HEADERS} ${GNSS_RECEIVER_INTERFACE_HEADERS})
source_group(Headers FILES ${GNSS_RECEIVER_HEADERS} ${GNSS_RECEIVER_INTERFACE_HEADERS})

target_link_libraries(gnss_rx ${Boost_LIBRARIES}
        ${GNURADIO_RUNTIME_LIBRARIES}
        ${GNURADIO_BLOCKS_LIBRARIES}
        ${GNURADIO_FFT_LIBRARIES}
        ${GNURADIO_FILTER_LIBRARIES}
        rx_core_lib
        gnss_sp_libs
        signal_source_adapters
        datatype_adapters
        input_filter_adapters
        conditioner_adapters
        resampler_adapters
        jammer_detectors_adapters
        )
