# cmake file

set(PRJ_RT_NAME lrp_executed_rt)

set(SOURCES ${PRJ_RT_NAME}.cpp)

add_library(${PRJ_RT_NAME} ${RUNTIME_LIB_TYPE} ${SOURCES})

install(TARGETS ${PRJ_RT_NAME} EXPORT ${PRJ_NAME} 
  ARCHIVE DESTINATION "runtime/lib"
  LIBRARY DESTINATION "runtime/lib")

