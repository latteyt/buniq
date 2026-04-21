# buniq

[![License: CC BY-NC 4.0](https://img.shields.io/badge/License-CC%20BY--NC%204.0-blue.svg)](https://creativecommons.org/licenses/by-nc/4.0/)

`buniq` is a multithreaded line de-duplication CLI built around a blocked, cache-friendly Bloom filter.
It reads text lines from stdin or a single file, prints first-seen lines immediately, and reports per-worker counts at exit.

## Features

- Concurrent worker-based input processing
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

## Usage

```bash
./build/buniq [-n worker_count] [-s scale] [-p precision] [input_file]
```

Options:

- `-n` number of workers, default: `hardware_concurrency() - 1`
- `-s` Bloom filter scale parameter, default: `8`
- `-p` precision parameter, default: `6`

Notes:

- If `input_file` is omitted, input is read from stdin
- Options are parsed with `getopt`, so they usually come before the file argument

## Behavior

- Input is processed line by line with a maximum line length of `1024`
- Longer lines raise `Line Too Long`
- First-seen lines are printed immediately
- Each worker prints a final counter line: `worker %zu: %zu`
- A queue item with `len == 0` is used as the worker shutdown sentinel

## Project Layout

- `src/buniq.cpp`: program entrypoint
- `src/atomic_queue.h`: SPSC queues between producer and workers
- `src/bloom_filter.hpp`: blocked, cache-friendly Bloom filter implementation
- `src/murmur3.h`: hash function implementation

## Example

```bash
printf 'a\nb\na\n' | ./build/buniq
```

## License

This project is licensed under the [Creative Commons Attribution-NonCommercial 4.0 International](https://creativecommons.org/licenses/by-nc/4.0/) license.
