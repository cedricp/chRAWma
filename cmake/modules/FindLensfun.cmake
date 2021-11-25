# - Find the native lensfun includes and library
#
# This module defines
#  LENSFUN_INCLUDE_DIR, where to find lensfun.h, etc.
#  LENSFUN_LIBRARIES, the libraries to link against to use lensfun.
#  LENSFUN_FOUND, If false, do not try to use lensfun.
# also defined, but not for general use are
#  LENSFUN_LIBRARY, where to find the lensfun library.

find_path(LENSFUN_INCLUDE_DIR lensfun/lensfun.h)
mark_as_advanced(LENSFUN_INCLUDE_DIR)
 
set(LENSFUN_NAMES ${LENSFUN_NAMES} lensfun liblensfun)
find_library(LENSFUN_LIBRARY NAMES ${LENSFUN_NAMES})
mark_as_advanced(LENSFUN_LIBRARY)
 
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LENSFUN DEFAULT_MSG LENSFUN_LIBRARY LENSFUN_INCLUDE_DIR)
 
IF(LENSFUN_FOUND)
  SET(LensFun_LIBRARIES ${LENSFUN_LIBRARY})
  SET(LensFun_INCLUDE_DIRS ${LENSFUN_INCLUDE_DIR})
ENDIF(LENSFUN_FOUND)