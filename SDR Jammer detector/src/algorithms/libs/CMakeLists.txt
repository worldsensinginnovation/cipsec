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


set(GNSS_SPLIBS_SOURCES
    pass_through.cc
    complex_byte_to_float_x2.cc
    byte_x2_to_complex_byte.cc
    cshort_to_float_x2.cc
    short_x2_to_cshort.cc
    complex_float_to_complex_byte.cc
    sdr_signal_processing.cc
        new_valve.cc
)


include_directories(
     $(CMAKE_CURRENT_SOURCE_DIR)
     ${CMAKE_SOURCE_DIR}/src/core/system_parameters
     ${CMAKE_SOURCE_DIR}/src/core/receiver
     ${CMAKE_SOURCE_DIR}/src/core/interfaces
     ${Boost_INCLUDE_DIRS}
     ${GLOG_INCLUDE_DIRS}
     ${GFlags_INCLUDE_DIRS}
     ${GNURADIO_RUNTIME_INCLUDE_DIRS}
     ${GNURADIO_BLOCKS_INCLUDE_DIRS}
     ${VOLK_INCLUDE_DIRS}
     ${VOLK_GNSSSDR_INCLUDE_DIRS}
)


file(GLOB GNSS_SPLIBS_HEADERS "*.h")
list(SORT GNSS_SPLIBS_HEADERS)
add_library(gnss_sp_libs ${GNSS_SPLIBS_SOURCES} ${GNSS_SPLIBS_HEADERS})
source_group(Headers FILES ${GNSS_SPLIBS_HEADERS})

target_link_libraries(gnss_sp_libs
        ${GNURADIO_RUNTIME_LIBRARIES}
        ${VOLK_LIBRARIES} ${ORC_LIBRARIES}
        ${VOLK_GNSSSDR_LIBRARIES}
        ${GNURADIO_BLOCKS_LIBRARIES}
        ${GNURADIO_FFT_LIBRARIES}
        ${GNURADIO_FILTER_LIBRARIES}
        ${OPT_LIBRARIES}
        gnss_rx
        rx_core_lib
)

if(NOT VOLK_GNSSSDR_FOUND)
    add_dependencies(gnss_sp_libs volk_gnsssdr_module)
endif(NOT VOLK_GNSSSDR_FOUND)
