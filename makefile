#!/bin/sh

buffer:
	nasm buffers.asm -o ./Binaries/BUFFERS.COM -f bin -l ./Listings/buffers.lst -O0v
