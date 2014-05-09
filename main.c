#include <stdio.h>
#include <ft2build.h>
#include FT_FREETYPE_H

int bytes_per_row(int bitmap_width) {
	return bitmap_width / 8 + (bitmap_width % 8 > 0 ? 1 : 0);
}
int get_pixel(int bitmap_width,
              int bitmap_rows,
              unsigned char const* bitmap,
              int x,
              int y) {
    int const byte = x / 8;
    int const bit = 7 - (x - 8 * byte);
    int const p = (bitmap[y * bytes_per_row(bitmap_width) + byte] & (1 << bit)) > 0;
    return p;
}
