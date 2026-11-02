cmake_minimum_required(VERSION 3.16)
project(CircuitGuardPro LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui Widgets Network REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets Network REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(NGSPICE REQUIRED ngspice)

set(PROJECT_SOURCES
    src/ai/AiDiagnosticExplainer.h
    src/cli/main.cpp
    src/compiler/CircuitCompiler.h
    src/core/AltiumInternal.h
    src/core/AltiumParser.cpp
    src/core/AltiumParser.h
    src/core/AltiumText.cpp
    src/core/AltiumText.h
    src/core/AsciiParser.cpp
    src/core/BoardIndex.cpp
    src/core/BoardIndex.h
    src/core/BoardRender.cpp
    src/core/BoardRender.h
    src/core/BoardSvg.cpp
    src/core/Bom.cpp
    src/core/Bom.h
    src/core/Bundle.cpp
    src/core/Bundle.h
    src/core/Compare.cpp
    src/core/Compare.h
    src/core/CompoundFile.cpp
    src/core/CompoundFile.h
    src/core/CoreDataStructures.h
    src/core/Drc.cpp
    src/core/Drc.h
    src/core/Export.cpp
    src/core/Export.h
    src/core/FabEngine.cpp
    src/core/FabEngine.h
    src/core/Inflate.cpp
    src/core/Inflate.h
    src/core/LayerStyle.h
    src/core/Library.cpp
    src/core/Library.h
    src/core/Model.h
    src/core/Netlist.cpp
    src/core/Netlist.h
    src/core/NetlistProviders.h
    src/core/Recent.cpp
    src/core/Recent.h
    src/core/Search.cpp
    src/core/Search.h
    src/core/Step.cpp
    src/core/Step.h
    src/core/ThreeD.cpp
    src/core/ThreeD.h
    src/editor/CompilerWorkspaceWidget.h
    src/editor/SchematicScene.h
    src/gui/BoardView.cpp
    src/gui/BoardView.h
    src/gui/Icons.cpp
    src/gui/Icons.h
    src/gui/MainWindow.cpp
    src/gui/MainWindow.h
    src/gui/SchematicView.cpp
    src/gui/SchematicView.h
    src/gui/main.cpp
    src/simulator/SpiceSimulator.h
)

add_executable(circuitguard_pro ${PROJECT_SOURCES})

target_include_directories(circuitguard_pro PRIVATE
    src
    ${NGSPICE_INCLUDE_DIRS}
)

target_link_libraries(circuitguard_pro PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    ${NGSPICE_LIBRARIES}
)

target_link_directories(circuitguard_pro PRIVATE ${NGSPICE_LIBRARY_DIRS})
