NAME = cpu-hungry
CFLAGS = -lpthread

.PHONY : all $(NAME) clean

all : $(NAME)

$(NAME) : $(NAME).c
	gcc -o $@ $< $(CFLAGS)

clean:
	rm -f $(NAME)