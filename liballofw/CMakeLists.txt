# liballofw: Framework for Immserive Multi-display Applications.
# Author: Donghao Ren, 2015

  CMAKE_MINIMUM_REQUIRED ( VERSION 2.8 )
  PROJECT ( liballofw )

#----------------------------------------------------------------------------------------------------------------------
# User Configuration Parameters

  SET ( WITH_SKIA   TRUE CACHE BOOL "Build with Skia.")

#----------------------------------------------------------------------------------------------------------------------
# Configuration Parameters

  SET ( CMAKE_BUILD_TYPE Release )

  SET ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules" )

  # Enable C++11 support (mandatory)
  SET ( CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}" )

  # Show all warnings.
  # SET ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall" )


  IF ( ${CMAKE_SYSTEM_NAME} MATCHES "Win" )
  # Windows specific options
  ADD_DEFINITIONS( -DYAML_CPP_DLL )
  SET ( GLEW_LIBRARY "../dependencies/windows-binaries/glew-2.0.0/lib/Release/x64/glew32" )
  SET ( GLEW_INCLUDE_DIR "./dependencies/windows-binaries/glew-2.0.0/include" )
  SET ( BUILD_SHARED_LIBS YES )
  SET ( CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS YES )
  SET ( FFMPEG_INCLUDE_DIR "./dependencies/windows-binaries/ffmpeg-3.2.2/include" )
  SET ( FFMPEG_LDFLAGS
    ../dependencies/windows-binaries/ffmpeg-3.2.2/lib/avutil
    ../dependencies/windows-binaries/ffmpeg-3.2.2/lib/swscale
    ../dependencies/windows-binaries/ffmpeg-3.2.2/lib/avcodec
    ../dependencies/windows-binaries/ffmpeg-3.2.2/lib/avformat
  )
  ENDIF ( )

