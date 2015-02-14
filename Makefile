all : client_op server_op

server_op : server/server.c
	gcc -w $< -o server/$@

client_op : client/client.c
	gcc -w $< -o client/$@

clean :
	rm server/server_op client/client_op

