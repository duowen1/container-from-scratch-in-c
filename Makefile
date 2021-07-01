.PHONY: clean

all:container

container:
	gcc -c ./src/container.c -o test.o
	gcc -c ./src/cap.c -o cap.o
	gcc -c ./src/cgroup.c -o cgroup.o
	gcc -o container test.o cap.o cgroup.o

server:server.c
	gcc server.c -o server

client:client.c
	gcc client.c -o client

threads-cpu: cpu-hungry.c
	gcc -o threads-cpu cpu-hungry.c -lpthread

run:
	sudo ./container mycontainer ../rootfs

clean:
	rm *.o
	rm container