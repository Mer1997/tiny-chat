cmake_minimum_required(VERSION 3.14)
project(
  chatroom
  VERSION 0.1
  DESCRIPTION "A tiny chatroom using c++"
  LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# # Get all .cc files in the current directory file(GLOB CC_FILES "*.cc")

# foreach(CC_FILE ${CC_FILES}) # Extract the filename without the extension
# get_filename_component(TARGET_NAME ${CC_FILE} NAME_WE)

# # Create an executable target for each .cc file add_executable(${TARGET_NAME}
# ${CC_FILE}) endforeach()

include(CMakePrintHelpers)

# Only do these if this is the main project, and not if it is included through
# add_subdirectory CMAKE_PROJECT_NAME 是顶层的 project name， PROJECT_NAME 是当前文件的
# project name
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

  # Optionally set things like CMAKE_CXX_STANDARD,
  # CMAKE_POSITION_INDEPENDENT_CODE here

  # Let's ensure -std=c++xx instead of -std=g++xx
  set(CMAKE_CXX_EXTENSIONS OFF)

  # Let's nicely support folders in IDEs
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

# FetchContent added in CMake 3.11, downloads during the configure step
include(FetchContent)
# FetchContent_MakeAvailable was not added until CMake 3.14; use our shim
if(${CMAKE_VERSION} VERSION_LESS 3.14)
  include(cmake/add_FetchContent_MakeAvailable.cmake)
endif()

# Set the output directory for all executables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

file(GLOB SUBDIRS LIST_DIRECTORIES true *)
foreach(SUBDIR ${SUBDIRS})
    if(IS_DIRECTORY ${SUBDIR} AND EXISTS ${SUBDIR}/CMakeLists.txt)
        add_subdirectory(${SUBDIR})
    endif()
endforeach()
