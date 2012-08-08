add_executable(run main.c)
add_dependencies(run may_cl)
include_directories(${OPENCL_INCLUDE_DIRS})
target_link_libraries(run may_cl ${OPENCL_LIBRARIES})
