CFLAGS = -O0 -ggdb3 -Wall
LDFLAGS = "-lpthread"

TARGET = mapred

OBJS = src/main.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $^

%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $<

clean:
	rm -f $(OBJS)
	rm -f $(TARGET)