////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <ft2build.h>
#include FT_FREETYPE_H

////////////////////////////////////////////////////////////////////////////////

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
    //int const bit = (x - 8 * byte);
    int const p = (bitmap[y * bytes_per_row(bitmap_width) + byte] & (1 << bit)) > 0;
    return p;
}

void set_pixel(int bitmap_width,
               int bitmap_rows,
               unsigned char* bitmap,
               int x,
               int y,
               int value) {
    int const byte = x / 8;
    int const bit = 7 - (x - 8 * byte);
    if (value) {
        bitmap[y * bytes_per_row(bitmap_width) + byte] |= (1 << bit);
    } else {
        unsigned char mask = 0xFF - (1 << bit);
        bitmap[y * bytes_per_row(bitmap_width) + byte] &= mask;
    }
}

void create_bitmap(int bitmap_width, int bitmap_rows, unsigned char** bitmap) {
    *bitmap = malloc(bytes_per_row(bitmap_width) * bitmap_rows);
}

void free_bitmap(unsigned char* bitmap) {
    free(bitmap);
}

void center_bitmap(int target_width, int* bitmap_width, int* bitmap_rows, unsigned char** bitmap) {
    int dst_width = target_width;
    int dst_rows = *bitmap_rows;
    unsigned char* dst_bitmap = 0;
    create_bitmap(dst_width, dst_rows, &dst_bitmap);
    
    int offset_x = (dst_width - *bitmap_width) / 2;
    for (int y = 0; y < dst_rows; ++y) {
        for (int x = 0; x < dst_width; ++x) {
            int src_x = x - offset_x;
            int p = 0;
            if (src_x >= 0 && src_x < *bitmap_width) {
                p = get_pixel(*bitmap_width, *bitmap_rows, *bitmap, src_x, y);
            }
            set_pixel(dst_width, dst_rows, dst_bitmap, x, y, p);
        }
    }
    
    free_bitmap(*bitmap);
    *bitmap = dst_bitmap;
    *bitmap_width = dst_width;
    *bitmap_rows = dst_rows;
}

void rotate_bitmap(int* bitmap_width, int* bitmap_rows, unsigned char** bitmap) {
    int dst_width = *bitmap_rows;
    int dst_rows = *bitmap_width;
    unsigned char* dst_bitmap;
    create_bitmap(dst_width, dst_rows, &dst_bitmap);
    for (int y = 0; y < dst_rows; ++y) {
        for (int x = 0; x < dst_width; ++x) {
            set_pixel(dst_width, dst_rows, dst_bitmap, x, y, get_pixel(*bitmap_width, *bitmap_rows, *bitmap, y, *bitmap_rows - x - 1));
        }
    }
    free_bitmap(*bitmap);
    *bitmap = dst_bitmap;
    *bitmap_width = dst_width;
    *bitmap_rows = dst_rows;
}

void translate_bitmap(int dx, int* bitmap_width, int* bitmap_rows, unsigned char** bitmap) {
    int dst_width = *bitmap_width + dx;
    int dst_rows = *bitmap_rows;
    unsigned char* dst_bitmap;
    create_bitmap(dst_width, dst_rows, &dst_bitmap);
    for (int y = 0; y < dst_rows; ++y) {
        for (int x = 0; x < dst_width; ++x) {
            int src_x = x - dx;
            int p = 0;
            if (src_x >= 0 && src_x < *bitmap_width) {
                p = get_pixel(*bitmap_width, *bitmap_rows, *bitmap, src_x, y);
            }
            set_pixel(dst_width, dst_rows, dst_bitmap, x, y, p);
        }
    }
    free_bitmap(*bitmap);
    *bitmap = dst_bitmap;
    *bitmap_width = dst_width;
    *bitmap_rows = dst_rows;
}

