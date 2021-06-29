.PHONY: clean

all:container

container:container.c
	gcc container.c -o container -lcap -lseccomp

server:server.c
	gcc server.c -o server

client:client.c
	gcc client.c -o client

threads-cpu: cpu-hungry.c
	gcc -o threads-cpu cpu-hungry.c -lpthread

run:
	sudo ./container mycontainer ../rootfs

clean:
	rm container