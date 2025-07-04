add_executable(andsec-scanner main.cpp
        pf/utils.cc pf/utils.h
        pf/data-base.cc pf/data-base.h
        pf/task-base.cpp pf/task-base.h
        pf/policy-base.cpp pf/policy-base.h
        pf/task-manager.cpp pf/task-manager.h
        pf/policy-filter.cc pf/policy-filter.h
        pf/general-setting.cc pf/general-setting.h

        tika-wrap/src/java-env.cpp tika-wrap/src/java-env.h

        hs-wrap/src/regex-matcher.cpp hs-wrap/src/regex-matcher.h

        sqlite3-wrap/src/sqlite3-wrap.cc sqlite3-wrap/src/sqlite3-wrap.h
)
target_include_directories(andsec-scanner PUBLIC
        ${HS_INCLUDE_DIRS}
        ${QT5_INCLUDE_DIRS}
        ${GIO_INCLUDE_DIRS}
        ${OPENCC_INCLUDE_DIRS}
        ${SQLITE3_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/app
        ${JAVA_HOME}/include
        ${JAVA_HOME}/include/linux
        ${CMAKE_SOURCE_DIR}/app/hs-wrap/src
        ${CMAKE_SOURCE_DIR}/app/sqlite3-wrap/src
)
target_link_libraries(andsec-scanner PUBLIC
        ${HS_LIBRARIES}
        ${QT5_LIBRARIES}
        ${GIO_LIBRARIES}
        ${OPENCC_LIBRARIES}
        ${SQLITE3_LIBRARIES}
        -L${JAVA_HOME}/lib/server -ljvm
        -Wl,-rpath=${JAVA_HOME}/lib/server
)
target_compile_options(andsec-scanner PUBLIC -fPIC)
target_compile_definitions(andsec-scanner PUBLIC -D JAVA_HOME=\\"${JAVA_HOME}\\")
