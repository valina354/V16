#ifndef PERIPHERALS_H
#define PERIPHERALS_H

#include <cstdint>
#include <memory>
#include <vector>
#include <mutex>
#include <SDL.h>
#include "cpu.h"

class Peripherals {
private:
    CPU16& cpu;
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_AudioDeviceID audio_device;
    std::vector<uint8_t> keyboard_buffer;
    uint32_t last_pit_tick_time;

    std::mutex vga_mutex;
    std::mutex keyboard_mutex;

    static const int CHAR_WIDTH = 8;
    static const int CHAR_HEIGHT = 16;
    static const int TEXT_COLS = 80;
    static const int TEXT_ROWS = 25;
    static const int SCREEN_WIDTH = TEXT_COLS * CHAR_WIDTH;
    static const int SCREEN_HEIGHT = TEXT_ROWS * CHAR_HEIGHT;

    std::vector<uint16_t> vga_text_buffer;
    uint8_t crtc_address_register;
    uint16_t cursor_pos;
    uint16_t vram_address_ptr;

    bool cursor_enabled;

    SDL_Color vga_palette[16];

    void render_text_mode();

public:
    Peripherals(CPU16& owner_cpu);
    ~Peripherals();
    bool init_audio();
    uint16_t read_port(uint16_t port);
    void write_port(uint16_t port, uint16_t value);
    void update();
};

#endif // PERIPHERALS_H