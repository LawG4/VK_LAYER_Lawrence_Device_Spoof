cmake_minimum_required(VERSION 3.6.0 FATAL_ERROR)

# If we're on windows we need to tell the linker which symbols to export
# we do this with a def file
if(WIN32)
	set(SOURCES ${CMAKE_CURRENT_LIST_DIR}/VkFakeLayer.def)
endif()

# build up a list of the rest of the source files
set(SOURCES ${SOURCES}
)

target_sources(VkFakeLayer PRIVATE ${SOURCES})
target_sources(VkFakeLayer PRIVATE ${CMAKE_CURRENT_LIST_DIR}/VK_LAYER_Lawrence_Device_Spoof.json)