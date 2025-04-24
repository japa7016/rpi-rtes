CC       := gcc
CFLAGS   := -O3 -Wall -pthread
LDFLAGS  := 
LIBS     := -lpthread

SRCS     := scheduler.c
OBJS     := $(SRCS:.c=.o)
TARGET   := scheduler

.PHONY: all clean distclean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	-rm -f $(OBJS)

distclean: clean
	-rm -f $(TARGET)
