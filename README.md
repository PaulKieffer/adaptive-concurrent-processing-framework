# Adaptive Concurrent Processing Framework

A modern C++20 framework for concurrent task processing, workload management and performance analysis.

## Overview

The Adaptive Concurrent Processing Framework (ACPF) is a learning and portfolio project focused on modern C++ system development.

The goal is to design and implement a modular framework for concurrent task processing with a focus on:

- Concurrency
- Synchronization
- Performance
- Workload management
- Clean and maintainable C++ architecture

The project is inspired by concepts from my Bachelor's thesis on adaptive indexing in time-series databases, particularly workload-aware processing, producer-consumer architectures and concurrent execution.

## Current Status

Early development

Currently implemented:

- C++20 project structure
- CMake-based build system
- Debug and Release configurations
- CMake build and test presets
- Compiler warnings (-Wall, -Wextra, -Wpedantic)
- AddressSanitizer (ASan) configuration
- ThreadSanitizer (TSan) configuration
- clang-format configuration
- GoogleTest integration
- CTest integration
- Thread-safe TaskQueue
- Blocking task retrieval using std::condition_variable
- Controlled queue shutdown
- Unit tests for task insertion/retrieval and shutdown behavior

## TaskQueue

The TaskQueue provides the foundation for the framework's future worker and scheduling components.

Current functionality includes:

- Thread-safe task insertion
- Blocking task retrieval
- Producer-consumer synchronization
- Graceful shutdown
- Rejection of new tasks after shutdown


## Planned features:

- Additional TaskQueue concurrency tests
- Thread pool
- Worker thread management
- Task lifecycle management
- Runtime metrics
- Performance benchmarks
- Workload monitoring
- Adaptive scheduling

## Requirements

- C++20 compatible compiler
- CMake >= 3.20
- GoogleTest (managed through the project's CMake configuration)

## Build

Configure the Debug build:

```bash
cmake --preset debug
```
Build:
```bash
cmake --build --preset debug
```
Run tests:
```bash
ctest --preset debug 
```
### Release Build
```bash
cmake --preset release
cmake --build --preset release
```
### AddressSanitizer
```bash
cmake --preset asan
cmake --build --preset asan
ctest --preset asan 
```
#### ThreadSanitizer
```bash
cmake --preset tsan
cmake --build --preset tsan
ctest --preset tsan 
```
## Development

The project uses:

- CMake for build configuration
- GoogleTest for unit testing
- CTest for test execution
- AddressSanitizer for memory error detection
- ThreadSanitizer for data-race detection
- clang-format for consistent code formatting
- Git for version control

The project is developed incrementally with small, focused commits and reproducible build configurations.
