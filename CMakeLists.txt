cmake_minimum_required(VERSION 3.20)
project(UrsaDemo VERSION 0.1.0 LANGUAGES CXX)

set(URSA_ENGINE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../Engine" CACHE PATH "Path to the UrsaEngine repository")
if(NOT EXISTS "${URSA_ENGINE_DIR}/CMakeLists.txt")
  message(FATAL_ERROR "UrsaEngine was not found at '${URSA_ENGINE_DIR}'. Set URSA_ENGINE_DIR to the engine repository path.")
endif()

enable_testing()
add_subdirectory("${URSA_ENGINE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/Engine")
find_package(Vulkan 1.2 REQUIRED COMPONENTS glslc)

function(ursa_demo_shader target source)
  get_filename_component(name "${source}" NAME)
  set(output "${CMAKE_CURRENT_BINARY_DIR}/shaders/${name}.spv")
  add_custom_command(OUTPUT "${output}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/shaders"
    COMMAND Vulkan::glslc "${CMAKE_CURRENT_SOURCE_DIR}/${source}" -o "${output}"
    DEPENDS "${source}" VERBATIM)
  target_sources(${target} PRIVATE "${output}")
  set_source_files_properties("${output}" PROPERTIES GENERATED TRUE HEADER_FILE_ONLY TRUE)
endfunction()

add_executable(UrsaTriangle src/Triangle.cpp)
target_link_libraries(UrsaTriangle PRIVATE Ursa::Engine)
target_compile_definitions(UrsaTriangle PRIVATE URSA_SHADER_DIR="${CMAKE_CURRENT_BINARY_DIR}/shaders")
ursa_demo_shader(UrsaTriangle shaders/triangle.vert)
ursa_demo_shader(UrsaTriangle shaders/triangle.frag)

add_test(NAME UrsaWindowSmoke COMMAND UrsaTriangle --hidden --frames 20 --stress-window)
