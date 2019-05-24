CFLAGS = -O0 -ggdb3 -Wall
LDFLAGS = "-lpthread"

TARGET = mapred

OBJS = main.o

all: $(TARGET)

$(TARGET): src/$(OBJS)
	$(CC) -o $@ $(LDFLAGS) $^

%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $<

clean:
	rm -f $(OBJS)
	rm -f $(TARGET)