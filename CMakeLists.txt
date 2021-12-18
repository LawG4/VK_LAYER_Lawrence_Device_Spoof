cmake_minimum_required(VERSION 3.6.0 FATAL_ERROR)

# Start by ensuring the Vulkan loader has been pulled.
if(NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/Vulkan-Loader/loader)
	message(STATUS "Vulkan Loader not found, pulling sources")
	
	# Activate git submodule from within CMake
	execute_process(
		COMMAND git submodule update --init --recursive
		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
	)
endif()

# Create a Project it will contain the library and a test application
project(FakeLayer VERSION 0.1.0 LANGUAGES CXX C)
add_executable(VkTestApplication Source/VkTestApplication/Main.cpp)
add_library(VkFakeLayer SHARED Source/VkFakeLayer/VkFakeLayer.cpp)

# Make the test application depend on the layer
add_dependencies(VkTestApplication VkFakeLayer)

# Add the loader to the build system and ensure that we pull dependencies
set(UPDATE_DEPS "True" CACHE STRING "Update the Vulkan loader dependencies")
set(CMAKE_BUILD_TYPE "Debug" CACHE INTERNAL "Default build type for loader" FORCE)

# Ensure that the build system uses the vulkan headers in this repo not the users installed ones
set(VULKAN_HEADERS_INSTALL_DIR "${CMAKE_CURRENT_LIST_DIR}/Vulkan-Loader/external/Vulkan-Headers" 
	CACHE PATH "Path to Vulkan headers downloaded as submoule" FORCE)
set(VulkanHeaders_INCLUDE_DIR "${VULKAN_HEADERS_INSTALL_DIR}/include" CACHE PATH "" FORCE)
set(VulkanRegistry_DIR "${VULKAN_HEADERS_INSTALL_DIR}/share/vulkan/registry" CACHE PATH "" FORCE)
add_subdirectory(Vulkan-Loader)

# Make the Layer (and Thus test application) depend on the Vulkan loader target
add_dependencies(VkFakeLayer vulkan vl_update_deps)

# Allow the layer and test application to include the vulkan headers
target_include_directories(VkFakeLayer PUBLIC 
	${CMAKE_CURRENT_LIST_DIR}/Vulkan-Loader/external/Vulkan-Headers/include
	${CMAKE_CURRENT_LIST_DIR}/Vulkan-Loader/loader
	${CMAKE_CURRENT_LIST_DIR}/Vulkan-Loader/loader/generated)
target_include_directories(VkTestApplication PUBLIC 
	${CMAKE_CURRENT_LIST_DIR}/Vulkan-Loader/external/Vulkan-Headers/include/vulkan)

# Add a custom target which is responsible for copying the json manifest file to the exe directory
set(Layer_name VK_LAYER_Lawrence_Device_Spoof.json)
add_custom_command(
	OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/bin/$<CONFIG>/${Layer_name}
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${CMAKE_CURRENT_LIST_DIR}/Source/VkFakeLayer/${Layer_name}
		${CMAKE_CURRENT_BINARY_DIR}/bin/$<CONFIG>/${Layer_name}
	DEPENDS ${CMAKE_CURRENT_LIST_DIR}/Source/VkFakeLayer/${Layer_name}
	COMMENT "Copying layer manifest file : ${Layer_name}"
	VERBATIM
)
target_sources(VkFakeLayer PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/bin/$<CONFIG>/${Layer_name} )

# Set the executable and layer to be outputted into the same directory
set_target_properties(VkFakeLayer VkTestApplication vulkan
	PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:VkTestApplication>")

# Always default to running the Test application
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT VkTestApplication)

# Make VktestApplication use c++17 for current_path
set_property(TARGET VkTestApplication PROPERTY CXX_STANDARD 17)

# Vulkan-Loader Generates a bunch of projects we want to group together and hide
set_target_properties(asm_offset 
	vl_update_deps VulkanLoader_generated_source PROPERTIES FOLDER "Vulkan Loader Targets")

# Add the different CMake Lists for the executable and the layer so we can add more sources later
add_subdirectory(Source/VkTestApplication)
add_subdirectory(Source/VkFakeLayer)

# Now link the test appplication to the Vulkan loader as that's easier than manually retrieving pointers
target_link_libraries(VkTestApplication vulkan)

# Add a little function that sorts all of the files in VS
# Yoinked from https://stackoverflow.com/a/31423421
function(assign_source_group)
    foreach(_source IN ITEMS ${ARGN})
        if (IS_ABSOLUTE "${_source}")
            file(RELATIVE_PATH _source_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_source}")
        else()
            set(_source_rel "${_source}")
        endif()
        get_filename_component(_source_path "${_source_rel}" PATH)
        string(REPLACE "Source/" "" _source_path "${_source_path}")
        string(REPLACE "/" "\\" _source_path_msvc "${_source_path}")
        source_group("${_source_path_msvc}" FILES "${_source}")
    endforeach()
endfunction(assign_source_group)

# Sort the Test Apps files
get_target_property(PROJ_SOURCES VkTestApplication SOURCES)
assign_source_group(${PROJ_SOURCES})

# Sort the Layer's files
get_target_property(PROJ_SOURCES VkFakeLayer SOURCES)
assign_source_group(${PROJ_SOURCES})