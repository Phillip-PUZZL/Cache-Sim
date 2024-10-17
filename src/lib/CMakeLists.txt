add_library(
        io
        headers/io.h
        sources/io.c
)


target_link_directories(io PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")