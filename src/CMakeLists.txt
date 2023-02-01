add_library(compile_options INTERFACE)

target_compile_features(compile_options INTERFACE cxx_std_20)
# target_compile_options(compile_options INTERFACE
#     -march=native -Wall -Wextra -Wpedantic -O0
# )

add_executable(mandelbrot mandelbrot.cpp)

target_link_libraries(mandelbrot PRIVATE olcPixelGameEngine compile_options)