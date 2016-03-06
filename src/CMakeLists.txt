project(reflection_lib)

include_directories( ./ )

set(SRC 
	constructor.cpp
	constructor.h
	enum.cpp
	enum.h
	exception.h
	field.cpp
	field.h
	method.cpp
	method.h
	param.cpp
	param.h
	reflection.h
	type.cpp
	type.h
	utils.h
	string_helper.h
	qualified_type.cpp
	qualified_type.h
)

add_library(lib ${SRC})
set_target_properties(lib PROPERTIES OUTPUT_NAME "reflection")

set(LIBRARY_OUTPUT_PATH ${HOME}/lib)
