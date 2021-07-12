SERVER = server
CLIENT = client

.PHONY : all $(SERVER) $(CLIENT) clean

all : $(SERVER) $(CLIENT)

$(SERVER) : $(SERVER).c
	gcc -o $@ $<

$(CLIENT) : $(CLIENT).c
	gcc -o $@ $<

clean:
	rm -f $(SERVER) $(CLIENT)