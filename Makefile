include libpressf.mk

CC = gcc
CFLAGS = -Wall -pedantic -g -std=c89 -DPF_TEST_BUILD=1
TARGET = libpressf-tests
SOURCES = $(PRESS_F_SOURCES) main.c
HEADERS = $(PRESS_F_HEADERS)

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

run: $(TARGET)
	./$(TARGET)
	@if [ $$? -eq 0 ]; then \
		echo "All tests passed!"; \
	else \
		echo "Tests failed..."; \
		exit 1; \
	fi

clean:
	rm -f $(TARGET) *.o

.PHONY: clean run
