# Simulith

A lightweight distributed time-sync middleware for simulations.

## Overview

Simulith is a high-performance middleware designed to coordinate simulation time and facilitate communication across distributed components in small satellite operational simulations.
It was developed to replace the proprietary NOS Engine within the NASA Operational Simulator for Small Satellites (NOS3), offering an open-source alternative focused on determinism, speed, and simplicity.

## Features

* Centralized simulation time control
* Support for blocking and non-blocking message transactions
* ZeroMQ-based communication backend
* Designed for running fast-time and Monte Carlo simulations
* Written in C for performance and minimal overhead

## Architecture

Simulith consists of a central time server and multiple distributed clients that may represent flight software, hardware simulators, or ground station interfaces.
Each client connects to the Simulith time server. 
The time server coordinates the simulation time and ensures all participants are synchronized for each simulation step or event.

## Quick Start

Building:
```
git clone https://github.com/jlucas9/simulith.git
cd simulith
mkdir build && cd build
cmake ..
make
```

Unit testing:
```
ctest
```

System testing:
```
./simulith_server &
./simulith_client
```

## Dependencies

* ZeroMQ (libzmq)
* CMake >= 3.10
* A C compiler (GCC, Clang, etc.)

## License

Simulith is licensed under the MIT License. See the LICENSE file for details.

## Contributing

Simulith is under active development.
Contributions are welcome!
Feel free to open issues, fork the repo, or submit pull requests.