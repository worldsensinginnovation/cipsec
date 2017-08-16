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

#
#set(SDRJD_ENGINE_SOURCES detector_msg_receiver.cc sdrjd.cc)
#
include_directories(
    ${CMAKE_SOURCE_DIR}/src/core/interfaces
    ${CMAKE_SOURCE_DIR}/src/core/receiver
    ${CMAKE_SOURCE_DIR}/src/core/libs
    ${CMAKE_SOURCE_DIR}/src/algorithms/libs
    ${GLOG_INCLUDE_DIRS}
    ${GFlags_INCLUDE_DIRS}
    ${GNURADIO_RUNTIME_INCLUDE_DIRS}
    ${GNURADIO_BLOCKS_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${VOLK_GNSSSDR_INCLUDE_DIRS}
)
#
#file(GLOB SDRJD_ENGINE_HEADERS "*.h")
#list(SORT SDRJD_ENGINE_HEADERS)
#add_library(sdrjd_engine_lib ${SDRJD_ENGINE_SOURCES} ${SDRJD_ENGINE_HEADERS})
#source_group(Headers FILES ${SDRJD_ENGINE_HEADERS})
#
#
#add_dependencies(sdrjd_engine_lib glog-${glog_RELEASE})
#
#target_link_libraries(sdrjd_engine_lib gnss_sp_libs)

add_definitions( -DSDRJD_VERSION="${VERSION}" )
add_definitions( -DGSDRJD_INSTALL_DIR="${CMAKE_INSTALL_PREFIX}" )

add_executable(sdrjd ${CMAKE_CURRENT_SOURCE_DIR}/main.cc)

add_custom_command(TARGET sdrjd POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:sdrjd>
                                   ${CMAKE_SOURCE_DIR}/install/$<TARGET_FILE_NAME:sdrjd>)


target_link_libraries(sdrjd    ${MAC_LIBRARIES}
                                       ${Boost_LIBRARIES}
                                       ${GNURADIO_RUNTIME_LIBRARIES}
                                       ${GNURADIO_BLOCKS_LIBRARIES}
                                       ${GNURADIO_FFT_LIBRARIES}
                                       ${GNURADIO_FILTER_LIBRARIES}
                                       ${GFlags_LIBS}
                                       ${GLOG_LIBRARIES}
                                       ${VOLK_GNSSSDR_LIBRARIES} ${ORC_LIBRARIES}
                                       ${GNSS_SDR_OPTIONAL_LIBS}
                                       rx_core_lib
                                       gnss_rx
                                       gnss_sp_libs
)

                               
install(TARGETS sdrjd
        RUNTIME DESTINATION bin
        COMPONENT "sdrjd"
)

#find_program(GZIP gzip
#                 /bin
#                 /usr/bin
#                 /usr/local/bin
#                 /opt/local/bin
#                 /sbin
#                 )
#
#if(NOT GZIP_NOTFOUND)
#    execute_process(COMMAND gzip -9 -c ${CMAKE_SOURCE_DIR}/docs/manpage/sdrjd-manpage
#                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR} OUTPUT_FILE "${CMAKE_BINARY_DIR}/sdrjd.1.gz")
#
#    install(FILES ${CMAKE_BINARY_DIR}/sdrjd.1.gz DESTINATION share/man/man1)
#endif(NOT GZIP_NOTFOUND)


