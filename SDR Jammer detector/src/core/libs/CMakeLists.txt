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


set(CORE_LIBS_SOURCES 
    ini.cc 
    INIReader.cc 
    string_converter.cc
)
	
include_directories(
    $(CMAKE_CURRENT_SOURCE_DIR)
    ${GLOG_INCLUDE_DIRS}
    ${GFlags_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
)

file(GLOB CORE_LIBS_HEADERS "*.h")
list(SORT CORE_LIBS_HEADERS)
add_library(rx_core_lib ${CORE_LIBS_SOURCES} ${CORE_LIBS_HEADERS})
source_group(Headers FILES ${CORE_LIBS_HEADERS})
