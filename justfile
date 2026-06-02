basic:
    mkdir -p bin/plots
    g++ -std=c++20 -O2 -march=native examples/basic.cpp -o bin/basic
    ./bin/basic

large_dataset:
    mkdir -p bin/plots
    g++ -std=c++20 -O2 -march=native examples/large_dataset.cpp -o bin/large
    ./bin/large

multi_plot:
    mkdir -p bin/plots
    g++ -std=c++20 -O2 -march=native examples/multi_plot.cpp -o bin/multi
    ./bin/multi

stress:
    g++ -std=c++20 -O3 -mavx2 -mfma -ffast-math -fno-trapping-math -fno-math-errno -march=native stress/stresstest.cpp -o bin/stress
    ./bin/stress

download-catch2:
    #!/usr/bin/env sh
    if [ ! -f tests/catch_amalgamated.hpp ]; then
        curl -fsSL https://github.com/catchorg/Catch2/releases/download/v3.7.1/catch_amalgamated.hpp -o tests/catch_amalgamated.hpp
    fi
    if [ ! -f tests/catch_amalgamated.cpp ]; then
        curl -fsSL https://github.com/catchorg/Catch2/releases/download/v3.7.1/catch_amalgamated.cpp -o tests/catch_amalgamated.cpp
    fi

test: download-catch2
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build -- -j$(nproc)
    cd build && ctest --output-on-failure
