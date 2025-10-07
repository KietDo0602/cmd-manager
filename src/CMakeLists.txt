# Find lrelease tool
find_program(Qt5_LRELEASE_EXECUTABLE lrelease REQUIRED)

# Collect source and header files
file(GLOB_RECURSE SRC_FILES
    main.cpp
    ui/*.cpp
    core/*.cpp
)

file(GLOB_RECURSE HEADER_FILES
    ui/*.h
    core/*.h
)

# Compile Qt resource file
qt5_add_resources(RESOURCE_FILES ../assets/resources.qrc)

# Define the executable
add_executable(CMD-Manager ${SRC_FILES} ${HEADER_FILES} ${RESOURCE_FILES})

# Include directories
target_include_directories(CMD-Manager PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/core
)

# Link Qt libraries
target_link_libraries(CMD-Manager Qt5::Widgets)

# Install binary
install(TARGETS CMD-Manager
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION .
    COMPONENT Runtime
)

# Optional: Install headers
install(FILES ${HEADER_FILES} DESTINATION include)

# List of .ts files
set(TS_FILES
    ${CMAKE_SOURCE_DIR}/assets/translations/cmd_manager_fr.ts
    ${CMAKE_SOURCE_DIR}/assets/translations/cmd_manager_es.ts
    ${CMAKE_SOURCE_DIR}/assets/translations/cmd_manager_zh.ts
    ${CMAKE_SOURCE_DIR}/assets/translations/cmd_manager_ru.ts
    ${CMAKE_SOURCE_DIR}/assets/translations/cmd_manager_vi.ts
)

# Output directory for .qm files
set(QM_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/assets/translations)

# Generate .qm files from .ts files
set(QM_FILES "")
foreach(TS_FILE ${TS_FILES})
    get_filename_component(TS_NAME ${TS_FILE} NAME_WE)
    set(QM_FILE ${QM_OUTPUT_DIR}/${TS_NAME}.qm)

    add_custom_command(
        OUTPUT ${QM_FILE}
        COMMAND ${Qt5_LRELEASE_EXECUTABLE} ${TS_FILE} -qm ${QM_FILE}
        DEPENDS ${TS_FILE}
        COMMENT "Generating ${QM_FILE}"
        VERBATIM
    )

    list(APPEND QM_FILES ${QM_FILE})
endforeach()

# Create a custom target for translations
add_custom_target(translations ALL DEPENDS ${QM_FILES})

# Ensure translations are built before the main executable
add_dependencies(CMD-Manager translations)
