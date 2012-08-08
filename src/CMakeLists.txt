find_package(OpenCL REQUIRED)
find_package(DevIL REQUIRED)
find_package(MayCore REQUIRED)
include_directories(${OPENCL_INCLUDE_DIRS} ${IL_INCLUDE_DIR} ${MAYCORE_INCLUDE_DIRS})
add_library(may_cl STATIC
	lib.c ex.c mcl.c program.c error.c image.c iloader.c filter.c
	filters/const.c filters/blur.c filters/image.c)
target_link_libraries(may_cl ${OPENCL_LIBRARIES} ${IL_LIBRARIES} ${MAYCORE_LIBRARIES})
