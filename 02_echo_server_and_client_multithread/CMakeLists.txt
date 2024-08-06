# In subdir1/CMakeLists.txt
file(GLOB SOURCES "*.cc")

# Extract the directory name prefix
get_filename_component(DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(REGEX MATCH "^[0-9]+" PREFIX ${DIR_NAME})

foreach(SOURCE ${SOURCES})
  get_filename_component(TARGET_NAME ${SOURCE} NAME_WE)
  set(TARGET_NAME_UNIQUE "${PREFIX}_${TARGET_NAME}")
  message(STATUS "Creating target: ${TARGET_NAME_UNIQUE}")
  add_executable(${TARGET_NAME_UNIQUE} ${SOURCE})
endforeach()
