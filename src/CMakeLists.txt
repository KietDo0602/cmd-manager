# Collect all source files
file(GLOB_RECURSE SRC_FILES
    ui/*.cpp
    core/*.cpp
    main.cpp
)

file(GLOB_RECURSE HEADER_FILES
    ui/*.h
    core/*.h
)

# Compile Qt resource file
qt5_add_resources(RESOURCE_FILES ../assets/resources.qrc)

add_executable(CMD-Manager ${SRC_FILES} ${HEADER_FILES} ${RESOURCE_FILES})

target_include_directories(CMD-Manager PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ui ${CMAKE_CURRENT_SOURCE_DIR}/core)

target_link_libraries(CMD-Manager Qt5::Widgets)

# Install binary
install(TARGETS CMD-Manager
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION .
    COMPONENT Runtime
)

# Optional: Install headers
install(FILES ${HEADER_FILES} DESTINATION include)
