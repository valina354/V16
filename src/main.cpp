#include <iostream>
#include <memory>
#include <thread>
#include "cpu.h"
#include "peripherals.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " program.asm" << std::endl;
        return 1;
    }

    srand(time(NULL));

    try {
        auto cpu = std::make_shared<CPU16>(nullptr);
        auto peripherals = std::make_shared<Peripherals>(*cpu);
        new (cpu.get()) CPU16(peripherals);

        std::cout << "Loading BIOS..." << std::endl;
        cpu->load_program("bios.asm", 0x7C00);
        std::cout << "Loading Program..." << std::endl;
        cpu->load_program(argv[1], 0x1000);
        cpu->set_pc(0x7C00);

        std::cout << "Starting CPU thread..." << std::endl;
        std::thread cpu_thread([&cpu]() {
            cpu->run();
            });

        const int FRAME_DURATION = 1000 / 60;

        while (cpu->is_running()) {
            uint32_t frame_start = SDL_GetTicks();

            peripherals->update();

            uint32_t frame_time = SDL_GetTicks() - frame_start;
            if (frame_time < FRAME_DURATION) {
                SDL_Delay(FRAME_DURATION - frame_time);
            }
        }

        std::cout << "Joining CPU thread..." << std::endl;
        cpu_thread.join();

    }
    catch (const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "\nCPU Halted." << std::endl;
    return 0;
}