TARGET := bin/ccord.dll
CC := gcc
CFLAGS := -Wall -Wextra -O2 -g -Iinclude
LDFLAGS := -lcurl -lPortAudio -lopus -lsodium

SRCS := $(shell find src -name '*.c')
OBJS := $(patsubst src/%.c, build/%.o, $(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(@D)
	$(CC) -shared $(CFLAGS) -o $@ $^ $(LDFLAGS)

build/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build/ $(TARGET)

.PHONY: all clean