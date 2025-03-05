PRESS_F_ROOT_DIR := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))

PRESS_F_SOURCES := \
  $(PRESS_F_ROOT_DIR)/src/debug.c \
  $(PRESS_F_ROOT_DIR)/src/dma.c \
  $(PRESS_F_ROOT_DIR)/src/emu.c \
  $(PRESS_F_ROOT_DIR)/src/font.c \
  $(PRESS_F_ROOT_DIR)/src/hle.c \
  $(PRESS_F_ROOT_DIR)/src/hw/2102.c \
  $(PRESS_F_ROOT_DIR)/src/hw/2114.c \
  $(PRESS_F_ROOT_DIR)/src/hw/3850.c \
  $(PRESS_F_ROOT_DIR)/src/hw/3851.c \
  $(PRESS_F_ROOT_DIR)/src/hw/beeper.c \
  $(PRESS_F_ROOT_DIR)/src/hw/f8_device.c \
  $(PRESS_F_ROOT_DIR)/src/hw/fairbug_parallel.c \
  $(PRESS_F_ROOT_DIR)/src/hw/hand_controller.c \
  $(PRESS_F_ROOT_DIR)/src/hw/schach_led.c \
  $(PRESS_F_ROOT_DIR)/src/hw/selector_control.c \
  $(PRESS_F_ROOT_DIR)/src/hw/system.c \
  $(PRESS_F_ROOT_DIR)/src/hw/vram.c \
  $(PRESS_F_ROOT_DIR)/src/input.c \
  $(PRESS_F_ROOT_DIR)/src/romc.c \
  $(PRESS_F_ROOT_DIR)/src/screen.c \
  $(PRESS_F_ROOT_DIR)/src/software.c

PRESS_F_HEADERS := \
  $(PRESS_F_ROOT_DIR)/src/config.h \
  $(PRESS_F_ROOT_DIR)/src/debug.h \
  $(PRESS_F_ROOT_DIR)/src/dma.h \
  $(PRESS_F_ROOT_DIR)/src/emu.h \
  $(PRESS_F_ROOT_DIR)/src/font.h \
  $(PRESS_F_ROOT_DIR)/src/hle.h \
  $(PRESS_F_ROOT_DIR)/src/hw/2102.h \
  $(PRESS_F_ROOT_DIR)/src/hw/2114.h \
  $(PRESS_F_ROOT_DIR)/src/hw/3850.h \
  $(PRESS_F_ROOT_DIR)/src/hw/3851.h \
  $(PRESS_F_ROOT_DIR)/src/hw/beeper.h \
  $(PRESS_F_ROOT_DIR)/src/hw/f8_device.h \
  $(PRESS_F_ROOT_DIR)/src/hw/fairbug_parallel.h \
  $(PRESS_F_ROOT_DIR)/src/hw/hand_controller.h \
  $(PRESS_F_ROOT_DIR)/src/hw/kdbug.h \
  $(PRESS_F_ROOT_DIR)/src/hw/schach_led.h \
  $(PRESS_F_ROOT_DIR)/src/hw/selector_control.h \
  $(PRESS_F_ROOT_DIR)/src/hw/system.h \
  $(PRESS_F_ROOT_DIR)/src/hw/vram.h \
  $(PRESS_F_ROOT_DIR)/src/input.h \
  $(PRESS_F_ROOT_DIR)/src/romc.h \
  $(PRESS_F_ROOT_DIR)/src/screen.h \
  $(PRESS_F_ROOT_DIR)/src/software.h \
  $(PRESS_F_ROOT_DIR)/src/types.h

PRESS_F_WAVETABLES_CC = gcc
PRESS_F_WAVETABLES_CFLAGS = -Wall -Wextra -O2
PRESS_F_WAVETABLES_TARGET = wavetables-bin
PRESS_F_WAVETABLES_SRC = $(PRESS_F_ROOT_DIR)/src/wavetables/main.c $(PRESS_F_ROOT_DIR)/src/wave.c

.PHONY: all run clean

all: $(PRESS_F_WAVETABLES_TARGET)

$(PRESS_F_WAVETABLES_TARGET): $(PRESS_F_WAVETABLES_SRC)
	cd $(PRESS_F_ROOT_DIR) && $(PRESS_F_WAVETABLES_CC) $(PRESS_F_WAVETABLES_CFLAGS) $(CFLAGS) -o $(PRESS_F_WAVETABLES_TARGET) $(PRESS_F_WAVETABLES_SRC) -lm && \
  ./$(PRESS_F_WAVETABLES_TARGET)
