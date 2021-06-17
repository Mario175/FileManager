cmake_minimum_required(VERSION 3.5)

project(FileManager LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:10485760,655360")

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check https://doc.qt.io/qt/deployment-android.html for more information.
# They need to be set before the find_package( ...) calls below.

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

set(PROJECT_SOURCES
        main.cpp
        MainWindow.cpp
        MainWindow.hpp
        mainwindow.ui
        progressdialog.ui
        FileMoverDelegate.cpp
        FileMoverDelegate.hpp
        ItemContextMenu.cpp
        ItemContextMenu.hpp
        OrderedFileSystemModel.cpp
        OrderedFileSystemModel.hpp
        FileProgressDialog.cpp
        FileProgressDialog.hpp
        NewDirDialog.cpp
        NewDirDialog.hpp
        NewFileDialog.cpp
        NewFileDialog.hpp
        FileTableView.cpp
        FileTableView.hpp
        FileTabSelector.cpp
        FileTabSelector.hpp
        QuickAccessBar.cpp
        QuickAccessBar.hpp
        #CThread.cpp
        #CThread.hpp
        Resources/resource.qrc
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(FileManager
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(FileManager SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(FileManager
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(FileManager PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

