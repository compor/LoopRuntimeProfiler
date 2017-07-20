# cmake file

include(FindPackageHandleStandardArgs)
include(FindPythonInterp)

function(find_python_module name)
  set(pkg_name "PY_${name}")
  string(TOUPPER "${pkg_name}" module)

  set(result FALSE)

  # a module's location is usually a directory,
  # but for binary modules it's a .so file
  set(prg_str "import re, ${name}; \
  print re.compile('/__init__.py.*').sub('',${name}.__file__)")

  if(PYTHONINTERP_FOUND)
    execute_process(COMMAND ${PYTHON_EXECUTABLE} -c "${prg_str}"
      RESULT_VARIABLE result
      OUTPUT_VARIABLE location
      ERROR_QUIET
      OUTPUT_STRIP_TRAILING_WHITESPACE)
  else()
    message(WARNING "Could not find Python interpreter")
  endif()

  if(NOT result)
    message(STATUS "Python module ${module} location: '${location}'")
    set(${module} ${location})
    set(${module} ${location} CACHE STRING "Location of Python module: ${name}")
  endif()

  find_package_handle_standard_args(${pkg_name} DEFAULT_MSG ${module})

  set("${module}_FOUND" ${${pkg_name}_FOUND} PARENT_SCOPE)
endfunction()

