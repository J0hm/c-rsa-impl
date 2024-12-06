CC = gcc
CFLAGS = -Wall
LDFLAGS =
OBJFILES = main.o
TARGET = cli

all: $(TARGET)

$(TARGET): $(OBJFILES)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJFILES) $(LDFLAGS)

clean:
	rm -f $(OBJFILES) $(TARGET) *~