void extend_bitmap(int dx, int dy, int* bitmap_width, int* bitmap_rows, unsigned char** bitmap) {
    int dst_width = *bitmap_width + dx;
    int dst_rows = *bitmap_rows + dy;
    unsigned char* dst_bitmap;
    create_bitmap(dst_width, dst_rows, &dst_bitmap);
    for (int y = 0; y < dst_rows; ++y) {
        for (int x = 0; x < dst_width; ++x) {
            int p = 0;
            if (x < *bitmap_width && y < *bitmap_rows) {
                p = get_pixel(*bitmap_width, *bitmap_rows, *bitmap, x, y);
            }
            set_pixel(dst_width, dst_rows, dst_bitmap, x, y, p);
        }
    }
    free_bitmap(*bitmap);
    *bitmap = dst_bitmap;
    *bitmap_width = dst_width;
    *bitmap_rows = dst_rows;
}

void debug_output(int bitmap_width,
                  int bitmap_rows,
                  unsigned char* bitmap) {
    for (int y = 0; y < bitmap_rows; ++y) {
        for (int x = 0; x < bitmap_width; ++x) {
            printf("%d", get_pixel(bitmap_width, bitmap_rows, bitmap, x, y));
        }
        printf("\n");
    }
}

void python_output(int bitmap_width,
                   int bitmap_rows,
                   unsigned char* bitmap) {
    printf("width  = %d\n", bitmap_width);
    printf("height = %d\n", bitmap_rows);
    printf("data   = [\n");
    int bpr = bytes_per_row(bitmap_width);
    for (int y = 0; y < bitmap_rows; ++y) {
        printf("  ");
        for (int x = 0; x < bpr; ++x) {
            printf("0x%02X", bitmap[y * bpr + x]);
            if (y < bitmap_rows-1 || x < bpr-1) {
                printf(",");
            }
        }
        printf("\n");
    }
    printf("]");
}

////////////////////////////////////////////////////////////////////////////////

FT_Error init_font(FT_Library* library, FT_Face* face, FT_Long face_id, int size, char const* filename) {
    FT_Error error = FT_Init_FreeType(library);
    if (error) {
        printf("Error: Unable to init freetype.\n");
        return error;
    }
    
    error = FT_New_Face(*library,
                        filename,
                        face_id,
                        face);
    if (error == FT_Err_Unknown_File_Format) {
        printf("Error: Unsupported font file format.\n");
        return error;
    } else if (error) {
        printf("Error: Unable to read font file: '%s'.\n", filename);
        return error;
    }
    
    (*face)->style_flags |= FT_STYLE_FLAG_ITALIC;
    
    FT_UInt dpi = 384;
    
    error = FT_Set_Char_Size(*face,
                             0,
                             size*64,
                             0,
                             dpi);
    if (error) {
        printf("Error: Unable to set character size.\n");
        return error;
    }
    
    return 0;
}

FT_Error free_font(FT_Library* library, FT_Face* face) {
    FT_Error error = FT_Done_Face(*face);
    if (error) {
        printf("Error: Unable to free the face.\n");
        return error;
    }
    
    error = FT_Done_FreeType(*library);
    if (error) {
        printf("Error: Unable to free freetype library.\n");
        return error;
    }
    
    return 0;
}

