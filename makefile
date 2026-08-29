#!/bin/sh

buffer:
	nasm buffers.asm -o ./bin/BUFFERS.COM -f bin -l ./lst/buffers.lst -O0v
