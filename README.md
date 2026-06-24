# modularEpoll

A dependency-free, zero-copy, zero-allocation C++23 module abstraction layer over the native Linux epoll asynchronous event monitoring subsystem.

## Overview

This repository isolates the legacy `<sys/epoll.h>` interface within a strict compile-time module boundary. By utilizing the explicit structural packing attribute `[[gnu::packed]]`, this library achieves a 1:1 binary memory layout configuration identical to the underlying Linux kernel's `struct epoll_event`. 

This structural mapping completely eliminates intermediate data translation layers, heap allocation tracking vectors, and secondary buffer data-shuffling overhead inside the active runtime loop execution path.

## Design Highlights

* **Zero-Allocation Waiting:** The `wait` interface accepts a standard `std::span` buffer configuration and streams kernel updates directly into application memory space.
* **100% Macro Encapsulation:** System flags like `EPOLLIN` and `EPOLLET` are captured locally and purged via compiler directives to prevent user-space pollution.
* **Type-Safe Bitwise Management:** Implements customized operator logic over strong enum structures for clean event filtering configurations.
* **Strict Single-Ownership:** The main event engine leverages explicit move semantics and utilizes modern `std::exchange` boundaries to enforce safe resource handling.

## Prerequisites

To build and integrate this standalone object file, your system requires:
* Linux operating system kernel
* GCC 11+ supporting C++20/C++23 Modules (`-fmodules-ts`)

## Pipeline Architecture

* `modularEpoll.cppm` : Core module definition containing the zero-copy inline assembly forwarding interface.
* `build.sh` : Standalone build script automating local standard library module instantiation and driver compilation tasks.

## Compilation

The wrapper provides a single-command build file designed to compile the module natively as an independent framework component.

To execute the object generation toolchain, run:
```bash
chmod +x build.sh
./build.sh
```

The underlying pipeline performs an optimization pass mapping to:
```bash
g++ -std=c++23 -fmodules-ts -O3 -c modularEpoll.cppm -o modularEpoll.o
```

This output generates the following deployment objects:
1. `modularEpoll.o` : The standalone binary artifact ready for downstream project linkage.
2. `gcm.cache/` : The standard library compiler cache directory tree required for interface resolution.

## Operational Interface Example

Once compiled via your project's main build configuration toolchain, the engine interface can be imported directly into operational execution pathways:

```cpp
import modularEpoll;
import std;

int main() {
    try {
        Async::EventLoop loop;
        
        // Register arbitrary file descriptors with explicit type safety
        int mock_fd = 0; 
        loop.add_fd(mock_fd, Async::EventFlags::Read | Async::EventFlags::EdgeTriggered, nullptr);

        std::array<Async::Event, 16> event_buffer;
        
        // High-efficiency kernel-to-user pass-through call
        int ready = loop.wait(std::chrono::milliseconds(100), event_buffer);
        
        for (int i = 0; i < ready; ++i) {
            if (Async::has_flag(event_buffer[i].events, Async::EventFlags::Read)) {
                // Execute low-latency processing steps here
            }
        }
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
```

## License
GNU GPLv3 License

Copyright (c) 2026 mxreal64

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see <https://gnu.org/licenses>.
