CC = gcc
PKG_CONFIG ?= pkg-config

CFLAGS = -std=c17 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Wpedantic -Werror -Iinclude
LDFLAGS =

LIBSSH_CFLAGS := $(shell $(PKG_CONFIG) --cflags libssh 2>/dev/null)
LIBSSH_LIBS := $(shell $(PKG_CONFIG) --libs libssh 2>/dev/null)

CFLAGS += $(LIBSSH_CFLAGS)
LDFLAGS += -pthread $(LIBSSH_LIBS)

ifeq ($(strip $(LIBSSH_LIBS)),)
# libssh not available
else
CFLAGS += -DMAUM_HAVE_LIBSSH
endif

SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)

BIN = maum

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(OBJ) $(BIN)

.PHONY: all clean
