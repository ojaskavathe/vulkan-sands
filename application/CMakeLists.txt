project(application)
add_executable(application Application.cpp)

if (NOT Vulkan_GLSLC_EXECUTABLE)
    message("Error: Couldn't Locate the Vulkan SDK for glslc!")
    return()
endif()

set(SHADER_SOURCE_DIR ${CMAKE_SOURCE_DIR}/res/shaders)
set(SHADER_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/res/shaders)

set(SHADER_FINAL_DIR $<TARGET_FILE_DIR:application>/res/shaders)

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
    COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_FINAL_DIR}
)

foreach(SHADER IN LISTS SHADERS)
    get_filename_component(FILENAME ${SHADER} NAME_WE)
    add_custom_command(
        OUTPUT ${SHADER_BINARY_DIR}/${FILENAME}.spv
        COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${SHADER} -o ${SHADER_BINARY_DIR}/${FILENAME}.spv
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${SHADER_BINARY_DIR}/${FILENAME}.spv
            ${SHADER_FINAL_DIR}/${FILENAME}.spv
        DEPENDS ${SHADER} ${SHADER_BINARY_DIR}
        COMMENT "[glslc]: Compiling ${FILENAME}")
    list(APPEND SPV_SHADERS ${SHADER_BINARY_DIR}/${FILENAME}.spv)
endforeach()

add_custom_target(shaders ALL DEPENDS ${SPV_SHADERS})
# add_dependencies(application shaders)

target_link_libraries(application PRIVATE engine)
