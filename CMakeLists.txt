project(kded_brightness)

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
set(QT_MIN_VERSION "5.7.0")

find_package(ECM 0.0.11 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMInstallIcons)
include(ECMMarkAsTest)
include(ECMMarkNonGuiExecutable)
include(ECMOptionalAddSubdirectory)
include(ECMQtDeclareLoggingCategory)
include(FeatureSummary)
include(CheckIncludeFiles)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    DBus
    Sensors
)

set(KF5_MIN_VERSION "5.24.0")
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    CoreAddons
    DBusAddons
)

find_package(KDED ${KF5_MIN_VERSION} CONFIG REQUIRED)

set(CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules
    ${CMAKE_MODULE_PATH}
)
set(CMAKE_INCLUDE_CURRENT_DIR true)

SET(SRCS
    screen_light_manager.cpp
    screen_light_manager_dbus_connector.cpp
)

add_library(kded_brightness MODULE
            ${SRCS}
)
target_link_libraries(kded_brightness
		      KF5::CoreAddons
		      KF5::DBusAddons
                      Qt5::Sensors
)

install(FILES kded_brightness.desktop
        DESTINATION ${SERVICES_INSTALL_DIR}/kded
        RENAME kded_brightness.desktop
)

install(TARGETS kded_brightness
        DESTINATION ${PLUGIN_INSTALL_DIR}
)
