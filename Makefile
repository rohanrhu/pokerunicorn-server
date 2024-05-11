# Poker Server
# Copyright (C) 2023, Oğuzhan Eroğlu <meowingcate@gmail.com> (https://meowingcat.io)
# Licensed under GPLv3 License
# See LICENSE for more info

BUILD_DATE = "\"$(shell date +'%d.%m.%Y %H:%M')\""
BUILD_NUMBER = $(shell printf $$(($$(cat build.txt) + 1)))

VERBOSE = $(shell printf $$((PKRSRV_BUILD_VERBOSE)))

ifeq ($(strip $(VERBOSE)), "")
    VERBOSE := 1
endif

REVISION = "\"$(shell printf $$(git rev-parse --short HEAD))\""

CC = gcc

CFLAGS += -std=c17 \
		 -O0 \
		 -ggdb \
		 -I. \
		 -I/usr/include/postgresql \
		 -pthread \
		 -lm \
		 -lpq \
		 -lssl \
		 -lcrypto \
		 -ldl \
		 -lgmp \
		 -lcurl \
		 -Wno-unused-command-line-argument \
		 -Wmissing-field-initializers \
		 -Werror=return-type \
		 \
		 -DPKRSRV_VERBOSE=$(VERBOSE) \
		 -DPKRSRV_BUILD=$(BUILD_NUMBER) \
		 -DPKRSRV_BUILD_DATE=$(BUILD_DATE) \
		 -DPKRSRV_REVISION=$(REVISION)

ifneq ($(shell uname -s), Darwin)
    CFLAGS += 
endif

LDFLAGS += 

SOURCES = $(shell find . -wholename "./src/*.c")
HEADERS = $(shell find . -wholename "./include/*.h")
EXECUTABLES = build/pkrsrv
OBJECTS = $(addprefix ./build/, $(notdir $(filter-out ./src/pkrsrv.o, $(SOURCES:.c=.o))))
RM = rm -rf

.PHONY: clean \
		hiredis

all: build/pkrsrv

# <Thirdparty>

thirdparty/jsonic/jsonic.o:
	$(MAKE) -C thirdparty/jsonic jsonic.o

thirdparty/hiredis/libhiredis.a:
	$(MAKE) -C thirdparty/hiredis

thirdparty/lua/liblua.a:
	$(MAKE) -C thirdparty/lua

# </Thirdparty>

build/pkrsrv: src/pkrsrv.c \
			  build/tests/rsa.o \
			  include/pkrsrv.h \
			  build/deposit.o \
			  build/lobby.o \
			  build/server.o \
			  build/poker.o \
			  build/player.o \
			  build/card.o \
			  build/arg.o  \
			  build/table.o \
			  build/trie.o \
			  build/websocket.o \
			  build/random.o \
			  build/rsa.o \
			  build/eventloop.o \
			  build/uniqid.o \
			  build/string.o \
			  build/ref.o \
			  build/util.o \
			  build/db.o \
			  build/redis.o \
			  thirdparty/jsonic/jsonic.o \
			  thirdparty/lua/liblua.a \
			  thirdparty/hiredis/libhiredis.a
	mkdir -p build/

	$(CC) -o $@ $(filter-out %.h, $^) $(CFLAGS) $(LDFLAGS)
	chmod +x build/pkrsrv

	printf $$(($$(cat build.txt) + 1)) > build.txt
	
	@echo "\033[32mBuild: ${BUILD_NUMBER}\033[0m"
	@echo "\033[32mExecutable: ./build/pkrsrv is built.\033[0m"

build/arg.o: src/arg.c include/arg.h
	mkdir -p build/
	$(CC) -c -o $@ $(filter-out %.h, $^) $(CFLAGS)

build/eventloop.o: src/eventloop.c include/eventloop.h
	mkdir -p build/
	$(CC) -c -o $@ $(filter-out %.h, $^) $(CFLAGS)

build/table.o: src/table.c include/table.h
	mkdir -p build/
	$(CC) -c -o $@ $(filter-out %.h, $^) $(CFLAGS)

build/poker.o: src/poker.c include/poker.h
	mkdir -p build/
	$(CC) -c -o $@ $(filter-out %.h, $^) $(CFLAGS)

build/player.o: src/account.c include/account.h
	mkdir -p build/
	$(CC) -c -o $@ $(filter-out %.h, $^) $(CFLAGS)

build/random.o: src/random.c include/random.h
	mkdir -p build/
	$(CC) -c -o $@ $(filter-out %.h, $^) $(CFLAGS)

build/rsa.o: src/rsa.c include/rsa.h
	mkdir -p build/
	$(CC) -c -o $@ $(filter-out %.h, $^) $(CFLAGS)

build/card.o: src/card.c include/card.h
	mkdir -p build/
	$(CC) -c -o $@ $(filter-out %.h, $^) $(CFLAGS)

build/util.o: src/util.c include/util.h
	mkdir -p build/
	$(CC) -c -o $@ $(filter-out %.h, $^) $(CFLAGS)

build/uniqid.o: src/uniqid.c include/uniqid.h
	mkdir -p build/
	$(CC) -c -o $@ $(filter-out %.h, $^) $(CFLAGS)

build/server.o: src/server.c include/server.h
	mkdir -p build/
	$(CC) -c -o $@ $(filter-out %.h, $^) $(CFLAGS)

build/lobby.o: src/lobby.c include/lobby.h
	mkdir -p build/
	$(CC) -c -o $@ $(filter-out %.h, $^) $(CFLAGS)

build/string.o: src/string.c include/string.h
	mkdir -p build/
	$(CC) -c -o $@ $(filter-out %.h, $^) $(CFLAGS)

build/websocket.o: src/websocket.c include/websocket.h
	mkdir -p build/
	$(CC) -c -o $@ $(filter-out %.h, $^) $(CFLAGS)

build/trie.o: src/trie.c include/trie.h
	mkdir -p build/
	$(CC) -c -o $@ $(filter-out %.h, $^) $(CFLAGS)

build/ref.o: src/ref.c include/ref.h
	mkdir -p build/
	$(CC) -c -o $@ $(filter-out %.h, $^) $(CFLAGS)

build/redis.o: src/redis.c include/redis.h
	mkdir -p build/
	$(CC) -c -o $@ $(filter-out %.h, $^) $(CFLAGS)

build/db.o: src/db.c include/db.h
	mkdir -p build/
	$(CC) -c -o $@ $(filter-out %.h, $^) $(CFLAGS)

build/deposit.o: src/deposit.c include/deposit.h
	mkdir -p build/
	$(CC) -c -o $@ $(filter-out %.h, $^) $(CFLAGS)

build/tests/rsa.o: tests/rsa/rsa.c tests/rsa/rsa.h
	mkdir -p build/tests/
	$(CC) -c -o $@ $(filter-out %.h, $^) $(CFLAGS)

clean:
	$(RM) build/
	$(RM) $(EXECUTABLES)
	$(MAKE) -C thirdparty/jsonic clean
	$(MAKE) -C thirdparty/hiredis clean
	$(MAKE) -C thirdparty/lua clean

test: build/pkrsrv
	mkdir -p build/
	./build/pkrsrv