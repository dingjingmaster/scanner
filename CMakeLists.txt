cmake_minimum_required(VERSION 3.15)
project(andsec-scanner LANGUAGES C CXX)

set(DEBUG ON)

set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_PATCH 0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(JAVA_HOME "${CMAKE_SOURCE_DIR}/data/java")

set(ENV{PKG_CONFIG_PATH} "/usr/local/lib64/pkgconfig/:/usr/local/lib/pkgconfig:/opt/dev/qt-5.15.15/lib/pkgconfig/:${ENV_CONFIG_PATH}")

add_definitions(
        -w -g
        -D QT_NO_KEYWORDS
        -D QT_MESSAGELOGCONTEXT
        -D DDEBUG

        -D G_LOG_TAG=\\"${PROJECT_NAME}\\"
        -D PACKAGE_NAME=\\"${PROJECT_NAME}\\"
        -D G_LOG_DOMAIN=\\"${PROJECT_NAME}\\"
        -D INSTALL_PATH=\\"/usr/local/andsec\\"
        -D DATA_DIR=\\"/var/lib/andsec/scanner/\\"
        -D C_LOG_DIR=\\"/usr/local/andsec/log/\\"
        -D PACKAGE_VERSION=\\"${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}\\")

find_package (PkgConfig)
find_package(Qt5 COMPONENTS Core Network REQUIRED)

pkg_check_modules(HS REQUIRED libhs)
pkg_check_modules(GIO REQUIRED gio-2.0)
pkg_check_modules(SQLITE3 REQUIRED sqlite3)
pkg_check_modules(QT5 REQUIRED Qt5Core Qt5Network)

if (EXISTS /opt/dev/qt-5.15.15)
    set(Qt5Core_MOC_EXECUTABLE /opt/dev/qt-5.15.15/bin/moc)
    set(QT5_LIBRARIES "-L/opt/dev/qt-5.15.15/lib -l${QT5_LIBRARIES}")
endif ()

if (EXISTS /usr/lib/x86_64-linux-gnu/libhs.a)
    set(HS_LIBRARIES "/usr/lib/x86_64-linux-gnu/libhs.a")
endif ()

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${GIO_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/app)
include_directories(${CMAKE_SOURCE_DIR}/hook-func/3thrd/capstone/include)
include_directories(${CMAKE_SOURCE_DIR}/hook-func/3thrd/capstone/include/capstone)

add_definitions(-D _GNU_SOURCE)

cmake_host_system_information(RESULT OS QUERY OS_NAME)
cmake_host_system_information(RESULT RELEASE QUERY OS_RELEASE)
execute_process(COMMAND uname -m OUTPUT_VARIABLE OS_ARCH OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND bash -c "cat /etc/os-release|grep -E '^NAME'|awk -F'=' '{print $2}'|sed 's/\"//g'|tr '\n' ' '|sed 's/ //g'" OUTPUT_VARIABLE DISTRIB)
if ("${DISTRIB}" STREQUAL "Gentoo")
    execute_process(COMMAND bash -c "/lib64/qt5/bin/qmake --version | grep -i 'qt version' | awk '{print $4}' | tr '\n' ' '|sed 's/ //g'" OUTPUT_VARIABLE CUR_QT_VERSION)
else ()
    execute_process(COMMAND bash -c "qmake --version | grep -i 'qt version' | awk '{print $4}' | tr '\n' ' '|sed 's/ //g'" OUTPUT_VARIABLE CUR_QT_VERSION)
endif ()

MESSAGE("")
MESSAGE(STATUS "OS name         : '${OS}'")
MESSAGE(STATUS "OS arch         : '${OS_ARCH}'")
MESSAGE(STATUS "OS distribute   : '${DISTRIB}'")
MESSAGE(STATUS "OS release      : '${RELEASE}'")
MESSAGE(STATUS "Qt5 includes    : '${QT5_INCLUDE_DIRS}'")
MESSAGE(STATUS "Qt5 libs        : '${QT5_LIBRARIES}'")
MESSAGE(STATUS "Hyper scan libs : '${HS_LIBRARIES}'")
MESSAGE("")

add_subdirectory(app)
add_subdirectory(test)
add_subdirectory(hook)
add_subdirectory(tools)
add_subdirectory(common)
add_subdirectory(opencc)
add_subdirectory(hook-func)
