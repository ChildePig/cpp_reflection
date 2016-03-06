project(test_json)

set(TEST_SRC 
	json11.cpp
	json11.hpp
	test_json.cpp
)

include_directories(${HOME}/src)

link_directories(${HOME}/lib)

add_executable(test_json ${TEST_SRC})
add_dependencies(test_json lib)

target_link_libraries(test_json reflection)

set(EXECUTABLE_OUTPUT_PATH ${HOME}/bin)
