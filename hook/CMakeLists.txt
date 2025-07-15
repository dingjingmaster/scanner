# hook
add_library(dlp-hook-main SHARED
        hook-common.h
        hook-main.c
)
target_compile_options(dlp-hook-main PUBLIC -fPIC)
target_link_libraries(dlp-hook-main PUBLIC hook-func common)
set_target_properties(dlp-hook-main PROPERTIES PREFIX "")
target_include_directories(dlp-hook-main PUBLIC ${CMAKE_SOURCE_DIR}/app)

add_library(hook-gdk-screenshot SHARED
        hook-common.h hook-common.c
        hook-gdk-screenshot.c
)

target_compile_options(hook-gdk-screenshot PUBLIC -fPIC)
set_target_properties(hook-gdk-screenshot PROPERTIES PREFIX "")
target_link_libraries(hook-gdk-screenshot PUBLIC hook-func common)
target_include_directories(hook-gdk-screenshot PUBLIC ${CMAKE_SOURCE_DIR}/app)

add_library(hook-ld_preload SHARED
        hook-ld_preload.c
)

target_compile_options(hook-ld_preload PUBLIC -fPIC)
set_target_properties(hook-ld_preload PROPERTIES PREFIX "")

