CC = $(CROSS_COMPILE)gcc
CFLAGS = -g -Wall

TARGET = writer
SRCS = writer.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

clean:
	rm -f $(TARGET) $(OBJS)
