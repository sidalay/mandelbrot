add_library(compile_options INTERFACE)

target_compile_features(compile_options INTERFACE cxx_std_20)
target_compile_options(compile_options INTERFACE
    -luser32 -lgdi32 -lopengl32 -lgdiplus -lShlwapi -ldwmapi -lstdc++fs -static -march=native -mavx
)

add_executable(mandelbrot mandelbrot.cpp)

target_link_libraries(mandelbrot PRIVATE olcPixelGameEngine -luser32 -lgdi32 -lopengl32 -lgdiplus -lShlwapi -ldwmapi -lstdc++fs)