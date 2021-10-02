#include "CHIP8.h"

#include <string>

#include <argparse/argparse.hpp>
#include <fmt/color.h>
#include <fmt/ostream.h>
#include <raylib.h>

int main(int argc, const char* argv[])
{
#ifdef DISABLE_RAYLIB_LOGS
    SetTraceLogLevel(LOG_NONE);
#endif

    argparse::ArgumentParser args("chip-8", "0.1.0");
    args.add_argument("ROM").help("The ROM to load.");
    args.add_argument("speed").help("Set the target speed.").default_value(700).scan<'i', int>();
    args.add_argument("--scale", "-s").help("Set the window scale.").default_value(20).scan<'i', int>();
    try
    {
        args.parse_args(argc, argv);
    }
    catch (const std::runtime_error& e)
    {
        fmt::print("{}\n{}\n", e.what(), args);
        return 0;
    }

    CHIP8 chip;
    const std::string file = args.get("ROM");
    const bool loaded = chip.load_program(file);
    if (!loaded)
    {
        fmt::print(fmt::emphasis::bold | fg(fmt::color::red), "Failed to load \"{}\"\n", file);
        return 1;
    }

    const int scale = args.get<int>("-s");
    const int speed = args.get<int>("speed");

    InitWindow(CHIP8::width * scale, CHIP8::height * scale, "Dan's CHIP-8 Emulator");
    SetTargetFPS(speed);

    BeginDrawing();
    ClearBackground(BLACK);
    EndDrawing();

    while (!WindowShouldClose())
    {
        BeginDrawing();
        chip.cycle();
        if (chip.should_draw())
        {
            const auto& pixels = chip.get_pixels();
            for (int i = 0; i < pixels.size(); ++i)
            {
                const int row = i / CHIP8::width;
                const int column = i % CHIP8::width;
                DrawRectangle(column * scale, row * scale, scale, scale, pixels[i] ? WHITE : BLACK);
            }
        }
        EndDrawing();
    }
}
