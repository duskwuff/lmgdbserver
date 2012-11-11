CFLAGS = -Wall

CFLAGS += $(shell pkg-config --cflags libusb-1.0)
LDLIBS += $(shell pkg-config --libs libusb-1.0)

TARGET = lmgdbserver
OBJECTS = lmgdbserver.o

default: $(TARGET)
clean:
	rm -f $(TARGET) $(OBJECTS)
$(TARGET): $(OBJECTS)

