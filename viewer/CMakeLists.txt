cmake_minimum_required(VERSION 2.8)
set(CMAKE_CXX_FLAGS "-O3 -std=c++14 -Wall -Wextra -pedantic")
file(GLOB VIEWSRC
	"*.h"
	"*.hpp"
	"*.cpp"
	)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt5Core)
find_package(Qt5Widgets)
find_package(Qt5Gui)
find_package(Qt5Quick)
find_package(Qt5OpenGL)
qt5_add_resources(RESOURCES res.qrc)

add_executable(shipEscape ${VIEWSRC} ${RESOURCES})
qt5_use_modules(shipEscape Quick Core Gui Opengl)
