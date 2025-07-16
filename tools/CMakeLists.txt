add_executable(show-task
        show-task.cc
        ../app/pf/utils.cc
        ../app/pf/data-base.cc
        ../app/pf/proc-inject.cc
        ../common/proc-list.c
        ../sqlite3-wrap/src/sqlite3-wrap.cc
)
target_compile_options(show-task PUBLIC -fPIC)
target_include_directories(show-task PUBLIC
        ${QT5_INCLUDE_DIRS}
        ${GIO_INCLUDE_DIRS}
        ${OPENCC_INCLUDE_DIRS}
        ${SQLITE3_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/app/pf
        ${CMAKE_SOURCE_DIR}/opencc/src
        ${CMAKE_SOURCE_DIR}/hs-wrap/src
        ${CMAKE_SOURCE_DIR}/sqlite3-wrap/src
)
target_link_libraries(show-task PUBLIC -ldl
        libopencc
        ${QT5_LIBRARIES}
        ${GIO_LIBRARIES}
        ${OPENCC_LIBRARIES}
        ${SQLITE3_LIBRARIES}
)
set_target_properties(show-task PROPERTIES AUTOUIC ON AUTOMOC ON AUTORCC ON)

add_executable(show-result
        show-result.cc
        ../app/pf/utils.cc
        ../app/pf/data-base.cc
        ../app/pf/proc-inject.cc
        ../common/proc-list.c
        ../sqlite3-wrap/src/sqlite3-wrap.cc
)
target_compile_options(show-result PUBLIC -fPIC)
target_include_directories(show-result PUBLIC
        ${QT5_INCLUDE_DIRS}
        ${GIO_INCLUDE_DIRS}
        ${OPENCC_INCLUDE_DIRS}
        ${SQLITE3_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/app/pf
        ${CMAKE_SOURCE_DIR}/opencc/src
        ${CMAKE_SOURCE_DIR}/hs-wrap/src
        ${CMAKE_SOURCE_DIR}/sqlite3-wrap/src
)
target_link_libraries(show-result PUBLIC -ldl
        libopencc
        ${QT5_LIBRARIES}
        ${GIO_LIBRARIES}
        ${OPENCC_LIBRARIES}
        ${SQLITE3_LIBRARIES}
)
set_target_properties(show-result PROPERTIES AUTOUIC ON AUTOMOC ON AUTORCC ON)

add_executable(parse-policy parse-policy.cc)
target_compile_options(parse-policy PUBLIC -fPIC)
target_include_directories(parse-policy PUBLIC
        ${QT5_INCLUDE_DIRS}
        ${GIO_INCLUDE_DIRS}
        ${OPENCC_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/opencc/src
)
target_link_libraries(parse-policy PUBLIC -ldl
        libopencc
        ${QT5_LIBRARIES}
        ${GIO_LIBRARIES}
        ${OPENCC_LIBRARIES}
)
set_target_properties(parse-policy PROPERTIES AUTOUIC ON AUTOMOC ON AUTORCC ON)

add_executable(rule-id rule-id.cc
        ../app/pf/utils.cc
        ../app/pf/data-base.cc
        ../app/pf/proc-inject.cc
        ../common/proc-list.c
        ../sqlite3-wrap/src/sqlite3-wrap.cc
)
target_compile_options(rule-id PUBLIC -fPIC)
target_include_directories(rule-id PUBLIC
        ${QT5_INCLUDE_DIRS}
        ${GIO_INCLUDE_DIRS}
        ${OPENCC_INCLUDE_DIRS}
        ${SQLITE3_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/app/pf
        ${CMAKE_SOURCE_DIR}/opencc/src
        ${CMAKE_SOURCE_DIR}/hs-wrap/src
        ${CMAKE_SOURCE_DIR}/sqlite3-wrap/src
)
target_link_libraries(rule-id PUBLIC -ldl
        -lQt5Core libopencc
        ${QT5_LIBRARIES}
        ${GIO_LIBRARIES}
        ${OPENCC_LIBRARIES}
        ${SQLITE3_LIBRARIES}
)
set_target_properties(rule-id PROPERTIES AUTOUIC ON AUTOMOC ON AUTORCC ON)
