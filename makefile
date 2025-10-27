
cflags := -lX11 -lXrandr -Wall -g
cc := gcc
src_dir := src
build_dir = := build

src := $(wildcard $(src_dir)/*c)
target := auto_ref_rate

all:
	$(cc) $(src) $(cflags) -o $(target)
