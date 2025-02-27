add_executable(andsec-scanner main.cpp
        pf/utils.cc pf/utils.h
        pf/data-base.cc pf/data-base.h
        pf/task-base.cpp pf/task-base.h
        pf/policy-base.cpp pf/policy-base.h
        pf/task-manager.cpp pf/task-manager.h
        pf/policy-filter.cc pf/policy-filter.h

        sqlite3-wrap/src/sqlite3-wrap.cc sqlite3-wrap/src/sqlite3-wrap.h
)
target_include_directories(andsec-scanner PUBLIC ${QT5_INCLUDE_DIRS} ${SQLITE3_INCLUDE_DIRS} ${GIO_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/app)
target_link_libraries(andsec-scanner PUBLIC ${QT5_LIBRARIES} ${SQLITE3_LIBRARIES} ${GIO_LIBRARIES})
target_compile_options(andsec-scanner PUBLIC -fPIC)