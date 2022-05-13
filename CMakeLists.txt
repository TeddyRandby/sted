cmake_minimum_required(VERSION 3.21.3)

project(example C)

include(GNUInstallDirs)

add_library(
  sted SHARED
  src/sted.h
)

set_target_properties(
  sted
  PROPERTIES
  LINKER_LANGUAGE C
  VERSION 1
  SOVERSION 1
  PUBLIC_HEADER src/sted.h
  C_STANDARD 17
)

configure_file(sted.pc.in sted.pc @ONLY)

target_include_directories(sted PUBLIC src)

install(
  TARGETS sted
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
  FILES ${CMAKE_BINARY_DIR}/sted.pc
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig
)

add_executable(example main.c)
target_link_libraries(example sted m)
