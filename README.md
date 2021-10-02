# Dan's CHIP-8 Emulator

Nothing particularly special. Uses Raylib for graphics.

# Building:

Dependencies:
* [argparse][]
* [fmtlib][]
* [raylib][]

[argparse] and [fmtlib] aren't exactly _needed_ dependencies, I'm just lazy and also wanted coloured output for one lone error message 🤷‍♂️.

You can use the CMake option `USE_FALLBACK` to download and build these dependencies from source if you don't have them installed. Example:
```cmake
cmake -B build -DUSE_FALLBACK=ON
cmake --build build
```

# Resources Used:

* https://multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/
* http://devernay.free.fr/hacks/chip8/C8TECH10.HTM

There may have been more, I just can't remember them. Those were the primary sources, however.

[argparse]: https://github.com/p-ranav/argparse
[fmtlib]: https://github.com/fmtlib/fmt
[raylib]: https://github.com/raysan5/raylib
