CC = gcc
CFLAGS = -Wall -Wextra
TARGET = shell
SRCS = shell.c
OBJS = $(SRCS:.c=.o)

# Grup Üyeleri:
#[Hanan Haidar-B211210581]
#[Reem Almadhagi-Y245012049]
#[Dania Almassri-B221210569]
#[tasnim abdulhay-B211210585]
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGET)
