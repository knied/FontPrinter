all:
	gcc main.c -std=gnu99 -I/usr/include/freetype2 -lfreetype -o FontPrinter
