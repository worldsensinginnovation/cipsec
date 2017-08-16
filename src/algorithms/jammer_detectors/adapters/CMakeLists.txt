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


set(JAMMER_DETECTORS_ADAPTER_SOURCES
    pulsed_jammer_detector.cc
    wb_jammer_detector.cc
    cw_jammer_detector.cc
    lfm_jammer_detector.cc
)


include_directories(
     $(CMAKE_CURRENT_SOURCE_DIR)
     ${CMAKE_SOURCE_DIR}/src/core/interfaces
     ${CMAKE_SOURCE_DIR}/src/core/receiver
        ${CMAKE_SOURCE_DIR}/src/algorithms/jammer_detectors/gnuradio_blocks
     ${CMAKE_SOURCE_DIR}/src/algorithms/libs
     ${Boost_INCLUDE_DIRS}
     ${GLOG_INCLUDE_DIRS}
     ${GFlags_INCLUDE_DIRS}
     ${GNURADIO_RUNTIME_INCLUDE_DIRS}
     ${GNURADIO_BLOCKS_INCLUDE_DIRS}
     ${VOLK_GNSSSDR_INCLUDE_DIRS}
)

file(GLOB JAMMER_DETECTORS_ADAPTER_HEADERS "*.h")
list(SORT JAMMER_DETECTORS_ADAPTER_HEADERS)
add_library(jammer_detectors_adapters ${JAMMER_DETECTORS_ADAPTER_SOURCES} ${JAMMER_DETECTORS_ADAPTER_HEADERS})
source_group(Headers FILES ${JAMMER_DETECTORS_ADAPTER_HEADERS})
target_link_libraries(jammer_detectors_adapters gnss_sp_libs jammer_detectors_gr_blocks ${Boost_LIBRARIES} ${GNURADIO_RUNTIME_LIBRARIES} ${GNURADIO_BLOCKS_LIBRARIES})

