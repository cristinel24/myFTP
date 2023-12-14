CC := g++ -Wall
CFLAGS := -pthread -lsqlite3

SERVER_SRC := src/server.cpp \
			  src/modules/Database.cpp

COMMON_SRC := src/modules/utils.cpp \
			  src/modules/Logger.cpp \
			  src/modules/FileManager.cpp
			  
CLIENT_SRC := src/client.cpp

all: client server

client:
	$(CC) $(CLIENT_SRC) $(COMMON_SRC) $(CFLAGS) -o client

server: 
	$(CC) $(SERVER_SRC) $(COMMON_SRC)  $(CFLAGS) -o server

clean:
	rm -f $(CLIENT_OBJ) $(SERVER_OBJ) $(COMMON_OBJ) client server