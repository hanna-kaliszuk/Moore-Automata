# moore_aut
A simple simulation of a Moore automaton in C.

## Description
This is a dynamic allocated library that simulates a Moore automaton. It is a simple implementation of a Moore automaton 
that can be used to simulate the behavior of a Moore machine. It allows you to:
* Create a Moor automaton or a simple version of it
* Set the input of the automaton
* Set the initial state of the automaton
* Get the output of the machine
* Connect and disconnect the machines
* Make a transition between states
* Delete the automaton and all its connections

In order for the `ma.h` library to work properly, you need to use the additional library `ma_additional.h` which is 
included in the repository. 

It also includes an example program that demonstrates how to use the library.

## Makefile
The Makefile is included in the repository, and it can be used to build a shared library (`libma.so`) and an example 
executable (`ma_example.c`).

* **Compiler**: `gcc`
* **Targets**:
  - `all`        - builds both the shared library and the example program
  - `libma.so`   - compiles source files into object files then links them into a shared library
  - `ma_example` - compiles the example program
  - `clean`      - removes all generated object files, the shared library and the example program
* **Source files**:
  - `SRC`:         `ma.c`, `ma_additional.c`
  - `EXAMPLE_SRC`: `ma_example.c`
* **Usage**:
```bash
make # to build the shared library and the example program
make clean # to remove all generated object files, the shared library and the example program
```

## Installation
In order to build the library and example program: 
1. Clone the repository:
```bash
git clone https://github.com/hanna-kaliszuk/moore_aut.git
cd moore_aut
```
2. Build the project using the provided 'makefile':
```bash
make
```
This will create the shared library `libma.so` and the example program `ma_example`.

3. To clean up generated files, run:
```bash
make clean
```

