add_executable(test-utils.run test-utils.cc ../app/pf/utils.cc)
target_link_libraries(test-utils.run PUBLIC ${QT5_LIBRARIES} ${OPENCC_LIBRARIES})
target_include_directories(test-utils.run PUBLIC ${QT5_INCLUDE_DIRS} ${OPENCC_INCLUDE_DIRS})
target_compile_options(test-utils.run PUBLIC -fPIC)
