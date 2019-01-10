cmake_minimum_required(VERSION 3.1.0)
project(mkbouncer LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")
include(MkUtils)

# Download dependencies
# ---------------------

MkDownloadCatchorgCatch2()
MkDownloadMeasurementKitMkCurl()
MkDownloadMeasurementKitMkMock()
MkDownloadNlohmannJson()
MkDownloadCaBundle()

if("${MSVC}")
  MkDownloadMeasurementKitPrebuiltWindowsCurl()
  list(APPEND CMAKE_INCLUDE_PATH "${MK_WINDOWS_CURL_INCLUDE_PATH}")
  list(APPEND CMAKE_LIBRARY_PATH "${MK_WINDOWS_CURL_LIBRARY_PATH}")
  add_definitions(${MK_WINDOWS_CURL_DEFINITIONS})
  list(APPEND MKBOUNCER_LIBS ${MK_WINDOWS_CURL_EXTRA_LINK_LIBS})
endif()

# Checks
# ------

find_package(CURL REQUIRED)

# Compiler flags
# --------------

MkSetCompilerFlags()

# Library and binary
# ------------------

set(MKBOUNCER_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
    ${CURL_INCLUDE_DIRS} ${CMAKE_REQUIRED_INCLUDES})

add_library(mkbouncer mkbouncer.cpp)
list (APPEND MKBOUNCER_LIBS ${CURL_LIBRARIES})
if("${WIN32}" OR "${MINGW}")
  list(APPEND MKBOUNCER_LIBS "ws2_32")
  if ("${MINGW}")
      list(APPEND MKBOUNCER_LIBS -static-libgcc -static-libstdc++)
  endif()
endif()
list(APPEND MKBOUNCER_LIBS Threads::Threads)

# Testing
# -------

set(BUILD_TESTING "ON" CACHE BOOL "Whether to build tests")
if(${BUILD_TESTING})
  add_executable(integration-tests integration-tests.cpp)
  target_include_directories(integration-tests PUBLIC ${MKBOUNCER_INCLUDES})
  target_link_libraries(integration-tests "${MKBOUNCER_LIBS}" mkbouncer)
  add_executable(tests tests.cpp)
  target_include_directories(tests PUBLIC ${MKBOUNCER_INCLUDES})
  target_link_libraries(tests "${MKBOUNCER_LIBS}")
  enable_testing()
  add_test(NAME integration_tests COMMAND integration-tests)
  add_test(NAME mocked_tests COMMAND tests)
endif()
