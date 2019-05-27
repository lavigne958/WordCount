CFLAGS = -O2 -Wall -Werror -fstack-protector-strong
LDFLAGS = -pthread

TARGET = mapred

OBJS = src/main.o \
	src/map.o \
	src/reduce.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $^

%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $<

clean:
	rm -f $(OBJS)
	rm -f $(TARGET)
