build:
	mkdir -p build
	mkdir -p build/inter
	gcc -I ./glad/include ./glad/src/glad.c -c -o ./build/inter/glad.o
	g++ -g -I ./glad/include -I ./glm/include circles.cpp ./build/inter/glad.o -lglfw -ldl -o build/circles  

