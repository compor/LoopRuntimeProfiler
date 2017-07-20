# cmake file

macro(handle_llvm_dependency isStandaloneBuild)
  if(${isStandaloneBuild})
    # find_package in CONFIG mode will lookup and include LLVMConfig.cmake
    find_package(LLVM REQUIRED CONFIG)

    if(NOT LLVM_FOUND)
      message(FATAL_ERROR "Could not find LLVM")
    endif()

    message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
    message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
  endif()

  if(LLVM_PACKAGE_VERSION VERSION_LESS "3.8.0")
    set(LLVM_RUNTIME_OUTPUT_INTDIR "${CMAKE_BINARY_DIR}/bin/${CMAKE_CFG_INT_DIR}")
    set(LLVM_RUNTIME_OUTPUT_INTDIR "${CMAKE_BINARY_DIR}/lib/${CMAKE_CFG_INT_DIR}")
  endif()

  # These are required if functions are used:
  # - add_llvm_loadable_module()
  # - add_llvm_library
  if(${isStandaloneBuild})
    list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
  else()
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}/share/llvm/cmake/")
  endif()

  include(AddLLVM)
  include(HandleLLVMOptions)
endmacro()

