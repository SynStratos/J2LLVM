clang++ -O3 -Wno-return-type -std=c++11 `llvm-config --cppflags --ldflags --libs core --system-libs` main.cpp -o main
