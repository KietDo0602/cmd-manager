cmake_minimum_required(VERSION 3.5)
project(CmdRunnerQt)

# Enable Qt's automatic MOC (Meta-Object Compiler)
set(CMAKE_AUTOMOC ON)

# Find Qt5 Widgets
find_package(Qt5 COMPONENTS Widgets REQUIRED)

# Define the executable
add_executable(CmdRunnerQt
    main.cpp
    mainwindow.cpp
    mainwindow.h
    terminaldialog.cpp
    terminaldialog.h
)

# Link Qt5 libraries
target_link_libraries(CmdRunnerQt Qt5::Widgets)

# Install the executable
install(TARGETS CmdRunnerQt
    RUNTIME DESTINATION bin        # Windows & Linux
    BUNDLE DESTINATION .           # macOS .app bundles
    COMPONENT Runtime
)

# Optional: Install headers (if needed for development)
install(FILES
    mainwindow.h
    terminaldialog.h
    DESTINATION include
)

# Optional: Install documentation
install(FILES README.md
    DESTINATION share/doc/CmdRunnerQt
)

# Include system libraries for packaging
include(InstallRequiredSystemLibraries)

# CPack configuration for cross-platform installers
set(CPACK_PACKAGE_NAME "CmdRunnerQt")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_CONTACT "yourname@example.com")

# Platform-specific packaging
if(WIN32)
    set(CPACK_GENERATOR "NSIS")       # Windows installer
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop")  # macOS .dmg
elseif(UNIX)
    set(CPACK_GENERATOR "DEB")        # Linux .deb package
endif()

include(CPack)
