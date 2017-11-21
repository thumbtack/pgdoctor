CC = gcc
CFLAGS = -Wall -pedantic -std=c99 -I$(shell pg_config --includedir) -L$(shell pg_config --libdir)
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
	VALGRIND = valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes
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
	rm -fr $(BIN) $(BIN_TEST) *.o

test: tests/test_custom_checks.c $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $(BIN_TEST) $(LDFLAGS) -lcheck -lsubunit -pthread -lrt -lm
	./$(BIN_TEST)

valgrind: test
	CK_FORK=no $(VALGRIND) ./$(BIN_TEST)
