CC = gcc
CFLAGS = -Wall -O2 -I. 
LDFLAGS = -ldbus-1 -lbluetooth -lpthread -lcurl

SRC = blue_device_finder.c \
      cloud_forward.c \
      ipc_api.c \
      Main_App_Iot_Bluetooth.c

OBJ = $(SRC:.c=.o)

TARGET = iot_app

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
