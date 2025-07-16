add_executable(test-utils.run test-utils.cc ../app/pf/utils.cc ../common/utils.c ../app/pf/proc-inject.c)
target_link_libraries(test-utils.run PUBLIC
        ${GIO_LIBRARIES} ${QT5_LIBRARIES} ${OPENCC_LIBRARIES} -ldl common libopencc
)
target_include_directories(test-utils.run PUBLIC ${GIO_INCLUDE_DIRS} ${QT5_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/opencc/src)
target_compile_options(test-utils.run PUBLIC -fPIC)
set_target_properties(test-utils.run PROPERTIES AUTOUIC ON AUTOMOC ON AUTORCC ON)
