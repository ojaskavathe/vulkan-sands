project(application)
add_executable(application Application.cpp)

add_custom_command(
  TARGET application POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/res
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/res
  COMMENT "Copying res to output directory: ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}")

target_link_libraries(application PRIVATE renderer)