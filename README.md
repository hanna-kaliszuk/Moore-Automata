# Moore Automata

A C implementation of a dynamically linked library for simulating binary Moore automata, with support for state transitions, signal connections, and synchronous execution of multiple automata.

> Project for **Architektura Komputerów i Systemy Operacyjne** (Computer Architecture and Operating Systems), summer semester 2024/25, University of Warsaw.

## What is Moore Automata?

A Moore automaton is a finite-state machine whose output depends only on its current state.

The library implements binary Moore automata with:

* `n` one-bit input signals,
* `m` one-bit output signals,
* an `s`-bit internal state,
* a transition function defining the next state,
* an output function defining the output based on the current state.

Signals are stored as sequences of bits packed into arrays of `uint64_t`.

The library supports both individual automata and networks of connected automata. Output signals of one automaton can be connected to input signals of another, allowing more complex systems to be constructed from smaller components.

## Features

* Creating full Moore automata with custom transition and output functions
* Creating simplified automata with an identity output function
* Setting the input and state of an automaton
* Reading the current output
* Connecting output signals to input signals of another automaton
* Disconnecting existing connections
* Performing synchronous transitions of multiple automata
* Dynamic memory allocation and deallocation
* Handling invalid arguments and memory allocation failures
* Building the implementation as a shared library (`libma.so`)
* Testing allocation failures and memory leaks

## Technologies

### Language and Build Tools

* C
* GCC
* GNU Make
* Shared libraries (`.so`)
* Function pointers
* Dynamic memory allocation
* Bitwise operations

### Testing and Debugging

* Valgrind
* Linker wrapping for memory allocation failure testing
* Provided memory testing infrastructure

## How the Automata Work

Each automaton consists of input signals, output signals, and an internal state.

The transition function calculates the next state based on the current state and input:

```text
current state + input
          │
          ▼
 transition function
          │
          ▼
      next state
```

The output function determines the output from the current state:

```text
current state
      │
      ▼
 output function
      │
      ▼
     output
```

Automata can also be connected together:

```text
+-------------+       +-------------+
| Automaton A | ----> | Automaton B |
|   outputs   |       |    inputs   |
+-------------+       +-------------+
```

When several automata are processed together, `ma_step()` performs their transition synchronously. The next state of each automaton is calculated using the values from before the transition, and the states are updated only after all next states have been determined.

This makes it possible to build larger synchronous systems from interconnected Moore automata.

## Memory Management

The implementation dynamically allocates memory for automata, signal arrays, and connection structures.

Allocation failures are explicitly handled. If an operation cannot be completed, already allocated resources are released and the existing data structures are kept consistent.

The project uses the memory testing infrastructure provided with the assignment. It allows allocation failures to be simulated for functions including:

* `malloc`
* `calloc`
* `realloc`
* `reallocarray`
* `strdup`
* `strndup`

Memory leaks were also checked using Valgrind.

## Building

The project uses GCC and GNU Make.

Build the shared library:

```bash
make libma.so
```

Build the shared library and the example program:

```bash
make
```

Remove generated files:

```bash
make clean
```

The resulting shared library is:

```text
libma.so
```

The project is compiled using the flags required by the course assignment:

```text
-Wall -Wextra -Wno-implicit-fallthrough
-std=gnu17
-fPIC
-O2
```

The shared library is linked with the required linker wrapping options for memory allocation testing.

## Running

After building the project:

```bash
./ma_example
```

The example program demonstrates how to create and use Moore automata through the public library interface.

## Project Structure

```text
.
├── ma.c                   # Main library implementation
├── ma.h                   # Public library interface
├── ma_additional.c        # Internal helper functions
├── ma_additional.h        # Internal data structures and helpers
├── ma_example.c           # Example program
├── memory_tests.c         # Memory allocation testing
├── memory_tests.h         # Memory testing declarations
├── Makefile               # Build configuration
└── README.md
```

### Implementation

The main implementation is contained in:

* `ma.c` — implementation of the public Moore automaton interface
* `ma_additional.c` — internal data structures, signal handling, and connection management
* `ma_additional.h` — internal structures and helper functions

### Files Provided with the Assignment

The following files were provided by the university as part of the assignment:

* `ma.h` — public API of the library
* `ma_example.c` — example program demonstrating the library
* `memory_tests.c` and `memory_tests.h` — infrastructure for testing memory allocation failures

The implementation was written around the provided public interface.

## Useful Commands

### Build the library

```bash
make libma.so
```

### Build everything

```bash
make
```

### Run the example

```bash
./ma_example
```

### Remove generated files

```bash
make clean
```

## Notes

* The library uses `uint64_t` arrays to store arbitrary-length sequences of binary signals.
* Inputs can either be set directly or obtained from connected automata.
* Connected automata are processed synchronously.
* The implementation does not impose artificial limits on the number of signals apart from available memory and the machine word representation.
* `ma.h` defines the public interface and was provided as part of the assignment.
* The project was tested with simulated allocation failures and Valgrind to verify memory safety.
* The repository contains both the implementation and the supporting files provided with the course assignment.

## Course

This project was developed as a course assignment for **Computer Architecture and Operating Systems** at the University of Warsaw during the Summer Semester 2024/25.