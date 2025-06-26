add_executable(show-task
        show-task.cc
        ../app/pf/data-base.cc
        ../app/sqlite3-wrap/src/sqlite3-wrap.cc
)
target_compile_options(show-task PUBLIC -fPIC)
target_include_directories(show-task PUBLIC
        ${QT5_INCLUDE_DIRS}
        ${GIO_INCLUDE_DIRS}
        ${SQLITE3_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/app/pf
        ${CMAKE_SOURCE_DIR}/app/hs-wrap/src
        ${CMAKE_SOURCE_DIR}/app/sqlite3-wrap/src
)
target_link_libraries(show-task PUBLIC
        ${QT5_LIBRARIES}
        ${GIO_LIBRARIES}
        ${SQLITE3_LIBRARIES}
)
