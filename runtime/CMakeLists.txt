# cmake file

set(PRJ_RT_NAME lrp_rt)

set(SOURCES ${PRJ_RT_NAME}.c)

add_library(${PRJ_RT_NAME} STATIC ${SOURCES})

