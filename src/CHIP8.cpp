#include "CHIP8.h"

#include <algorithm>
#include <fstream>
#include <vector>

#include <raylib.h>

#ifdef _MSC_VER
#define UNREACHABLE() __assume(0)
#else
#define UNREACHABLE() __builtin_unreachable()
#endif

namespace ranges = std::ranges;

CHIP8::CHIP8() : engine(std::random_device{}())
{
    ranges::copy(fontset, memory.data() + font_start);
}

void CHIP8::cycle() noexcept
{
    execute();
    update_keystates();
}

bool CHIP8::load_program(std::string_view filename)
{
    std::basic_ifstream<std::uint8_t> file(filename.data(), std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        return false;
    }

    const std::streampos size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<std::uint8_t> buffer;
    buffer.reserve(size);

    using stream_iter = std::istreambuf_iterator<std::uint8_t>;
    ranges::copy(stream_iter(file), stream_iter(), std::back_inserter(buffer));
    ranges::copy(buffer, memory.data() + program_start);

    return true;
}

void CHIP8::update_keystates() noexcept
{
    keypad[0] = IsKeyDown(KEY_X);
    keypad[1] = IsKeyDown(KEY_ONE);
    keypad[2] = IsKeyDown(KEY_TWO);
    keypad[3] = IsKeyDown(KEY_THREE);
    keypad[4] = IsKeyDown(KEY_Q);
    keypad[5] = IsKeyDown(KEY_W);
    keypad[6] = IsKeyDown(KEY_E);
    keypad[7] = IsKeyDown(KEY_A);
    keypad[8] = IsKeyDown(KEY_S);
    keypad[9] = IsKeyDown(KEY_D);
    keypad[10] = IsKeyDown(KEY_Z);
    keypad[11] = IsKeyDown(KEY_C);
    keypad[12] = IsKeyDown(KEY_FOUR);
    keypad[13] = IsKeyDown(KEY_R);
    keypad[14] = IsKeyDown(KEY_F);
    keypad[15] = IsKeyDown(KEY_V);
}

