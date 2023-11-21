find_package(Qt${QT_MAJOR_VERSION} ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test)

include(ECMMarkAsTest)

macro(ksane_tests)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    target_link_libraries(${_testname} Qt${QT_MAJOR_VERSION}::Test KSaneWidget)
    add_test(ksane-${_testname} ${_testname})
    ecm_mark_as_test(${_testname})
  endforeach(_testname)
endmacro()

#ksane_tests(
#  ksanetest
#)
