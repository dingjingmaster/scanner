add_executable(test-utils.run test-utils.cc ../app/pf/utils.cc ../app/pf/proc-inject.cc)
target_link_libraries(test-utils.run PUBLIC ${GIO_LIBRARIES} ${QT5_LIBRARIES} ${OPENCC_LIBRARIES})
target_include_directories(test-utils.run PUBLIC ${GIO_INCLUDE_DIRS} ${QT5_INCLUDE_DIRS} ${OPENCC_INCLUDE_DIRS})
target_compile_options(test-utils.run PUBLIC -fPIC)
