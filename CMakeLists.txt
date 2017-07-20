# cmake file

# build tool configuration

cmake_minimum_required(VERSION 3.2.0)

enable_testing()

# add project cmake module path
set(PRJ_CMAKE_MODULE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
list(APPEND CMAKE_MODULE_PATH ${PRJ_CMAKE_MODULE_DIR})

include(CMakePackageConfigHelpers)
include(Util)
include(HandleLLVMDependency)

option(PRJ_USE_LLVM_INTERNAL_MODULE "Use LLVM macro for creating module" OFF)
option(PRJ_SKIP_TESTS "Skip configuring the project tests" OFF)
option(PRJ_DEBUG "Enable project custom debug output facilities" ON)

message(STATUS "project debug: ${PRJ_DEBUG}")

get_version(VERSION PRJ_VERSION)

set(PRJ_NAME LoopRuntimeProfiler)
string(TOUPPER ${PRJ_NAME} PRJ_NAME_UPPER)

# part of LLVM or standalone
set(PRJ_STANDALONE_BUILD FALSE)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  project(${PRJ_NAME} C CXX)

  set(PRJ_STANDALONE_BUILD TRUE)

  message(STATUS "configuring as external LLVM project")
endif()


# CMP0056 is required for any libc++abi dependency of the try_compile() command
# this bumps required version to 3.2
# otherwise LD_LIBRARY_DIR needs to be set
set_policies(CMP0056 NEW)


# requirements

handle_llvm_dependency(${PRJ_STANDALONE_BUILD})


# project configuration

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

if(NOT LLVM_ENABLE_RTTI)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()

message(STATUS "RTTI support: ${LLVM_ENABLE_RTTI}")

if(NOT LLVM_ENABLE_EH)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
endif()

message(STATUS "EH support: ${LLVM_ENABLE_EH}")


# when we are building the project as standalone, we'd like to leave the install
# RPATH empty so that potentially we can dynamically link against other
# installed LLVM toolchains

if(PRJ_STANDALONE_BUILD)
  set(CMAKE_SKIP_BUILD_RPATH FALSE)
  set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
  #set(CMAKE_INSTALL_RPATH ${LLVM_LIBRARY_DIRS})

  if(CMAKE_VERSION VERSION_EQUAL "3.8.0" OR CMAKE_VERSION VERSION_GREATER "3.8.0")
    set(CMAKE_BUILD_RPATH ${LLVM_LIBRARY_DIRS})
  else()
    # unfortunately this also sets the install RPATH for older cmake versions
    #set(PRJ_LINKER_FLAGS "${PRJ_LINKER_FLAGS} -Wl,-rpath ${LLVM_LIBRARY_DIRS}")
  endif()

  set(PRJ_LINKER_FLAGS "${PRJ_LINKER_FLAGS} -Wl,--disable-new-dtags")

  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${PRJ_LINKER_FLAGS}")
  set(CMAKE_SHARED_LINKER_FLAGS 
    "${CMAKE_SHARED_LINKER_FLAGS} ${PRJ_LINKER_FLAGS}")
  set(CMAKE_MODULE_LINKER_FLAGS 
    "${CMAKE_MODULE_LINKER_FLAGS} ${PRJ_LINKER_FLAGS}")
endif()


# dependencies

#find_package(Foo CONFIG)

#if(Foo_FOUND)
  #set(${PRJ_NAME_UPPER}_USES_FOO 1)
#else()
  #set(${PRJ_NAME_UPPER}_USES_FOO 0)
  #message(WARNING "package Foo was not found; skipping.")
#endif()

#

set(LIB_NAME "LLVM${PRJ_NAME}Pass")
set(LIB_SOURCES "lib/LoopRuntimeProfilerPass.cpp")

if(NOT PRJ_USE_LLVM_INTERNAL_MODULE)
  add_library(${LIB_NAME} MODULE ${LIB_SOURCES})
else()
  # this follows the suggestion documented in
  # http://llvm.org/docs/CMake.html#cmake-out-of-source-pass
  # for allowing future inclusion of the plugin in the LLVM source tree
  # by using LLVM's internal cmake functions and utilities
  set(LLVM_RUNTIME_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/bin)
  set(LLVM_LIBRARY_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/lib)

  add_llvm_loadable_module(${LIB_NAME} ${LIB_SOURCES})
endif()

# according to http://llvm.org/docs/CMake.html
# we do not need the below library dependencies since the plugin will be loaded
# via opt or clang which already have those libs in their dependencies
# comment out the following two statements to test against static lib LLVM build

#llvm_map_components_to_libnames(llvm_libs core support analysis ipo passes)
#target_link_libraries(${LIB_NAME} PUBLIC ${llvm_libs})

if(Foo_FOUND)
  target_link_libraries(${LIB_NAME} PUBLIC Foo)
endif()

target_compile_definitions(${LIB_NAME} PUBLIC ${LLVM_DEFINITIONS})
target_compile_definitions(${LIB_NAME} PRIVATE VERSION_STRING=${PRJ_VERSION})

set(PRJ_DEBUG_VALUE 0)
if(PRJ_DEBUG)
  set(PRJ_DEBUG_VALUE 1)
endif()

target_include_directories(${LIB_NAME} PUBLIC ${LLVM_INCLUDE_DIRS})
target_include_directories(${LIB_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)
target_include_directories(${LIB_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>)
target_include_directories(${LIB_NAME} PUBLIC
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>)
target_include_directories(${LIB_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)

#

get_property(TRGT_PREFIX TARGET ${LIB_NAME} PROPERTY PREFIX)
get_property(TRGT_SUFFIX TARGET ${LIB_NAME} PROPERTY SUFFIX)

if(NOT TRGT_PREFIX)
  set(TRGT_PREFIX ${CMAKE_SHARED_MODULE_PREFIX})
endif()

if(NOT TRGT_SUFFIX)
  set(TRGT_SUFFIX ${CMAKE_SHARED_MODULE_SUFFIX})
endif()

#

set(TESTEE_PREFIX ${TRGT_PREFIX})
set(TESTEE_SUFFIX ${TRGT_SUFFIX})
set(TESTEE_LIB ${LIB_NAME})

add_subdirectory(unittests)
add_subdirectory(tests)
add_subdirectory(doc)


attach_compilation_db_command(${LIB_NAME})


# installation

set(PRJ_INSTALL_INCLUDE_DIR "include")
set(PRJ_INSTALL_SHARE_DIR "share")
set(PRJ_CONFIG_HEADER_FILE "Config.hpp")
set(PRJ_CMAKE_CONFIG_FILE "config.cmake")

set(DEPENDEE "")

if(Foo_FOUND)
  list(APPEND DEPENDEE ${FOO_LIB_DIR}${TRGT_PREFIX}${FOO_LIB}${TRGT_SUFFIX})
endif()

if(NOT PRJ_STANDALONE_BUILD)
  set(PRJ_INSTALL_INCLUDE_DIR ${PRJ_NAME}${PRJ_INSTALL_INCLUDE_DIR})
  set(PRJ_INSTALL_SHARE_DIR ${PRJ_NAME}${PRJ_INSTALL_SHARE_DIR})
endif()

if(PRJ_STANDALONE_BUILD)
  install(TARGETS ${LIB_NAME} EXPORT ${PRJ_NAME} LIBRARY DESTINATION "lib")
endif()

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/include")
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES
  "${CMAKE_CURRENT_BINARY_DIR}/include")

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/include/${PRJ_CONFIG_HEADER_FILE}.in"
  "${CMAKE_CURRENT_BINARY_DIR}/include/${PRJ_CONFIG_HEADER_FILE}")

string(TOLOWER ${PRJ_NAME} PRJ_NAME_LOWER)
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/share/cmake/${PRJ_CMAKE_CONFIG_FILE}.in"
  "${CMAKE_CURRENT_BINARY_DIR}/share/cmake/${PRJ_NAME_LOWER}-${PRJ_CMAKE_CONFIG_FILE}"
  @ONLY)

get_version(SHORT VERSION PRJ_VERSION_SHORT)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/share/cmake/${PRJ_NAME_LOWER}-config-version.cmake"
  VERSION ${PRJ_VERSION_SHORT} COMPATIBILITY SameMajorVersion)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/" 
  DESTINATION ${PRJ_INSTALL_INCLUDE_DIR}
  PATTERN "*.in" EXCLUDE)
install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/include/" 
  DESTINATION ${PRJ_INSTALL_INCLUDE_DIR})
install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/share/"
  DESTINATION ${PRJ_INSTALL_SHARE_DIR})

install(EXPORT ${PRJ_NAME} DESTINATION "${PRJ_INSTALL_SHARE_DIR}/cmake")

