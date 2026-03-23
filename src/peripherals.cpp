#include "peripherals.h"
#include "cpu.h"
#include "font.h"
#include <iostream>
#include <vector>
#include <algorithm>

Peripherals::Peripherals(CPU16& owner_cpu) : cpu(owner_cpu), window(nullptr), renderer(nullptr) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return;

    window = SDL_CreateWindow("CPU16 Virtual Machine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    vga_text_buffer.resize(TEXT_COLS * TEXT_ROWS, 0x0700 | ' ');
    cursor_pos = 0;
    crtc_address_register = 0;
    vram_address_ptr = 0;

    vga_palette[0] = {0,0,0,255}; vga_palette[1] = {0,0,170,255}; vga_palette[2] = {0,170,0,255}; vga_palette[3] = {0,170,170,255};
    vga_palette[4] = {170,0,0,255}; vga_palette[5] = {170,0,170,255}; vga_palette[6] = {170,85,0,255}; vga_palette[7] = {170,170,170,255};
    vga_palette[8] = {85,85,85,255}; vga_palette[9] = {85,85,255,255}; vga_palette[10] = {85,255,85,255}; vga_palette[11] = {85,255,255,255};
    vga_palette[12] = {255,85,85,255}; vga_palette[13] = {255,85,255,255}; vga_palette[14] = {255,255,85,255}; vga_palette[15] = {255,255,255,255};

    last_pit_tick_time = SDL_GetTicks();
}

Peripherals::~Peripherals() {
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();
}

uint16_t Peripherals::read_port(uint16_t port) {
    switch (port) {
    case 0x02: // Read Cursor Byte
        if (crtc_address_register == 0x0E) return (cursor_pos >> 8) & 0xFF;
        if (crtc_address_register == 0x0F) return cursor_pos & 0xFF;
        return 0;
    case 0x03: { // PIT Timer Tick
        uint32_t current_time = SDL_GetTicks();
        if (current_time - last_pit_tick_time >= 50) {
            last_pit_tick_time = current_time;
            return 1;
        }
        return 0;
    }
    }
    return 0;
}

void Peripherals::write_port(uint16_t port, uint16_t value) {
    switch (port) {
    case 0x01: // Select Cursor High/Low Byte
        crtc_address_register = value & 0xFF;
        break;
    case 0x02: // Write Cursor Byte
        if (crtc_address_register == 0x0E) cursor_pos = (cursor_pos & 0x00FF) | (value << 8);
        else if (crtc_address_register == 0x0F) cursor_pos = (cursor_pos & 0xFF00) | (value & 0xFF);
        break;
    case 0x10: // Set VRAM Pointer
        vram_address_ptr = value;
        break;
    case 0x11: // Write Char+Attr to VRAM and increment pointer
        if (vram_address_ptr < TEXT_COLS * TEXT_ROWS) {
            vga_text_buffer[vram_address_ptr++] = value;
        }
        break;
    case 0x12: // Fill Screen with Char+Attr
        std::fill(vga_text_buffer.begin(), vga_text_buffer.end(), value);
        break;
    }
}

void Peripherals::render_text_mode() {
    std::lock_guard<std::mutex> lock(vga_mutex);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    for (int y = 0; y < TEXT_ROWS; ++y) {
        for (int x = 0; x < TEXT_COLS; ++x) {
            int offset = y * TEXT_COLS + x;
            uint16_t cell = vga_text_buffer[offset];
            uint8_t ch = cell & 0xFF;
            uint8_t attr = (cell >> 8) & 0xFF;

            uint8_t fg_idx = attr & 0x0F;
            uint8_t bg_idx = (attr >> 4) & 0x0F;

            const unsigned char* glyph = vga_font_8x16[ch];

            for (int cy = 0; cy < CHAR_HEIGHT; ++cy) {
                for (int cx = 0; cx < CHAR_WIDTH; ++cx) {
                    if ((glyph[cy] >> (7 - cx)) & 1) {
                        SDL_SetRenderDrawColor(renderer, vga_palette[fg_idx].r, vga_palette[fg_idx].g, vga_palette[fg_idx].b, 255);
                    }
                    else {
                        SDL_SetRenderDrawColor(renderer, vga_palette[bg_idx].r, vga_palette[bg_idx].g, vga_palette[bg_idx].b, 255);
                    }
                    SDL_RenderDrawPoint(renderer, x * CHAR_WIDTH + cx, y * CHAR_HEIGHT + cy);
                }
            }
        }
    }

    if ((SDL_GetTicks() / 400) % 2 == 0) {
        int cursor_x = cursor_pos % TEXT_COLS;
        int cursor_y = cursor_pos / TEXT_COLS;
        SDL_Rect cursor_rect = { cursor_x * CHAR_WIDTH, cursor_y * CHAR_HEIGHT + 14, CHAR_WIDTH, 2 };
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &cursor_rect);
    }
}

void Peripherals::update() {
    render_text_mode();
    SDL_RenderPresent(renderer);
    SDL_Event e;
    while (SDL_PollEvent(&e)) { 
        if (e.type == SDL_QUIT) 
            cpu.halt(); 
    }
}