#----------------------------------------------------------------------------------------------------------------------
# Main Definitions

  # Put built libraries and binaries to bin and lib directories.
  SET ( EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin" )
  SET ( LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib" )

  # OS specific configurations.
  IF ( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )
    # For linux, report error when shared libraries contains undefined references.
    # Link c/c++ libraries statically.
    SET ( CMAKE_CXX_FLAGS "-fPIC ${CMAKE_CXX_FLAGS}" )
    SET ( CMAKE_EXE_LINKER_FLAGS "-Wl,--no-undefined -static-libgcc -static-libstdc++" )
    SET ( CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined -static-libgcc -static-libstdc++" )
    ADD_DEFINITIONS ( -DALLOFW_PLATFORM_LINUX )
  ENDIF ( )

  IF ( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
    ADD_DEFINITIONS ( -DALLOFW_PLATFORM_MACOSX )
  ENDIF ( )

  FIND_PACKAGE ( PkgConfig )

#----------------------------------------------------------------------------------------------------------------------
# Dependencies

  INCLUDE_DIRECTORIES (
    BEFORE SYSTEM
      ${OPENGL_INCLUDE_DIRS}
      dependencies/yaml-cpp/include
      dependencies/glfw/include
      dependencies/lodepng
  )

  ADD_LIBRARY ( lodepng
    dependencies/lodepng/lodepng.cpp
  )

  SET ( GLFW_BUILD_EXAMPLES OFF CACHE BOOL "GLFW Option" )
  SET ( GLFW_BUILD_TESTS OFF CACHE BOOL "GLFW Option" )
  SET ( GLFW_BUILD_DOCS  OFF CACHE BOOL "GLFW Option" )
  SET ( GLFW_INSTALL OFF CACHE BOOL "GLFW Option" )
  SET ( GLFW_DOCUMENT_INTERNALS OFF CACHE BOOL "GLFW Option" )
  ADD_SUBDIRECTORY ( dependencies/glfw )

  # Boost C++ Libraries (we use headers only).
  FIND_PACKAGE ( Boost REQUIRED )
  FIND_PACKAGE ( OpenGL REQUIRED )
  FIND_PACKAGE ( GLEW REQUIRED )
  Find_Package(Threads)

  FILE ( GLOB YAML_CPP_SOURCES "dependencies/yaml-cpp/src/[a-zA-Z]*.cpp")
  ADD_LIBRARY ( yaml-cpp
    ${YAML_CPP_SOURCES}
  )

  INCLUDE_DIRECTORIES (
    ${GLEW_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
  )

  IF ( ${WITH_SKIA} )
    FIND_PACKAGE ( Skia REQUIRED )
    PKG_CHECK_MODULES ( FFMPEG REQUIRED libavcodec libavutil libavformat libswscale )
    INCLUDE_DIRECTORIES (
      ${Skia_INCLUDE_DIRS}
      ${FFMPEG_INCLUDE_DIR}
    )
  ENDIF ( )

#----------------------------------------------------------------------------------------------------------------------
# Included Dependencies

#----------------------------------------------------------------------------------------------------------------------
# Global Include Directories

  INCLUDE_DIRECTORIES ( include )

#----------------------------------------------------------------------------------------------------------------------
# Skia Graphics Backend Library

  ADD_LIBRARY ( allofw SHARED
    src/stream.cpp
    src/math.cpp
    src/config.cpp
    src/logger.cpp
    src/opengl.cpp
    src/opengl_utils.cpp
    src/omnistereo.cpp
    src/warpblend.cpp
    src/allofw.cpp
  )

  TARGET_LINK_LIBRARIES ( allofw
    LINK_PRIVATE
      ${OPENGL_LIBRARIES}
      ${GLEW_LIBRARIES}
      ${GLFW_LIBRARIES}
      glfw
      lodepng
      yaml-cpp
  )


  IF ( ${CMAKE_SYSTEM_NAME} MATCHES "Win" )
  # Windows specific options
    TARGET_LINK_LIBRARIES ( allofw
      LINK_PRIVATE
        ws2_32
    )
  ENDIF ( )

  IF ( ${WITH_SKIA} )

    ADD_LIBRARY ( allofw-graphics SHARED
      src/graphics_skia.cpp
      src/graphics_ffmpeg.cpp
    )

    TARGET_LINK_LIBRARIES ( allofw-graphics
      LINK_PRIVATE
        allofw
        ${Skia_LIBRARIES}
        ${FFMPEG_LDFLAGS}
        ${OPENGL_LIBRARIES}
        ${GLEW_LIBRARIES}
        yaml-cpp
    )

    INSTALL (
      TARGETS
        allofw-graphics
      RUNTIME DESTINATION bin
      LIBRARY DESTINATION lib
      ARCHIVE DESTINATION lib
    )
  ENDIF ( )

#----------------------------------------------------------------------------------------------------------------------
# Samples

  FUNCTION(ADD_SAMPLE_EXECUTABLE sample_name sample_sources)
      ADD_EXECUTABLE ( ${sample_name} samples/${sample_sources} )
      TARGET_LINK_LIBRARIES ( ${sample_name}
        allofw
        ${OPENGL_LIBRARIES}
      )
      SET_TARGET_PROPERTIES ( ${sample_name}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/samples"
      )
  ENDFUNCTION(ADD_SAMPLE_EXECUTABLE)

#----------------------------------------------------------------------------------------------------------------------
# PKG-Config File

  CONFIGURE_FILE ( liballofw.pc.in
    ${LIBRARY_OUTPUT_PATH}/pkgconfig/liballofw.pc
    @ONLY
  )

#----------------------------------------------------------------------------------------------------------------------
# Install

  INSTALL (
    DIRECTORY
      include/allofw
    DESTINATION include
  )

  # Targets.
  INSTALL (
    TARGETS
      allofw
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
  )

  INSTALL (
    FILES
      ${LIBRARY_OUTPUT_PATH}/pkgconfig/liballofw.pc
    DESTINATION
      lib/pkgconfig
  )
