CC = gcc
CFLAGS = -c
LKFLAGS = -lseccomp
CONTAINER = container
CONTAINER_FLAGS = mycontainer ../../rootfs
OBJS_FILE = cap.o cgroup.o comp.o container.o

.PHONY : all everything $(CONTAINER)

all : everything

everything : clean $(OBJS_FILE) $(CONTAINER)

clean :
	rm -f *.o
	rm -f $(CONTAINER)

%.o : %.c
	$(CC) $(CFLAGS) -o $@ $<

$(CONTAINER) : $(OBJS_FILE)
	$(CC) -o $@ $^ $(LKFLAGS)

run :
	sudo ./$(CONTAINER) $(CONTAINER_FLAGS)
