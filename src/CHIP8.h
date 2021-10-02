#pragma once

#include <array>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <random>
#include <string_view>

#ifndef NODIS
#define NODIS [[nodiscard]]
#endif

class CHIP8
{
    std::mt19937 engine;
    std::array<std::uint8_t, 16> registers{};
    std::array<std::uint8_t, 4096> memory{};
    std::uint16_t index{};
    std::uint16_t pc = program_start;
    std::array<std::uint16_t, 16> stack{};
    std::uint8_t sp{};
    std::uint8_t delay_timer{};
    std::uint8_t sound_timer{};
    std::bitset<16> keypad{};
    std::bitset<64 * 32> video{};
    bool draw_flag{};

    constexpr static int font_start = 0x50;
    constexpr static int program_start = 0x200;
    constexpr static std::array<std::uint8_t, 80> fontset{
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

public:
    constexpr static int width = 64;
    constexpr static int height = 32;

    CHIP8();
    void cycle() noexcept;
    NODIS bool load_program(std::string_view file);
    NODIS bool should_draw() const noexcept { return draw_flag; }
    NODIS const auto& get_pixels() const noexcept { return video; }

private:
    NODIS std::uint16_t fetch_opcode() const noexcept
    {
        return memory[pc] << 8 | memory[pc + 1];
    }

    NODIS std::uint8_t& Vx(std::uint16_t op) noexcept
    {
        return registers[get_x(op)];
    }
    NODIS std::uint8_t& Vy(std::uint16_t op) noexcept
    {
        return registers[get_y(op)];
    }

    NODIS std::uint8_t get_x(uint16_t op) const noexcept
    {
        return (op & 0x0F'00) >> 8;
    }
    NODIS std::uint8_t get_y(uint16_t op) const noexcept
    {
        return (op & 0xF0) >> 4;
    }
    void execute() noexcept;
    void call(std::uint16_t addr) noexcept;
    void update_keystates() noexcept;
    void skip(std::uint8_t lhs, std::uint8_t rhs, bool comp) noexcept;
    NODIS std::uint8_t rnd();
    void draw(std::uint16_t op) noexcept;
    void opcode8(std::uint16_t op) noexcept;
    void opcodeE(std::uint16_t op) noexcept;
    void opcodeF(std::uint16_t op) noexcept;
};

#undef NODIS
