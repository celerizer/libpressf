# libpressf

libpressf is an emulation library for Fairchild F8 family systems, used in the following projects:

- [Press F](https://github.com/celerizer/Press-F), an F8 emulator and debugger for desktop PCs
- [Press F libretro](https://github.com/celerizer/Press-F-libretro), a Channel F emulator for the libretro API
- [Press F Ultra](https://github.com/celerizer/Press-F-Ultra), a Channel F emulator for the Nintendo 64 video game console
- [Press F Thumby](https://github.com/celerizer/Press-F-Thumby), a Channel F emulator for the RP2040-based Thumby microconsole

libpressf is written to maintain maximum compatibility across different build targets by adhering to C89 standards and minimizing the use of standard libraries.

## Building

- Include `libpressf.mk` in your project makefile:

```make
include libpressf/libpressf.mk
```

- Add the exported `PRESS_F_SOURCES` and `PRESS_F_HEADERS` to your project makefile:

```make
SOURCES += $(PRESS_F_SOURCES)
HEADERS += $(PRESS_F_HEADERS)
```

- Add any compile-time options you wish to change. A list of all compile-time options along with their default values can be found in `config.h`. As an example:

```make
CFLAGS += \
  -DPRESS_F_AUDIO_FREQUENCY=44100 \
  -DPRESS_F_BIG_ENDIAN=0 \
  -DPRESS_F_ROMC=1
```

## Usage

- The following example code can be used to initialize a Channel F console:

```c
f8_system_t system;

/* Initialize an F8 system consisting only of the main 3850 CPU */
f8_init(&system);

/* Hook up all devices of a Channel F console */
f8_system_init(&system, &f8_systems[F8_SYSTEM_CHANNEL_F]);

/* Add some program data to the 3851s that are now hooked up */
f8_write(&system, 0x0000, /* pointer to program data */, 0x0400);
f8_write(&system, 0x0400, /* pointer to program data */, 0x0400);
```

- Then, the system can begin processing using:

```c
/* Process one instruction... */
f8_step(&system);

/* ...or, process one 1/60sec frame */
f8_run(&system);
```
