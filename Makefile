CC = gcc
CFLAGS = -Wall -pedantic -std=c99 -I$(shell pg_config --includedir)
LDFLAGS = -lmicrohttpd -lpq

OBJECTS = $(filter-out main.o,$(patsubst %.c,%.o,$(wildcard *.c)))
OTHER = strconst.h
BIN = pgdoctor
BIN_TEST = $(BIN)_test
CFG_FILE = $(BIN).cfg
PREFIX = /usr/bin

ifdef DEBUG
CFLAGS += -DDEBUG -g
else
CFLAGS += -O3
endif

VALGRIND_EXISTS := $(shell valgrind --version 2>/dev/null)
ifdef VALGRIND_EXISTS
	VALGRIND = valgrind --leak-check=full
endif

$(BIN): main.c $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $(BIN) $(LDFLAGS)

%.o: %.c %.h $(OTHER)
	$(CC) -c $(CFLAGS) $< -o $@

install: $(BIN)
	install -m 0755 $(BIN) $(PREFIX)
	install -m 0600 $(CFG_FILE) /etc

uninstall:
	rm -f $(PREFIX)/$(BIN)
	rm -f /etc/$(CFG_FILE)

clean:
	rm -fr $(BIN) *.o

check: tests/test_custom_checks.c $(OBJECTS)
	$(CC) $(CFLAGS)  $^ -o $(BIN_TEST) $(LDFLAGS) -lcheck -pthread -lrt -lm
	$(VALGRIND) ./$(BIN_TEST)
	rm -f ./$(BIN_TEST)
