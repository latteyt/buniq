# Buniq: Blocked Bloom Filter Enpowered Uniq

[![License: CC BY-NC 4.0](https://img.shields.io/badge/License-CC%20BY--NC%204.0-blue.svg)](https://creativecommons.org/licenses/by-nc/4.0/)

`buniq` 是一个基于 blocked Bloom Filter 的多线程去重命令行工具，采用按 cache line 分块的方式提升缓存友好性。
它从标准输入或单个文件中读取文本行，实时输出首次出现的行，并在结束时打印每个 worker 的处理数量。

## 特性

- 多 worker 并发处理输入行
- 基于 blocked Bloom Filter 的快速近似去重
- 支持从 stdin 或一个文件读取
- 超长行会直接报错

## 构建

需要 C++17 和 CMake。

```bash
cmake -S . -B build
cmake --build build
```

配置时会生成 `build/compile_commands.json`，便于编辑器和静态分析工具使用。

## 安装

默认安装前缀是 `/usr/local`。

```bash
cmake --install build
```

## 使用

```bash
./build/buniq [-n worker_count] [-s scale] [-p precision] [input_file]
```

参数：

- `-n` worker 数量，默认是 `hardware_concurrency() - 1`
- `-s` Bloom Filter 规模参数，默认 `8`
- `-p` 精度参数，默认 `6`

说明：

- 不传 `input_file` 时从 stdin 读取
- 参数使用 `getopt` 解析，通常放在文件名之前

## 行为

- 输入按行处理，单行最大长度为 `1024`
- 过长输入会抛出 `Line Too Long`
- 首次出现的行会立刻输出
- 程序结束时，每个 worker 会输出一行统计信息：`worker %zu: %zu`
- worker 退出信号是一个 `len == 0` 的 item

## 代码结构

- `src/buniq.cpp`：程序入口
- `src/atomic_queue.h`：worker 间的 SPSC 队列
- `src/bloom_filter.hpp`：blocked、cache-friendly 的 Bloom Filter 实现
- `src/murmur3.h`：哈希函数

## 示例

```bash
printf 'a\nb\na\n' | ./build/buniq
```

## 许可证

本项目采用 [Creative Commons Attribution-NonCommercial 4.0 International](https://creativecommons.org/licenses/by-nc/4.0/) 许可证。
