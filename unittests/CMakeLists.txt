# cmake file

# requirements

if(PRJ_SKIP_TESTS)
  message(WARNING "Testing is disabled; skipping unit tests")

  return()
endif()


find_package(GTest)

if(NOT GTEST_FOUND)
  message(WARNING "Could not find GTest; skipping unit tests")

  return()
endif()

find_package(Threads REQUIRED)
find_package(Boost REQUIRED)

# configuration

# aggregate unit test targets under a pseudo-target
add_custom_target(unittests)

set(PRJ_TEST_NAMES Test${PRJ_NAME})

foreach(prj_test_name ${PRJ_TEST_NAMES})
  set(TEST_SOURCES "${prj_test_name}.cpp")

  add_executable(${prj_test_name} ${TEST_SOURCES})

  target_compile_definitions(${prj_test_name} PUBLIC ${LLVM_DEFINITIONS})

  target_include_directories(${prj_test_name} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
  target_include_directories(${prj_test_name} PUBLIC ${GTEST_INCLUDE_DIRS})
  target_include_directories(${prj_test_name} PUBLIC ${LLVM_INCLUDE_DIRS})
  target_include_directories(${prj_test_name} PUBLIC ${Boost_INCLUDE_DIRS})
  target_include_directories(${prj_test_name} PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>)
  target_include_directories(${prj_test_name} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)

  target_link_libraries(${prj_test_name} PUBLIC ${GTEST_BOTH_LIBRARIES})  
  target_link_libraries(${prj_test_name} PUBLIC ${CMAKE_THREAD_LIBS_INIT})  

  llvm_map_components_to_libnames(llvm_libs 
    core support asmparser analysis passes)

  target_link_libraries(${prj_test_name} PUBLIC ${llvm_libs})

  set_property(TARGET ${prj_test_name} APPEND_STRING PROPERTY 
    LINK_FLAGS " -Wl,--warn-unresolved-symbols")

  # setup dependency between unit test targets and their production code
  add_dependencies(${prj_test_name} ${TESTEE_LIB})

  # exclude unit test targets from main build
  set_target_properties(${prj_test_name} PROPERTIES EXCLUDE_FROM_ALL TRUE)

  add_dependencies(unittests ${prj_test_name})

  get_property(PRJ_UNIT_TESTS_EXE TARGET ${prj_test_name} PROPERTY NAME)
endforeach()


set(PRJ_UNIT_TESTS_SCRIPT "run_unit_tests.sh")
get_property(PRJ_UNIT_TESTS_TARGET TARGET unittests PROPERTY NAME)

configure_file("${CMAKE_SOURCE_DIR}/utils/scripts/${PRJ_UNIT_TESTS_SCRIPT}.in" 
  "${CMAKE_BINARY_DIR}/${PRJ_UNIT_TESTS_SCRIPT}"
  @ONLY)


set(TEST_DATA_DIR "${CMAKE_CURRENT_BINARY_DIR}/data")
file(MAKE_DIRECTORY ${TEST_DATA_DIR})

add_custom_command(TARGET unittests PRE_BUILD COMMAND 
  ${CMAKE_COMMAND} -E copy_directory 
  "${CMAKE_CURRENT_SOURCE_DIR}/data/" ${TEST_DATA_DIR})

