all:container

container:container.c
	gcc container.c -o container
	sudo ./container mycontainer ../rootfs

server:server.c
	gcc server.c -o server

client:client.c
	gcc client.c -o client

threads-cpu: cpu-hungry.c
	gcc -o threads-cpu cpu-hungry.c -lpthread

clean:
	rm container