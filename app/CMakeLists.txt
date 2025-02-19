add_executable(andsec-scanner main.cpp
        pf/task-base.cpp pf/task-base.h
        pf/policy-base.cpp pf/policy-base.h
        pf/task-manager.cpp pf/task-manager.h
)
target_include_directories(andsec-scanner PUBLIC ${QT5_INCLUDE_DIRS})
target_link_libraries(andsec-scanner PUBLIC ${QT5_LIBRARIES})
target_compile_options(andsec-scanner PUBLIC -fPIC)