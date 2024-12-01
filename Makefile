# main: main.c
# 	$(CC) src/main.c -o main -Wall -Wextra -pedantic -std=c99

clear:
	rm -f main.obj

build: clear
	$(CC) src/main.c -o main.obj -Wall -Wextra -std=c99


run: build
	./main.obj