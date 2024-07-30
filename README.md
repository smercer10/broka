# Broka

Broka is a multithreaded C++ order book implementation that supports numerous order types. It was created as a learning project, and is certainly **not** intended for consequential use.

[![MIT License](https://img.shields.io/badge/License-MIT-green.svg)](https://github.com/smercer10/broka/blob/main/LICENSE)
[![GitHub Actions Workflow Status](https://img.shields.io/github/actions/workflow/status/smercer10/broka/ci.yml?label=CI)](https://github.com/smercer10/broka/actions/workflows/ci.yml)

## Functionality

The primary API supports the following actions:

- Place an order
- Cancel an order
- Modify an order
- Retrieve basic order book data (e.g., total number of outstanding orders, quantity at each side/price level)

## Order Types

Five order types are currently supported:

- Day (not well-tested)
- Fill or kill
- Good 'til cancelled
- Immediate or cancel
- Market

## Build Locally

### Prerequisites

- C++20 compiler
- CMake 3.20 (can probably use 3.12+ but only 3.20 has been tested)
- Make (any recent version should be fine)

### Commands

- Build the CMake targets:

```bash
make
```

- Run the tests:

```bash
make test
```

- Clean the build directory:

```bash
make clean
```
