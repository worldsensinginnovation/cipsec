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


set(JAMMER_DETECTORS_GR_BLOCKS_SOURCES
        pulsed_jammer_detector_cc.cc
        wb_jammer_detector_cc.cc
        cw_jammer_detector_cc.cc
        lfm_jammer_detector_cc.cc
)


include_directories(
     $(CMAKE_CURRENT_SOURCE_DIR)
     ${CMAKE_SOURCE_DIR}/src/core/interfaces
     ${CMAKE_SOURCE_DIR}/src/core/receiver
     ${CMAKE_SOURCE_DIR}/src/algorithms/libs
     ${GLOG_INCLUDE_DIRS}
     ${GFlags_INCLUDE_DIRS}
     ${GNURADIO_RUNTIME_INCLUDE_DIRS}
     ${VOLK_GNSSSDR_INCLUDE_DIRS}
)


## Workaround for issues with the change of VOLK API
## See https://github.com/gnuradio/volk/issues/84
## and https://github.com/gnss-sdr/gnss-sdr/issues/31

if(NOT VOLK_VERSION)
    set(VOLK_VERSION "1.2.2")
endif(NOT VOLK_VERSION)

if(VOLK_VERSION VERSION_EQUAL 1.2.2)
    find_file(VOLK_GREATER_THAN_122
              NAMES volk/volk_32f_index_max_32u.h
              HINTS ${CMAKE_INSTALL_PREFIX}/include
              PATHS /usr/local/include
                    /usr/include
                    ${CMAKE_INSTALL_PREFIX}/include
              )
     if(VOLK_GREATER_THAN_122)
         set(VOLK_VERSION "1.2.3+")
     endif(VOLK_GREATER_THAN_122)
endif(VOLK_VERSION VERSION_EQUAL 1.2.2)

if(VOLK_VERSION VERSION_GREATER 1.2.2)
   add_definitions(-DVOLK_GT_122=1)
endif(VOLK_VERSION VERSION_GREATER 1.2.2)


file(GLOB JAMMER_DETECTORS_GR_BLOCKS_HEADERS "*.h")
list(SORT JAMMER_DETECTORS_GR_BLOCKS_HEADERS)
add_library(jammer_detectors_gr_blocks ${JAMMER_DETECTORS_GR_BLOCKS_SOURCES} ${JAMMER_DETECTORS_GR_BLOCKS_HEADERS})
source_group(Headers FILES ${JAMMER_DETECTORS_GR_BLOCKS_HEADERS})
target_link_libraries(jammer_detectors_gr_blocks gnss_sp_libs ${GNURADIO_RUNTIME_LIBRARIES} ${GNURADIO_FFT_LIBRARIES} ${VOLK_LIBRARIES} ${VOLK_GNSSSDR_LIBRARIES} ${OPT_LIBRARIES})
