# ==============================================================================
# Makefile for IoT Bluetooth Application
# ==============================================================================

# --- Compiler ---
CC = gcc

# --- Compiler Flags ---
CFLAGS = -Wall -Wextra -O2 -g -std=gnu11
# -Wall -Wextra: All warnings
# -O2: Optimization level 2
# -g: Debug symbols (for gdb)
# -std=gnu11: C11 with GNU extensions

# --- Include Paths ---
# Use pkg-config to get correct include paths
DBUS_CFLAGS = $(shell pkg-config --cflags dbus-1)
GLIB_CFLAGS = $(shell pkg-config --cflags glib-2.0)
JSON_CFLAGS = $(shell pkg-config --cflags json-c)

# Combine all includes
INCLUDES = -I. $(DBUS_CFLAGS) $(GLIB_CFLAGS) $(JSON_CFLAGS)

# --- Library Flags ---
DBUS_LIBS = $(shell pkg-config --libs dbus-1)
GLIB_LIBS = $(shell pkg-config --libs glib-2.0)
JSON_LIBS = $(shell pkg-config --libs json-c)
CURL_LIBS = $(shell pkg-config --libs libcurl)

# Combine all libraries
LDFLAGS = $(DBUS_LIBS) $(GLIB_LIBS) $(JSON_LIBS) $(CURL_LIBS) -lbluetooth -lpthread

# --- Source Files ---
SRC = blue_device_finder.c \
      cloud_forward.c \
      ipc_api.c \
      Main_App_Iot_Bluetooth.c

# --- Object Files ---
OBJ = $(SRC:.c=.o)

# --- Target Executable ---
TARGET = iot_app

# ==============================================================================
# Default Target
# ==============================================================================
.PHONY: all
all: check-deps $(TARGET)
	@echo "✓ Build successful!"
	@echo "Run with: ./$(TARGET)"

# ==============================================================================
# Check Dependencies
# ==============================================================================
.PHONY: check-deps
check-deps:
	@echo "Checking dependencies..."
	@pkg-config --exists dbus-1 || (echo "✗ DBus not found. Install: sudo apt install libdbus-1-dev" && exit 1)
	@pkg-config --exists glib-2.0 || (echo "✗ GLib not found. Install: sudo apt install libglib2.0-dev" && exit 1)
	@pkg-config --exists json-c || (echo "✗ json-c not found. Install: sudo apt install libjson-c-dev" && exit 1)
	@pkg-config --exists libcurl || (echo "✗ libcurl not found. Install: sudo apt install libcurl4-openssl-dev" && exit 1)
	@test -f /usr/include/bluetooth/bluetooth.h || (echo "✗ BlueZ not found. Install: sudo apt install libbluetooth-dev" && exit 1)
	@echo "✓ All dependencies found"

# ==============================================================================
# Build Target
# ==============================================================================
$(TARGET): $(OBJ)
	@echo "Linking $@..."
	$(CC) -o $@ $^ $(LDFLAGS)
	@echo "✓ Linked successfully"

# ==============================================================================
# Compile Source Files
# ==============================================================================
%.o: %.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# ==============================================================================
# Run Targets
# ==============================================================================
.PHONY: run
run: $(TARGET)
	@echo "Running $(TARGET)..."
	sudo ./$(TARGET)

.PHONY: run-debug
run-debug: CFLAGS += -DDEBUG -O0
run-debug: clean $(TARGET)
	@echo "Running in debug mode..."
	sudo gdb ./$(TARGET)

# ==============================================================================
# Clean
# ==============================================================================
.PHONY: clean
clean:
	@echo "Cleaning build artifacts..."
	rm -f $(OBJ) $(TARGET)
	@echo "✓ Clean complete"

.PHONY: distclean
distclean: clean
	@echo "Removing all generated files..."
	rm -f *~ *.log

# ==============================================================================
# Help
# ==============================================================================
.PHONY: help
help:
	@echo "Available targets:"
	@echo "  make              - Build the application"
	@echo "  make check-deps   - Check if all dependencies are installed"
	@echo "  make run          - Build and run the application"
	@echo "  make run-debug    - Build with debug flags and run with gdb"
	@echo "  make clean        - Remove build artifacts"
	@echo "  make distclean    - Remove all generated files"
	@echo "  make help         - Show this help message"

# ==============================================================================
# Debug: Print Variables
# ==============================================================================
.PHONY: print-vars
print-vars:
	@echo "CC: $(CC)"
	@echo "CFLAGS: $(CFLAGS)"
	@echo "INCLUDES: $(INCLUDES)"
	@echo "LDFLAGS: $(LDFLAGS)"
	@echo "SRC: $(SRC)"
	@echo "OBJ: $(OBJ)"
	@echo "TARGET: $(TARGET)"