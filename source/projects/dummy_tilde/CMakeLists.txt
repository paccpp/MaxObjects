cmake_minimum_required(VERSION 3.0)


include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-api/script/max-pretarget.cmake)

file(GLOB_RECURSE PROJECT_HEADERS
	${CMAKE_CURRENT_SOURCE_DIR}/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
)

file(GLOB_RECURSE PROJECT_SRC
	${CMAKE_CURRENT_SOURCE_DIR}/*.c
	${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

set(PROJECT_FILES
	${PROJECT_SRC}
	${PROJECT_HEADERS}
)

include_directories(
	"${C74_INCLUDES}"
)

add_library(
	${PROJECT_NAME}
	MODULE
	"${PROJECT_FILES}"
)


include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-api/script/max-posttarget.cmake)
