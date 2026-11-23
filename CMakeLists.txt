# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.16)
project(EigenQt VERSION 0.1.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(GenerateExportHeader)
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(EIGENQT_BUILD_TESTS "Build tests" OFF)
option(EIGENQT_BUILD_EXAMPLES "Build the widget gallery" OFF)
set(EIGENQT_QT_VERSION "" CACHE STRING "Qt major version (5 or 6); empty selects automatically")
if(NOT EIGENQT_QT_VERSION)
  find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
  set(EIGENQT_QT_VERSION ${QT_VERSION_MAJOR})
endif()
find_package(Qt${EIGENQT_QT_VERSION} REQUIRED COMPONENTS Widgets)
find_package(Eigen3 3.4 REQUIRED NO_MODULE)
set(CMAKE_AUTOMOC ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

function(eigenqt_library component)
  string(TOLOWER ${component} lower)
  add_library(eigenqt${component} ${ARGN})
  add_library(EigenQt::${component} ALIAS eigenqt${component})
  set_target_properties(eigenqt${component} PROPERTIES
    EXPORT_NAME ${component} VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR}
    POSITION_INDEPENDENT_CODE ON)
  if(APPLE)
    set_property(TARGET eigenqt${component} PROPERTY INSTALL_RPATH "@loader_path")
  elseif(UNIX)
    set_property(TARGET eigenqt${component} PROPERTY INSTALL_RPATH "$ORIGIN")
  endif()
  target_compile_features(eigenqt${component} PUBLIC cxx_std_17)
  if(WIN32)
    target_compile_definitions(eigenqt${component} PUBLIC NOMINMAX)
  endif()
  target_include_directories(eigenqt${component} PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
  string(TOUPPER ${component} upper)
  generate_export_header(eigenqt${component}
    EXPORT_FILE_NAME include/eigenqt/${lower}_export.h
    EXPORT_MACRO_NAME EIGENQT_${upper}_EXPORT
    STATIC_DEFINE EIGENQT_${upper}_STATIC_DEFINE)
  if(NOT BUILD_SHARED_LIBS)
    target_compile_definitions(eigenqt${component} PUBLIC EIGENQT_${upper}_STATIC_DEFINE)
  endif()
  target_link_libraries(eigenqt${component} PUBLIC Qt${EIGENQT_QT_VERSION}::Widgets Eigen3::Eigen)
  install(TARGETS eigenqt${component} EXPORT EigenQt${component}Targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
  install(FILES ${PROJECT_BINARY_DIR}/include/eigenqt/${lower}_export.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/eigenqt)
  install(EXPORT EigenQt${component}Targets NAMESPACE EigenQt::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/EigenQt)
endfunction()

eigenqt_library(Widgets src/NumericTableWidget.cpp src/VectorWidget.cpp src/MatrixWidget.cpp
  include/eigenqt/NumericTableWidget.h include/eigenqt/VectorWidget.h include/eigenqt/MatrixWidget.h)

find_package(OpenGL REQUIRED)
if(EIGENQT_QT_VERSION STREQUAL "6")
  find_package(Qt6 REQUIRED COMPONENTS OpenGLWidgets)
  set(eigenqt_gl_target Qt6::OpenGLWidgets)
else()
  find_package(Qt5 REQUIRED COMPONENTS OpenGL)
  set(eigenqt_gl_target Qt5::OpenGL)
endif()
eigenqt_library(Plot src/Plot2D.cpp src/Plot2DOpenGL.cpp src/Plot2DWidget.cpp
  include/eigenqt/Plot2DWidget.h)
target_link_libraries(eigenqtPlot PUBLIC eigenqtWidgets Eigen3::Eigen ${eigenqt_gl_target} OpenGL::GL)

eigenqt_library(Geometry src/RotationWidget.cpp src/FrameWidget.cpp src/Vector3DWidget.cpp
  src/PositionsWidget.cpp src/WrenchWidget.cpp src/WrenchPlotWidget.cpp src/Wrench3DWidget.cpp
  include/eigenqt/RotationWidget.h include/eigenqt/FrameWidget.h include/eigenqt/Vector3DWidget.h
  include/eigenqt/PositionsWidget.h include/eigenqt/WrenchWidget.h
  include/eigenqt/WrenchPlotWidget.h include/eigenqt/Wrench3DWidget.h)
target_link_libraries(eigenqtGeometry PUBLIC eigenqtPlot)

configure_package_config_file(cmake/EigenQtConfig.cmake.in
  ${PROJECT_BINARY_DIR}/EigenQtConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/EigenQt)
write_basic_package_version_file(${PROJECT_BINARY_DIR}/EigenQtConfigVersion.cmake
  VERSION ${PROJECT_VERSION} COMPATIBILITY SameMajorVersion)
install(FILES ${PROJECT_BINARY_DIR}/EigenQtConfig.cmake
  ${PROJECT_BINARY_DIR}/EigenQtConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/EigenQt)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES LICENSE DESTINATION ${CMAKE_INSTALL_DATADIR}/EigenQt)

if(EIGENQT_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
if(EIGENQT_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()
