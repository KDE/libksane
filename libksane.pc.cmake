prefix=${CMAKE_INSTALL_PREFIX}
exec_prefix=${BIN_INSTALL_DIR}
libdir=${LIB_INSTALL_DIR}
includedir=${KF5_INCLUDE_INSTALL_DIR}

Name: libksane
Description: libksane is a C++ library for SANE interface to control flat scanners.
URL: http://www.digikam.org
Requires:
Version: ${KSANE_LIB_VERSION_STRING}
Libs: -L${LIB_INSTALL_DIR} -lksane
Cflags: -I${KF5_INCLUDE_INSTALL_DIR}