void CHIP8::execute() noexcept
{
    // opcodes - http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#3.0
    std::uint16_t opcode = fetch_opcode();
    pc += 2;
    switch (opcode & 0xF0'00)
    {
    case 0x00'00:
        switch (opcode & 0x0F)
        {
        case 0x00: // 00E0 - CLS
            video.reset();
            break;
        case 0x0E: // 00EE - RET
            pc = stack[sp--];
            break;
        default:
            UNREACHABLE();
            break;
        }
        break;
    case 0x10'00: // 1nnn - JP addr
        pc = opcode & 0x0F'FF;
        break;
    case 0x20'00: // 2nnn - CALL addr
        call(opcode & 0x0F'FF);
        break;
    case 0x30'00: // 3xkk - SE Vx, byte
        skip(Vx(opcode), opcode & 0x00'FF, true);
        break;
    case 0x40'00: // 4xkk - SNE Vx, byte
        skip(Vx(opcode), opcode & 0x00'FF, false);
        break;
    case 0x50'00: // 5xy0 - SE Vx, Vy
        skip(Vx(opcode), Vy(opcode), true);
        break;
    case 0x60'00: // 6xkk - LD Vx, byte
        Vx(opcode) = (opcode & 0x00'FF);
        break;
    case 0x70'00: // 7xkk - ADD Vx, byte
        Vx(opcode) += opcode & 0x00'FF;
        break;
    case 0x80'00:
        opcode8(opcode);
        break;
    case 0x90'00: // 9xy0 - SNE Vx, Vy
        skip(Vx(opcode), Vy(opcode), false);
        break;
    case 0xA0'00: // Annn - LD I, addr
        index = opcode & 0x0F'FF;
        break;
    case 0xB0'00: // Bnnn - JP V0, addr
        pc = registers[0] + (opcode & 0x0F'FF);
        break;
    case 0xC0'00: // Cxkk - RND Vx, byte
        Vx(opcode) = rnd() & (opcode & 0x00'FF);
        break;
    case 0xD0'00: // Dxyn - DRW Vx, Vy, nibble
        draw(opcode);
        break;
    case 0xE0'00:
        opcodeE(opcode);
        break;
    case 0xF0'00:
        opcodeF(opcode);
        break;
    default:
        UNREACHABLE();
        break;
    }
    if (delay_timer > 0)
    {
        --delay_timer;
    }
    if (sound_timer > 0)
    {
        --sound_timer;
    }
}

void CHIP8::call(std::uint16_t addr) noexcept
{
    stack[++sp] = pc;
    pc = addr;
}

std::uint8_t CHIP8::rnd()
{
    std::uniform_int_distribution<std::uint16_t> dist(0, 255);
    return dist(engine);
}

void CHIP8::skip(std::uint8_t lhs, std::uint8_t rhs, bool comp) noexcept
{
    if (lhs == rhs == comp)
    {
        pc += 2;
    }
}

void CHIP8::opcode8(std::uint16_t opcode) noexcept
{
    switch (opcode & 0xF)
    {
    case 0x00: // 8xy0 - LD Vx, Vy
        Vx(opcode) = Vy(opcode);
        break;
    case 0x01: // 8xy1 - OR Vx, Vy
        Vx(opcode) |= Vy(opcode);
        break;
    case 0x02: // 8xy2 - AND Vx, Vy
        Vx(opcode) &= Vy(opcode);
        break;
    case 0x03: // 8xy3 - XOR Vx, Vy
        Vx(opcode) ^= Vy(opcode);
        break;
    case 0x04: // 8xy4 - ADD Vx, Vy
    {
        auto result = static_cast<std::int16_t>(Vx(opcode)) + Vy(opcode);
        registers[0x0F] = result > 0xFF;
        Vx(opcode) = static_cast<std::uint8_t>(result);
        break;
    }
    case 0x05: // 8xy5 - SUB Vx, Vy
        registers[0x0F] = Vx(opcode) > Vy(opcode);
        Vx(opcode) -= Vy(opcode);
        break;
    case 0x06: // 8xy6 - SHR Vx {, Vy}
        registers[0x0F] = Vx(opcode) & 1;
        Vx(opcode) >>= 1;
        break;
    case 0x07: // 8xy7 - SUBN Vx, Vy
        registers[0x0F] = Vy(opcode) > Vx(opcode);
        Vx(opcode) = Vy(opcode) - Vx(opcode);
        break;
    case 0x0E: // 8xyE - SHL Vx {, Vy}
        registers[0x0F] = (Vx(opcode) & 0x80) >> 7;
        Vx(opcode) <<= 1;
        break;
    default:
        UNREACHABLE();
        break;
    }
}

void CHIP8::draw(std::uint16_t opcode) noexcept
{
    std::uint16_t x = Vx(opcode);
    std::uint16_t y = Vy(opcode);
    std::uint16_t height = opcode & 0x0F;

    registers[0x0F] = 0;
    for (int yline = 0; yline < height; yline++)
    {
        std::uint16_t pixel = memory[index + yline];
        for (int xline = 0; xline < 8; xline++)
        {
            if ((pixel & (0x80 >> xline)) != 0)
            {
                int v_index = x + xline + ((y + yline) * width);
                if (video[v_index])
                {
                    registers[0x0F] = 1;
                }
                video[v_index] = video[v_index] ^ 1;
            }
        }
    }
    draw_flag = true;
}

void CHIP8::opcodeE(std::uint16_t opcode) noexcept
{
    switch (opcode & 0xFF)
    {
    case 0x9E: // Ex9E - SKP Vx
        if (keypad[Vx(opcode)])
        {
            pc += 2;
        }
        break;
    case 0xA1: // ExA1 - SKNP Vx
        if (!keypad[Vx(opcode)])
        {
            pc += 2;
        }
        break;
    default:
        UNREACHABLE();
        break;
    }
}

void CHIP8::opcodeF(std::uint16_t opcode) noexcept
{
    switch (opcode & 0xFF)
    {
    case 0x07: // Fx07 - LD Vx, DT
        Vx(opcode) = delay_timer;
        break;
    case 0x0A: // Fx0A - LD Vx, K
        if (keypad.none())
        {
            pc -= 2;
        }
        else
            for (int i = 0; i < keypad.size(); ++i)
            {
                Vx(opcode) = i;
            }
        break;
    case 0x15: // Fx15 - LD DT, Vx
        delay_timer = Vx(opcode);
        break;
    case 0x18: // Fx18 - LD ST, Vx
        sound_timer = Vx(opcode);
        break;
    case 0x1E: // Fx1E - ADD I, Vx
        index += Vx(opcode);
        break;
    case 0x29: // Fx29 - LD F, Vx
        index = 0x50 + (5 * Vx(opcode));
        break;
    case 0x33: // Fx33 - LD B, Vx
    {
        std::uint8_t value = Vx(opcode);
        memory[index + 2] = value % 10;
        value /= 10;
        memory[index + 1] = value % 10;
        value /= 10;
        memory[index] = value % 10;
        break;
    }
    case 0x55: // Fx55 - LD [I], Vx
    {
        const auto n = get_x(opcode);
        for (std::uint8_t i = 0; i <= n; ++i)
        {
            memory[index + i] = registers[i];
        }
        break;
    }
    case 0x65: // Fx65 - LD Vx, [I]
    {
        const auto n = get_x(opcode);
        for (std::uint8_t i = 0; i <= n; ++i)
        {
            registers[i] = memory[index + i];
        }
        break;
    }
    default:
        UNREACHABLE();
        break;
    }
}