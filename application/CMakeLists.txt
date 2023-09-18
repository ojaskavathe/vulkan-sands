project(application)
add_executable(application Application.cpp)

set(SHADER_SOURCE_DIR ${CMAKE_SOURCE_DIR}/res/shaders)
set(SHADER_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/res/shaders)

file(GLOB SHADERS
    ${SHADER_SOURCE_DIR}/*.vert
    ${SHADER_SOURCE_DIR}/*.frag
    ${SHADER_SOURCE_DIR}/*.comp
    ${SHADER_SOURCE_DIR}/*.geom
    ${SHADER_SOURCE_DIR}/*.tesc
    ${SHADER_SOURCE_DIR}/*.tese
    ${SHADER_SOURCE_DIR}/*.mesh
    ${SHADER_SOURCE_DIR}/*.task
    ${SHADER_SOURCE_DIR}/*.rgen
    ${SHADER_SOURCE_DIR}/*.rchit
    ${SHADER_SOURCE_DIR}/*.rmiss)

add_custom_command(
    OUTPUT ${SHADER_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_BINARY_DIR}
    COMMENT "Creating ${SHADER_BINARY_DIR}")

foreach(SHADER IN LISTS SHADERS)
    message("Shader: ${SHADER}")
    get_filename_component(FILENAME ${SHADER} NAME_WE)
    message("Filename: ${FILENAME}")
    add_custom_command(
        OUTPUT ${SHADER_BINARY_DIR}/${FILENAME}.spv
        COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${SHADER} -o ${SHADER_BINARY_DIR}/${FILENAME}.spv
        DEPENDS ${SHADER}
        COMMENT "[glslc]: Compiling ${FILENAME}")
    list(APPEND SPV_SHADERS ${SHADER_BINARY_DIR}/${FILENAME}.spv)
endforeach()

add_custom_target(shaders ALL DEPENDS ${SPV_SHADERS})
add_dependencies(application shaders)

target_link_libraries(application PRIVATE engine)