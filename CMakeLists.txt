cmake_minimum_required(VERSION 3.12.0)

PROJECT(RayTracingTheNextWeek)


FILE (GLOB ALL_SOURCES "./RayTracingTheNextWeek.cpp")
source_group(Src FILES  ${ALL_SOURCES})

FILE (GLOB ALL_INCLUDES "./include/*.h")
source_group(rtw_include FILES  ${ALL_INCLUDES})

FILE (GLOB ALL_DEPENDS  "./3rdParty/*.h")
source_group(3rdParty  FILES  ${ALL_DEPENDS})

if (MSVC)
	# Set the startup project .
	if (NOT ${CMAKE_VERSION} VERSION_LESS "3.12.0")
		 set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${CMAKE_PROJECT_NAME})
	endif ()
	add_definitions(-DRTW_OS_WIN=1)
else()
	add_definitions(-DRTW_OS_MAC=1)
endif (MSVC)

#set(CMAKE_BUILD_TYPE "Release") # default run in release,more faster

add_executable(${CMAKE_PROJECT_NAME} 
    ${ALL_SOURCES}
    ${ALL_INCLUDES}
	${ALL_DEPENDS})
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE "./include" "./3rdParty")



