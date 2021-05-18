# cmake macro to test if we use sane
#
#  SANE_FOUND - system has SANE libs
#  SANE_INCLUDE_DIR - the SANE include directory
#  SANE_LIBRARIES - The libraries needed to use SANE

# SPDX-FileCopyrightText: 2006 Marcus Hufgard <hufgardm@hufgard.de>
#
# SPDX-License-Identifier: BSD-3-Clause

FIND_PATH(SANE_INCLUDE_DIR sane/sane.h)

FIND_LIBRARY(SANE_LIBRARY NAMES  sane libsane
   PATH_SUFFIXES sane
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Sane  DEFAULT_MSG  SANE_LIBRARY SANE_INCLUDE_DIR )

MARK_AS_ADVANCED(SANE_INCLUDE_DIR SANE_LIBRARY)

if(Sane_FOUND AND NOT TARGET Sane::Sane)
    add_library(Sane::Sane UNKNOWN IMPORTED)
    set_target_properties(Sane::Sane PROPERTIES
        IMPORTED_LOCATION "${SANE_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${SANE_INCLUDE_DIR}"
    )
endif()
