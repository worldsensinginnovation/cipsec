# Copyright (C) 2012-2016  (see AUTHORS file for a list of contributors)
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

include_directories(
    ${CMAKE_SOURCE_DIR}/src/core/receiver
    ${Boost_INCLUDE_DIRS}
)

add_executable(jammer_msg_rx ${CMAKE_CURRENT_SOURCE_DIR}/sysv_msg_receiver.cc)

target_link_libraries(jammer_msg_rx    ${MAC_LIBRARIES}
                                       ${Boost_LIBRARIES}
                                       gnss_rx
)

add_custom_command(TARGET jammer_msg_rx POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:jammer_msg_rx>
                                   ${CMAKE_SOURCE_DIR}/install/$<TARGET_FILE_NAME:jammer_msg_rx>)



