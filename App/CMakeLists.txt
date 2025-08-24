cmake_minimum_required(VERSION 3.16)

project(App VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Core)

# Adicionado os novos arquivos strokedlabel.cpp e strokedlabel.h
set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    cputemperature.cpp
    cputemperature.h
    splashscreen.cpp
    splashscreen.h
    splashscreen.ui
    strokedlabel.cpp
    strokedlabel.h
    resources.qrc
    app.rc
)

if(QT_VERSION_MAJOR GREATER_EQUAL 6)
    qt_add_executable(App
        MANUAL_FINALIZATION
        WIN32
        ${PROJECT_SOURCES}
    )
else()
    add_executable(App WIN32 ${PROJECT_SOURCES} ${PROJECT_RESOURCES})
endif()

target_link_libraries(App PRIVATE Qt6::Widgets Qt6::Core)


set(HELPER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/helper/TempReader.exe
    ${CMAKE_CURRENT_SOURCE_DIR}/helper/LibreHardwareMonitorLib.dll
)

add_custom_command(TARGET App POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${HELPER_FILES}
    $<TARGET_FILE_DIR:App>
)

if(WIN32)
    target_link_libraries(App PRIVATE ole32 oleaut32 wbemuuid)
endif()

if(QT_VERSION VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.App)
endif()
set_target_properties(App PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist"
    MACOSX_BUNDLE_ICON_FILE icns/AppIcon.icns
    WIN32_EXECUTABLE TRUE
    WIN32_APPLICATION_ICON "images/logo.ico"
)


include(GNUInstallDirs)
install(TARGETS App
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(App)
endif()
