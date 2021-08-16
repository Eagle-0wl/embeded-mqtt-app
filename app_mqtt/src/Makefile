BIN:=app_mqtt
LDFLAGS:=-lmosquitto
SRC:=$(wildcard *.c)
OBJS:=$(SRC:.c=.o)

.PHONY: all clean

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS) 
clean:
	rm -f $(BIN) $(OBJS)
