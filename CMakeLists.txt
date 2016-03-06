project(reflection)

set(HOME ${reflection_SOURCE_DIR})

#if(CMAKE_COMPILER_IS_GNUCXX)
	add_definitions("-std=c++11")
#endif()

add_subdirectory(src)
add_subdirectory(test_json)
