CC = gcc
CFLAGS = -Wall -pedantic -std=c99 -I$(shell pg_config --includedir)
LDFLAGS = -lmicrohttpd -lpq

OBJECTS = $(filter-out main.o,$(patsubst %.c,%.o,$(wildcard *.c)))
OTHER = strconst.h
BIN = pgdoctor
BIN_TEST = $(BIN)_test

ifdef DEBUG
CFLAGS += -DDEBUG -g
else
CFLAGS += -O3
endif

$(BIN): main.c $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $(BIN) $(LDFLAGS)

%.o: %.c %.h $(OTHER)
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -fr $(BIN) *.o

check: tests/test_custom_checks.c $(OBJECTS)
	$(CC) $(CFLAGS)  $^ -o $(BIN_TEST) $(LDFLAGS) -lcheck -pthread -lrt -lm
	./$(BIN_TEST)
	rm -f ./$(BIN_TEST)
