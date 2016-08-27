#include "u.h"
#include "dat.h"
#include "console.h"
#include "vec.h"
#include "arena.h"
#include "font8x16.h"

u32* renderglyphs(Arena *a) {
        u32* buf = arenapusharray(a, FONT_GLYPHS*FONT_WIDTH*FONT_HEIGHT, u32);
        for (int i = 0; i < FONT_GLYPHS * FONT_HEIGHT; ++i) {
                for (int j =  0; j < FONT_WIDTH; ++j) {
                        if ((font8x16[i] >> (FONT_WIDTH - j)) & 0x1) {
                                buf[FONT_WIDTH*i + j] = 0x00000000;
                        } else {
                                buf[FONT_WIDTH*i + j] = 0xffffffff;
                        }
                }

        }
        return buf;
}

void copy(u32* dst, u32* src, int x, int y, int width, int height) {
        for (int i = 0; i < height; i++) {
                for (int j = 0; j < width; j++) {
                        dst[(y+i)*1920 + (x+j)] = src[(width* i) + j];
                }
        }

}

void copyrect(u32* dst, u32* src, int x, int y, int idx) {
        for (int i = 0; i < FONT_HEIGHT; i++) {
                for (int j = 0; j < FONT_WIDTH; j++) {
                        // dst[(y+i)*1920 + (x+j)] = 0x00ff0000; // for testing
                        dst[(y+i)*1920 + (x+j)] = src[idx*FONT_WIDTH*FONT_HEIGHT + (FONT_WIDTH * i) + j];
                }
        }
}

void rendertext(u32* dst, u32* src, char* str, int x, int y) {
        for (int i = 0; str[i]; ++i) {
                copyrect(dst, src, x+FONT_WIDTH*i, y, str[i]);
        }
}
