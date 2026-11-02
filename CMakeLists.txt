cmake_minimum_required(VERSION 3.16)
project(CircuitGuardPro LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# پیدا کردن پکیج‌های Qt (پشتیبانی از Qt5 یا Qt6)
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui Widgets Network REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets Network REQUIRED)

# پیدا کردن کتابخانه Ngspice و pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(NGSPICE REQUIRED ngspice)

# فایل‌های سورس پروژه
set(PROJECT_SOURCES
    src/main.cpp
    src/core/CoreDataStructures.h
    src/core/NetlistProviders.h
    src/simulator/SpiceSimulator.h
    src/compiler/CircuitCompiler.h
    src/ai/AiDiagnosticExplainer.h
    src/editor/SchematicScene.h
    src/editor/CompilerWorkspaceWidget.h
)

add_executable(circuitguard_pro ${PROJECT_SOURCES})

# اینکلود پث‌ها
target_include_directories(circuitguard_pro PRIVATE
    src
    ${NGSPICE_INCLUDE_DIRS}
)

# لینک کردن کتابخانه‌ها
target_link_libraries(circuitguard_pro PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    ${NGSPICE_LIBRARIES}
)

target_link_directories(circuitguard_pro PRIVATE ${NGSPICE_LIBRARY_DIRS})
