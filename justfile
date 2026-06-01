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
