# cmake file

set(PRJ_RT_NAME lrp_dummy_rt)

set(SOURCES ${PRJ_RT_NAME}.cpp)

add_library(${PRJ_RT_NAME} STATIC ${SOURCES})

install(TARGETS ${PRJ_RT_NAME} EXPORT ${PRJ_NAME} 
  ARCHIVE DESTINATION "runtime/lib")

