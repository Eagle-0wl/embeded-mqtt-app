BIN:=app_mqtt
CC:=gcc
LDFLAGS:=-lmosquitto -lsqlite3
CPPFLAGS:=-I$(CURDIR)
SRC:=$(wildcard *.c)
OBJS:=$(SRC:.c=.o)

.PHONY: all clean

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) $(BIN) $(OBJS)
