# Buniq: Blocked Bloom Filter Uniq

[![License: CC BY-NC 4.0](https://img.shields.io/badge/License-CC%20BY--NC%204.0-blue.svg)](https://creativecommons.org/licenses/by-nc/4.0/)

`buniq` is a line de-duplication CLI built around a blocked Bloom filter.
It reads text lines from stdin or a single file and prints first-seen lines immediately.

## Features

- Fast approximate de-duplication with a blocked Bloom filter
- Reads from stdin or one input file
- Fails fast on lines that exceed the configured limit

## Build

Requires C++17 and CMake.

```bash
cmake -S . -B build
cmake --build build
```

`compile_commands.json` is generated in `build/` during configuration.

## Install

The default install prefix is `/usr/local`.

```bash
cmake --install build
```

## Usage

```bash
./build/buniq [-c cardinality] [-p precision] [-s seed] [input_file]
```

Options:

- `-c` cardinality parameter, default: `8`
- `-p` precision parameter, default: `6`
- `-s` Bloom filter seed, default: random

Notes:

- If `input_file` is omitted, input is read from stdin
- Options are parsed with `getopt`, so they usually come before the file argument

## Behavior

- Input is processed line by line with a maximum line length of `128`
- Longer lines raise `Line Too Long`
- First-seen lines are printed immediately

## Project Layout

- `src/buniq.cpp`: program entrypoint
- `src/bloom_filter.hpp`: blocked, cache-friendly Bloom filter implementation
- `src/murmur3.h`: hash function implementation

## Example

```bash
printf 'a\nb\na\n' | ./build/buniq
```

## License

This project is licensed under the [Creative Commons Attribution-NonCommercial 4.0 International](https://creativecommons.org/licenses/by-nc/4.0/) license.
