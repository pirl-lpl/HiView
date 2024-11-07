# Find Qwt
# ~~~~~~~~
# Copyright (c) 2010, Tim Sutton <tim at linfiniti.com>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
# Once run this will define:
#
# QWT_FOUND       = system has QWT lib
# QWT_LIBRARY     = full path to the QWT library
# QWT_INCLUDE_DIR = where to find headers
#

set(QWT_LIBRARY_NAMES qwt-${QT_VERSION_BASE_LOWER} qwt qwt6 qwt6-${QT_VERSION_BASE_LOWER})

find_library(QWT_LIBRARY
  NAMES ${QWT_LIBRARY_NAMES}
  PATHS
    /usr/lib
    /usr/lib64
    /usr/local/lib
    /usr/local/lib/${QT_VERSION_BASE_LOWER}
    /opt/homebrew/lib
    "$ENV{LIB_DIR}/lib"
    "$ENV{LIB}"
)

set(_qwt_fw)
if(QWT_LIBRARY MATCHES "/qwt.*\\.framework")
  string(REGEX REPLACE "^(.*/qwt.*\\.framework).*$" "\\1" _qwt_fw "${QWT_LIBRARY}")
endif()

find_path(QWT_INCLUDE_DIR NAMES qwt.h PATHS
  "${_qwt_fw}/Headers/"
  /usr/include
  /usr/include/${QT_VERSION_BASE_LOWER}
  /usr/local/include
  /usr/local/include/${QT_VERSION_BASE_LOWER}
  "$ENV{LIB_DIR}/include"
  "$ENV{INCLUDE}"
  PATH_SUFFIXES qwt-${QT_VERSION_BASE_LOWER} ${QT_VERSION_BASE_LOWER}/qwt qwt qwt6
)

if (QWT_INCLUDE_DIR AND QWT_LIBRARY)
  set(QWT_FOUND TRUE)
endif (QWT_INCLUDE_DIR AND QWT_LIBRARY)
