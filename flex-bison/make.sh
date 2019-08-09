bison -d -o parser.cpp parser.y
flex -o tokens.cpp tokens.l parser.hpp

g++ -w -c  `llvm-config-6.0 --cppflags` -std=c++11 -o parser.o parser.cpp
g++ -w -c  `llvm-config-6.0 --cppflags` -std=c++11 -o codegen.o codegen.cpp
g++ -w -c  `llvm-config-6.0 --cppflags` -std=c++11 -o main.o main.cpp
g++ -w -c  `llvm-config-6.0 --cppflags` -std=c++11 -o tokens.o tokens.cpp

clang++-6.0 -O3 -Wno-return-type -std=c++11 `llvm-config-6.0 --cppflags --ldflags --libs core --system-libs` -o toy main.o parser.o tokens.o codegen.o

rm codegen.o main.o parser.cpp parser.hpp parser.o tokens.cpp tokens.o

echo Toy File Generated!