cmake_minimum_required(VERSION 3.6.0 FATAL_ERROR)

# If we're on windows we need to tell the linker which symbols to export
# we do this with a def file
# We also need to use a different manifest file on unix because the library 
# will have a different file name, ie dll/so
if(WIN32)
	set(SOURCES ${CMAKE_CURRENT_LIST_DIR}/VkFakeLayer.def
		${CMAKE_CURRENT_LIST_DIR}/WindowsManifest.json)
else()
	set(SOURCES ${SOURCES} ${CMAKE_CURRENT_LIST_DIR}/UnixManifest.json)
endif()
target_sources(VkFakeLayer PRIVATE ${SOURCES})