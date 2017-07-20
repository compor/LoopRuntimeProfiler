# cmake file

include(CMakeParseArguments)

function(debug message_txt)
  message(STATUS "[DEBUG] ${message_txt}")
endfunction()


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
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE ver
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  set(${get_version_VERSION} "${ver}" PARENT_SCOPE)
endfunction()


function(attach_compilation_db_command trgt)
  if(NOT TARGET ${trgt})
    fatal("cannot attach custom command to non-target: ${trgt}")
  endif()

  set(file "compile_commands.json")

  add_custom_command(TARGET ${trgt} POST_BUILD
    COMMAND ${CMAKE_COMMAND}
    ARGS -E copy_if_different ${file} "${CMAKE_CURRENT_SOURCE_DIR}/${file}"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    VERBATIM)
endfunction()


function(set_policies)
  math(EXPR is_even "${ARGC} % 2")
  math(EXPR upper "${ARGC} - 1")

  if(NOT is_even EQUAL 0)
    fatal("set_policies requires an even number of arguments")
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

