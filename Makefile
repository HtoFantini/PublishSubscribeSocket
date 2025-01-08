BIN_DIR = bin

all:
	@mkdir -p $(BIN_DIR)
	gcc -Wall -c server.c -o server.o -lm
	gcc -Wall -c client.c -o client.o -lm
	gcc -Wall server.o -o $(BIN_DIR)/server -lm
	gcc -Wall client.o -o $(BIN_DIR)/client -lm

clean:
	rm -rf $(BIN_DIR) *.o
