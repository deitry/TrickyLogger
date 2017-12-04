# компилятор
CC=g++

# флаги
CFLAGS=-std=c++11 -c -Wall -g

all: logger

logger: Run/main.o
	$(CC) Run/main.o -o Run/TrickyLogger

Run/main.o: src/main.cpp
	$(CC) $(CFLAGS) src/main.cpp -o Run/main.o


# удаление всех сгенерированных файлов

clean:
	rm -rf */*.o */TrickyLogger