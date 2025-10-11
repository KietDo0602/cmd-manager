cmake_minimum_required(VERSION 3.5)
project(CMD-Manager)

set(CMAKE_AUTOMOC ON)

find_package(Qt5 COMPONENTS Widgets Core Gui Svg REQUIRED)

# Add subdirectories
add_subdirectory(src)

# Install documentation
install(FILES README.md DESTINATION share/doc/CMD-Manager)

# Packaging
include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_NAME "CMD-Manager")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_CONTACT "kietdo0602@gmail.com")

if(WIN32)
    set(CPACK_GENERATOR "NSIS")
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
elseif(UNIX)
    set(CPACK_GENERATOR "DEB")
endif()

include(CPack)
