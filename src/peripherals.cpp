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

    static SDL_Texture* screen_texture = SDL_CreateTexture(
        renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
        SCREEN_WIDTH, SCREEN_HEIGHT
    );

    std::vector<uint32_t> pixels(SCREEN_WIDTH * SCREEN_HEIGHT, 0xFF000000);

    for (int y = 0; y < TEXT_ROWS; ++y) {
        for (int x = 0; x < TEXT_COLS; ++x) {
            uint16_t cell = vga_text_buffer[y * TEXT_COLS + x];
            uint8_t ch = cell & 0xFF;
            uint8_t attr = (cell >> 8) & 0xFF;

            SDL_Color fg = vga_palette[attr & 0x0F];
            SDL_Color bg = vga_palette[(attr >> 4) & 0x0F];

            uint32_t fg_color = (255 << 24) | (fg.r << 16) | (fg.g << 8) | fg.b;
            uint32_t bg_color = (255 << 24) | (bg.r << 16) | (bg.g << 8) | bg.b;

            const unsigned char* glyph = vga_font_8x16[ch];

            for (int cy = 0; cy < CHAR_HEIGHT; ++cy) {
                for (int cx = 0; cx < CHAR_WIDTH; ++cx) {
                    bool is_set = (glyph[cy] >> (7 - cx)) & 1;
                    pixels[(y * CHAR_HEIGHT + cy) * SCREEN_WIDTH + (x * CHAR_WIDTH + cx)] = is_set ? fg_color : bg_color;
                }
            }
        }
    }

    if ((SDL_GetTicks() / 400) % 2 == 0) {
        int cx = (cursor_pos % TEXT_COLS) * CHAR_WIDTH;
        int cy = (cursor_pos / TEXT_COLS) * CHAR_HEIGHT + 14;
        for (int i = 0; i < CHAR_WIDTH; i++) {
            pixels[cy * SCREEN_WIDTH + (cx + i)] = 0xFFFFFFFF;
            pixels[(cy + 1) * SCREEN_WIDTH + (cx + i)] = 0xFFFFFFFF;
        }
    }

    SDL_UpdateTexture(screen_texture, nullptr, pixels.data(), SCREEN_WIDTH * sizeof(uint32_t));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, screen_texture, nullptr, nullptr);
}

void Peripherals::update() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            cpu.halt();
        }
    }

    render_text_mode();
    SDL_RenderPresent(renderer);
}