int generate_glyph(FT_Face* face, FT_UInt glyph_index, int* out_bitmap_width, int* out_bitmap_rows, unsigned char** out_bitmap, int* out_bitmap_bottom) {
    /*printf("unic: %d\n", FT_ENCODING_UNICODE);
     for (int i = 0; i < (*face)->num_charmaps; ++i) {
     printf("charmap: %d\n", i);
     printf("encoding: %d\n", (*face)->charmaps[i]->encoding);
     }
     FT_Set_Charmap(*face, (*face)->charmaps[0]);*/
    
    //FT_UInt glyph_index = FT_Get_Char_Index(*face, charcode);
    
    FT_Int32 load_flags = 0;
    
    FT_Error error = FT_Load_Glyph(*face,
                                   glyph_index,
                                   load_flags);
    if (error) {
        printf("Error: Unable to load glyph.\n");
        return error;
    }
    
    FT_Render_Mode render_mode = FT_RENDER_MODE_MONO;
    
    error = FT_Render_Glyph((*face)->glyph,
                            render_mode);
    if (error) {
        printf("Error: Unable to render glyph.\n");
        return error;
    }
    
    FT_Bitmap* bitmap = &(*face)->glyph->bitmap;
    //printf("bitmap left: %d\n", (*face)->glyph->bitmap_left);
    //printf("bitmap top: %d\n", (*face)->glyph->bitmap_top);
    //printf("bitmap width: %d\n", bitmap->width);
    //printf("bitmap rows: %d\n", bitmap->rows);
    *out_bitmap_bottom = (*face)->glyph->bitmap_top - bitmap->rows;
    
    *out_bitmap_width = bitmap->width;
    *out_bitmap_rows = bitmap->rows;
    create_bitmap(bitmap->width, bitmap->rows, out_bitmap);
    // convert to our bitmap format
    int bits_per_row = 16 * (bitmap->width / 16 + (bitmap->width % 16 > 0 ? 1 : 0));
    for (int y = 0; y < bitmap->rows; ++y) {
        for (int x = 0; x < bitmap->width; ++x) {
            int offset = y * bits_per_row + x;
            int byte = offset / 8;
            int bit = 7 - (offset - byte * 8);
            int p = bitmap->buffer[byte] & (1 << bit);
            set_pixel(bitmap->width, bitmap->rows, *out_bitmap, x, y, p);
        }
    }
    
    return 0;
}

////////////////////////////////////////////////////////////////////////////////

int main(int argc, const char * argv[])
{
    //FT_ULong charcode = 'A';
    FT_Long face_id = 0;
    FT_UInt glyph_index = 0;
    int size = 8;
    int base_line = 0;
    int spacing = 0;
    int error = 0;
    if (argc % 2 != 0) {
        error = 1;
    } else {
        for (int i = 1; i < argc-1; i+=2) {
            if (!strcmp("-c", argv[i])) {
                glyph_index = atoi(argv[i+1]);
            } else if (!strcmp("-f", argv[i])) {
                face_id = atoi(argv[i+1]);
            } else if (!strcmp("-s", argv[i])) {
                size = atoi(argv[i+1]);
            } else if (!strcmp("-b", argv[i])) {
                base_line = atoi(argv[i+1]);
            } else if (!strcmp("-d", argv[i])) {
                spacing = atoi(argv[i+1]);
            } else {
                error = 1;
            }
        }
    }
    
    if (error) {
        printf("Usage: FontPrinter [-c <charcode>] [-f <face id>] [-s <size>] [-b <base line offset>] [-d <spacing>] <Font file>\n");
        return 1;
    }
    
    FT_Library  library;
    FT_Face face;
    //if (init_font(&library, &face, face_id, size, "Font.otf")) {
    if (init_font(&library, &face, face_id, size, argv[argc-1])) {
        printf("Exit...\n");
        return 1;
    }
    
    int bitmap_width = 0;
    int bitmap_rows = 0;
    unsigned char* bitmap = 0;
    int bitmap_bottom = 0;
    if (generate_glyph(&face, glyph_index, &bitmap_width, &bitmap_rows, &bitmap, &bitmap_bottom)) {
        printf("Exit...\n");
        return 1;
    }
    rotate_bitmap(&bitmap_width, &bitmap_rows, &bitmap);
    translate_bitmap(base_line + bitmap_bottom, &bitmap_width, &bitmap_rows, &bitmap);
    extend_bitmap(0, spacing, &bitmap_width, &bitmap_rows, &bitmap);
    //center_bitmap(384, &bitmap_width, &bitmap_rows, &bitmap);
    python_output(bitmap_width, bitmap_rows, bitmap);
    //debug_output(bitmap_width, bitmap_rows, bitmap);
    free_bitmap(bitmap);
    glyph_index++;
    
    if (free_font(&library, &face)) {
        printf("Exit...\n");
        return 1;
    }
    
    return 0;
}

////////////////////////////////////////////////////////////////////////////////