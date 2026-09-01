# Adaptive Concurrent Processing Framework

A modern C++20 framework for concurrent task processing, workload management and performance analysis.

## Overview

The goal is to design and implement a modular framework for concurrent task processing with a focus on:

 - Concurrency
 - Synchronization
 - Performance
 - Workload management
 - Clean and maintainable C++ architecture

## Current Status

Early development.

Currently implemented:

 - C++20 project structure
 - CMake-based build system
 - Debug and Release configurations
 - ASan and TSan configurations
 - GoogleTest and CTest integration
 - Thread-safe TaskQueue
 - Blocking task retrieval
 - Graceful queue shutdown
 - Multi-producer and multi-consumer support
 - ThreadPool with configurable worker count
 - Concurrent task execution
 - Cooperative worker reduction
 - Worker lifecycle management
 - Controller for coordinating queue shutdown and worker lifecycle

## Architecture

ACPF currently consists of three main components:

 - `TaskQueue` manages task storage, synchronization and queue shutdown.
 - `ThreadPool` manages worker threads and executes tasks from an external `TaskQueue`.
 - `Controller` coordinates the lifecycle of an associated `TaskQueue` and `ThreadPool`.

`ThreadPool` and `Controller` hold references to their associated objects rather than taking ownership.

The `Controller` coordinates shutdown by shutting down the queue, waiting for workers to stop and reaping stopped workers.

## TaskQueue

The `TaskQueue` provides the foundation for concurrent task processing.

Current functionality includes:
 
 - Thread-safe task insertion
 - Blocking task retrieval
 - Producer-consumer synchronization
 - Graceful shutdown
 - Rejection of new tasks after shutdown
 - Concurrent producer and consumer support
 - Queue size observation
 - Cooperative worker stop requests

## ThreadPool

The `ThreadPool` manages worker threads that consume and execute tasks from an external `TaskQueue`.

Current functionality includes:

 - Configurable worker count
 - Automatic worker startup
 - Concurrent task execution
 - Worker count observation
 - Cooperative worker reduction
 - Worker lifecycle management

Running tasks are not forcibly interrupted when workers are reduced.

## Controller

The `Controller` coordinates the lifecycle of an existing `TaskQueue` and `ThreadPool`.

It does not own either object.

During destruction, the Controller:

 1. Shuts down the TaskQueue
 2. Waits for workers to stop
 3. Reaps stopped workers

Running tasks are not forcibly interrupted. Controller destruction may therefore block until all workers have stopped.

## Tests

The project uses GoogleTest and CTest for automated testing.

The test suite covers:

 - TaskQueue functionality and shutdown semantics
 - Concurrent producer and consumer scenarios
 - ThreadPool task execution
 - Multiple workers and concurrent execution
 - Worker lifecycle and reduction
 - Controller shutdown behavior

AddressSanitizer and ThreadSanitizer are available for additional validation.

## Planned Features

 - Runtime metrics
 - Workload monitoring
 - Adaptive worker scaling
 - Adaptive scheduling
 - Performance benchmarks
 - Configurable monitoring intervals

## Requirements

 - C++20 compatible compiler
 - CMake >= 3.20
 - GoogleTest

## Build

Configure and build the Debug version:
```bash
cmake --preset debug
cmake --build --preset debug
```

Run the tests:
```bash
ctest --preset debug
```

## Release Build
```bash
cmake --preset release
cmake --build --preset release
```

## AddressSanitizer
```bash
cmake --preset asan
cmake --build --preset asan
ctest --preset asan
```

## ThreadSanitizer
```bash
cmake --preset tsan
cmake --build --preset tsan
ctest --preset tsan
```

## Development
The project uses:

 - CMake
 - GoogleTest
 - CTest
 - AddressSanitizer
 - ThreadSanitizer
 - clang-format
 - Git