SERVER = server
CLIENT = client
CLIENT_NAT = client_nat

.PHONY : all $(SERVER) $(CLIENT) $(CLIENT_NAT) clean

all : $(SERVER) $(CLIENT) $(CLIENT_NAT)

$(SERVER) : $(SERVER).c
	gcc -o $@ $<

$(CLIENT) : $(CLIENT).c
	gcc -o $@ $<

$(CLIENT_NAT) : $(CLIENT_NAT).c
	gcc -o $@ $<

clean:
	rm -f $(SERVER) $(CLIENT) $(CLIENT_NAT)