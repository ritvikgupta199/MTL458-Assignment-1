# Sets COMPILER to the default compiler for OSX and other systems
# taken from https://stackoverflow.com/questions/24563150/makefile-with-os-dependent-compiler
UNAME := $(shell uname)
ifeq ($(UNAME), Darwin)
	COMPILER = clang 
else
	COMPILER = gcc
endif

run:
	./shell
clean:
	rm -f shell
build:
	rm -f shell
	$(COMPILER) -o shell shell.c
all:
	rm -f shell
	$(COMPILER) shell.c -o shell
	clear
	./shell