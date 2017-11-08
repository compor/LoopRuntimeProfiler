# cmake file

include(CMakeParseArguments)


function(get_version)
  set(options SHORT)
  set(oneValueArgs VERSION)
  set(multiValueArgs)

  cmake_parse_arguments(get_version "${options}" "${oneValueArgs}"
    "${multiValueArgs}" ${ARGN})

  if(get_version_SHORT)
    set(cmd_arg "--abbrev=0")
  else()
    set(cmd_arg "--long")
  endif()

  execute_process(COMMAND git describe --tags --always ${cmd_arg}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE ver
    RESULT_VARIABLE result
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  if(result)
    set(ver "0.0.0")
  endif()

  set(${get_version_VERSION} "${ver}" PARENT_SCOPE)
endfunction()


function(format_check)
  set(options)
  set(oneValueArgs COMMAND)
  set(multiValueArgs FILES)

  cmake_parse_arguments(format_check "${options}" "${oneValueArgs}"
    "${multiValueArgs}" ${ARGN})

  set(rv 0)

  foreach(file ${format_check_FILES})
    execute_process(COMMAND
      ${format_check_COMMAND} ${file} | diff -u ${file} - &> /dev/null
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      RESULT_VARIABLE rv
      OUTPUT_STRIP_TRAILING_WHITESPACE)

    if(rv)
      break()
    endif()
  endforeach()

  if(rv)
    message(STATUS "code formatting is compliant")
  else()
    message(WARNING "code formatting is not compliant")
  endif()
endfunction()


function(attach_compilation_db)
  set(options)
  set(oneValueArgs TARGET)
  set(multiValueArgs)

  cmake_parse_arguments(acdb "${options}" "${oneValueArgs}"
    "${multiValueArgs}" ${ARGN})

  if(NOT TARGET ${acdb_TARGET})
    message(FATAL_ERROR
      "cannot attach custom command to non-target: ${acdb_TARGET}")
  endif()

  set(file "compile_commands.json")

  get_target_property(TRGT_TYPE ${acdb_TARGET} TYPE)
  file(TO_CMAKE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${file}" GENERATED_FILE)

  if(${TRGT_TYPE} STREQUAL "INTERFACE_LIBRARY")
    add_custom_command(OUTPUT ${GENERATED_FILE}
    COMMAND ${CMAKE_COMMAND}
      ARGS -E copy_if_different ${file} ${GENERATED_FILE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    VERBATIM)
  else()
    add_custom_command(TARGET ${acdb_TARGET} POST_BUILD
      COMMAND ${CMAKE_COMMAND}
        ARGS -E copy_if_different ${file} ${GENERATED_FILE}
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      VERBATIM)
  endif()
endfunction()


function(set_policies)
  math(EXPR is_even "${ARGC} % 2")
  math(EXPR upper "${ARGC} - 1")

  if(NOT is_even EQUAL 0)
    message(FATAL_ERROR "set_policies requires an even number of arguments")
  endif()

  foreach(idx RANGE 0 ${upper} 2)
    set(plc "${ARGV${idx}}")

    math(EXPR nxt_idx "${idx} + 1")
    set(newval "${ARGV${nxt_idx}}")

    if(POLICY ${plc})
      cmake_policy(GET ${plc} oldval)

      if(NOT oldval EQUAL newval)
        cmake_policy(SET "${plc}" "${newval}")

        message(STATUS "policy ${plc}: ${newval}")
      endif()
    else()
      message(WARNING "policy ${plc} is not defined")
    endif()
  endforeach()
endfunction()

