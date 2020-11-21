#include_directories(${Qt5Widgets_INCLUDE_DIRS} ${QtQml_INCLUDE_DIRS} ${Qt5Widgets_INCLUDE_DIRS} ${Qt5Positioning_INCLUDE_DIRS})
#add_definitions(${Qt5Widgets_DEFINITIONS} ${QtQml_DEFINITIONS} ${Qt5Quick_DEFINITIONS} ${Qt5Positioning_DEFINITIONS})
find_package(Qt5 COMPONENTS Widgets Qml Quick Location REQUIRED)
qt5_add_resources(QT_RESOURCES qml.qrc nav_icons.qrc)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(PROJECT "CarUI")
project(${PROJECT})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++17 -fstrict-aliasing -pedantic-errors -pedantic -Wno-deprecated-declarations -Wno-unused-variable")
set(HEADERS "gps.h" "dummy_gps.h" "ublox_gps.h" "SigmaDSP/SigmaDSP.h" "SigmaDSP/parameters.h" "SigmaDSP/tinyxml2.h" "radio.h" "amp.h" "arduino.h" "mpd.h" "util.h" )
set(SOURCES "main.cpp" "dummy_gps.cpp" "ublox_gps.cpp" "gps.cpp" "SigmaDSP/SigmaDSP.cpp" "SigmaDSP/tinyxml2.cpp" "radio.cpp" "amp.cpp" "arduino.cpp" "mpd.cpp" "util.cpp")
source_group("Header Files" FILES ${HEADERS})
source_group("Source Files" FILES ${SOURCES})
add_executable(${PROJECT} ${HEADERS} ${SOURCES} ${QT_RESOURCES})
target_link_libraries(${PROJECT}
    Qt5::Qml
    Qt5::Quick
    Qt5::Widgets
    Qt5::Location
    pthread
    mpdclient
    )

install(TARGETS CarUI DESTINATION bin)
