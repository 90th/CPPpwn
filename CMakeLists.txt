cmake_minimum_required(VERSION 3.30)
project(cpppwn LANGUAGES CXX)

option(CPPPWN_BUILD_EXAMPLE "Build the example program" OFF)


add_library(cpppwn
    src/Remote.cpp
    src/Process.cpp
    src/Server.cpp
    src/Shell.cpp
)

set_target_properties(
  cpppwn PROPERTIES 
    SOVERSION 1 
    VERSION 0.0.1
    CMAKE_CXX_STANDARD 23
    CMAKE_CXX_STANDARD_REQUIRED ON
    CMAKE_CXX_EXTENSIONS OFF
    CMAKE_EXPORT_COMPILE_COMMANDS ON
    CMAKE_CXX_VISIBILITY_PRESET "hidden"
    CMAKE_VISIBILITY_INLINES_HIDDEN "YES"
)

include(cmake/CPM.cmake)
include(GNUInstallDirs)

find_package(Threads REQUIRED)

CPMAddPackage("gh:chriskohlhoff/asio#asio-1-34-2@1.34.2")

# ASIO doesn't use CMake, we have to configure it manually. Extra notes for using on Windows:
#
# 1) If _WIN32_WINNT is not set, ASIO assumes _WIN32_WINNT=0x0501, i.e. Windows XP target, which is
# definitely not the platform which most users target.
#
# 2) WIN32_LEAN_AND_MEAN is defined to make Winsock2 work.
if(asio_ADDED)
  add_library(asio INTERFACE)
  target_include_directories(asio SYSTEM INTERFACE ${asio_SOURCE_DIR}/asio/include)
  target_compile_definitions(asio INTERFACE ASIO_STANDALONE ASIO_NO_DEPRECATED)
  target_link_libraries(asio INTERFACE Threads::Threads)

  if(WIN32)
    # macro see @ https://stackoverflow.com/a/40217291/1746503
    macro(get_win32_winnt version)
      if(CMAKE_SYSTEM_VERSION)
        set(ver ${CMAKE_SYSTEM_VERSION})
        string(REGEX MATCH "^([0-9]+).([0-9])" ver ${ver})
        string(REGEX MATCH "^([0-9]+)" verMajor ${ver})
        # Check for Windows 10, b/c we'll need to convert to hex 'A'.
        if("${verMajor}" MATCHES "10")
          set(verMajor "A")
          string(REGEX REPLACE "^([0-9]+)" ${verMajor} ver ${ver})
        endif("${verMajor}" MATCHES "10")
        # Remove all remaining '.' characters.
        string(REPLACE "." "" ver ${ver})
        # Prepend each digit with a zero.
        string(REGEX REPLACE "([0-9A-Z])" "0\\1" ver ${ver})
        set(${version} "0x${ver}")
      endif()
    endmacro()

    if(NOT DEFINED _WIN32_WINNT)
      get_win32_winnt(ver)
      set(_WIN32_WINNT ${ver})
    endif()

    message(STATUS "Set _WIN32_WINNET=${_WIN32_WINNT}")

    target_compile_definitions(asio INTERFACE _WIN32_WINNT=${_WIN32_WINNT} WIN32_LEAN_AND_MEAN)
  endif()
endif()

target_link_libraries(cpppwn
    PRIVATE Threads::Threads 
    PUBLIC asio
)

include(GenerateExportHeader)
generate_export_header(cpppwn)

target_include_directories(cpppwn
  PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

install(TARGETS cpppwn
  EXPORT cpppwnTargets
  EXPORT DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
  FILES 
    "include/Process.hpp"
    "include/Remote.hpp"
    "include/Server.hpp"
    "include/Shell.hpp"
    "include/Stream.hpp"

  DESTINATION 
    ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT cpppwnTargets
  FILE cpppwnTargets.make 
  NAMESPACE cpppwn::
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/cpppwn"
)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
  cpppwnConfigVersion.cmake 
  VERSION 0.0.1
  COMPATIBILITY SameMajorVersion
)

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in 
  "${CMAKE_CURRENT_BINARY_DIR}/cpppwnConfig.cmake"
  INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/cpppwn"
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/cpppwnConfig.cmake"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/cpppwn"
)

if (CPPPWN_BUILD_EXAMPLE)
  # --------- Example Program
  add_executable(example 
    example/main.cpp
  )
  target_link_libraries(example PRIVATE cpppwn)
endif()
