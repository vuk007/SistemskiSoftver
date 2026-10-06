
# System Software

A C++17 implementation of a small assembly toolchain and a RISC-V-inspired virtual machine, developed as part of the System Software course.

The project consists of three major components:

- **Assembler** - translates assembly source code into relocatable object files.
- **Linker** - combines object files, resolves symbols and relocations, and produces a final executable memory image.
- **Emulator** - loads the generated program and executes it on a software-emulated CPU with virtual memory and terminal I/O.

The complete workflow is:

```text
Assembly source
      │
      ▼
   Assembler
      │
      ▼
 Object files
      │
      ▼
    Linker
      │
      ▼
 program.hex
      │
      ▼
   Emulator
      │
      ▼
 Virtual CPU + Memory + Terminal
```

---

## Project Overview

The project implements the main stages of a simplified system software toolchain.

The assembler accepts assembly programs and produces relocatable object files containing sections, symbols and relocation information.

The linker processes one or more object files and creates a single executable memory image. It combines sections, assigns addresses, resolves symbols and applies relocation records.

The emulator then loads the resulting program into virtual memory and executes its instructions using a software implementation of the target CPU.

This makes the project a complete miniature toolchain:

```text
Source code
    ↓
Lexical analysis / Parsing
    ↓
Assembler
    ↓
Relocatable object file
    ↓
Linker
    ↓
Executable memory image
    ↓
Emulator
    ↓
Program execution
```

---

# Main Features

## Assembler

The assembler implements a two-pass assembly process and supports:

- assembly source parsing
- sections
- labels
- symbols
- global symbols
- external symbols
- `.section`
- `.global`
- `.extern`
- `.word`
- `.skip`
- `.end`
- instruction encoding
- register operands
- immediate operands
- memory operands
- symbolic operands
- indirect addressing
- relocation records
- forward references
- absolute relocations
- PC-relative relocations
- object file generation
- symbol table generation
- section data generation

The assembler separates symbol resolution from final address assignment, allowing unresolved symbols to be handled through relocation records.

### Relocatable Object Files

Generated object files contain information about:

- symbols
- sections
- section sizes
- section contents
- relocation records

This allows the linker to combine independently assembled modules.

---

# Linker

The linker combines multiple relocatable object files into a single executable image.

Its main responsibilities are:

- reading object files
- reading symbol tables
- reading section data
- joining sections with the same name
- assigning final section addresses
- resolving global and external symbols
- detecting multiple symbol definitions
- resolving relocations
- checking section overlap
- generating the final memory image

## Section Joining

Sections with the same name from different object files are combined into one output section.

For example:

```text
file1.o:
    .text
    .data

file2.o:
    .text
    .data
```

can be transformed into:

```text
.text
    file1 .text
    file2 .text

.data
    file1 .data
    file2 .data
```

The linker keeps track of the original offsets so that relocation records can be correctly adjusted after sections are joined.

---

## Section Placement

The linker supports explicit section placement using the `-place` option.

For example:

```text
-place=.text@0x40000000
```

allows a section to be assigned to a specific memory address.

Sections without an explicitly assigned address are placed after the explicitly positioned sections.

The linker also checks for overlapping sections and reports invalid memory layouts.

---

# Symbol Resolution

The linker maintains information about symbols from all input object files.

It handles:

- local symbols
- global symbols
- external symbols
- defined symbols
- undefined symbols
- multiple definitions

External references are resolved against global definitions from other object files.

For example:

```text
file1.o                  file2.o

.global main             .global function

main:                    function:
    call function            ...
```

The linker resolves the reference from `file1.o` to the symbol defined in `file2.o`.

---

# Relocation

Relocation records allow the assembler to postpone address-dependent calculations until linking.

The linker supports:

- absolute relocation
- PC-relative relocation
- displacement-based relocation

### Absolute Relocation

The final symbol address is written directly into the appropriate location.

```text
value = symbol_address
```

### PC-Relative Relocation

The linker calculates the displacement relative to the current instruction:

```text
displacement =
    symbol_address - (section_base + patch_address + instruction_size)
```

The implementation also validates the supported 12-bit signed displacement range:

```text
-2048 <= displacement <= 2047
```

This allows instructions to reference symbols whose final addresses are not known during assembly.

---

# Relocatable Linking

The linker also supports relocatable output.

In relocatable mode, the input sections can be combined without performing final memory placement and relocation patching.

This allows the linker to operate both as:

```text
Object files → final executable image
```

and as:

```text
Object files → combined relocatable object
```

---

# Emulator

The emulator provides a software implementation of the target processor.

It loads the executable memory image generated by the linker and executes the program instruction by instruction.

The emulator contains several major subsystems.

---

## CPU

The virtual CPU contains:

- 16 general-purpose registers
- program counter (`PC`)
- stack pointer (`SP`)
- status CSR
- handler CSR
- cause CSR
- instruction decoding
- instruction execution

The CPU implements arithmetic, logical, memory, control-flow and CSR-related operations.

---

# Instruction Execution

The emulator performs the typical CPU execution cycle:

```text
Fetch
  ↓
Decode
  ↓
Execute
  ↓
Update CPU state
  ↓
Fetch next instruction
```

Instructions are decoded from the virtual memory and executed by updating registers, memory and control-flow state.

The emulator supports operations such as:

- arithmetic instructions
- logical instructions
- shift instructions
- load/store operations
- conditional branches
- jumps
- calls
- returns
- stack operations
- CSR operations
- software interrupts
- invalid instruction handling
- halt

---

# Stack Operations

The virtual CPU supports stack-oriented operations including:

```text
PUSH
POP
CALL
RET
```

The stack pointer is maintained by the emulator and used for procedure calls and temporary storage.

This provides the basic execution model required for function calls and nested execution.

---

# Memory

The emulator provides a software implementation of the target memory system.

It supports:

- byte access
- word access
- memory reads
- memory writes
- loading executable data
- memory-mapped devices

The linked program is loaded into virtual memory before execution starts.

---

# Memory-Mapped Terminal

Terminal I/O is implemented through memory-mapped addresses.

This allows the emulated program to communicate with the outside world through the virtual terminal without requiring direct access to the host operating system.

The emulator supports:

- terminal output
- terminal input
- keyboard polling
- terminal interrupts

The architecture can therefore be represented as:

```text
             Virtual CPU
                 │
        ┌────────┴────────┐
        │                 │
     Memory           Interrupts
        │                 │
        └───────┬─────────┘
                │
          Memory-Mapped
             Terminal
```

---

# Interrupt System

The emulator implements an interrupt mechanism using CSR registers.

Important registers include:

- `status`
- `handler`
- `cause`

The emulator handles different types of events, including:

- software interrupts
- terminal interrupts
- invalid instructions

Interrupt handling updates the appropriate CPU state and transfers execution to the configured interrupt handler.

Interrupt masking is also supported through the CPU status information.

---

# Error Handling

The emulator detects invalid execution states such as unsupported or invalid instructions.

When an invalid instruction is encountered, the appropriate exception/interrupt mechanism is triggered instead of silently continuing execution.

This makes the emulator closer to the behavior of a real processor rather than being only a simple instruction interpreter.

---

# Project Architecture

The project is divided into three major layers.

```text
┌─────────────────────────────────────┐
│             Assembly Code            │
└──────────────────┬──────────────────┘
                   │
                   ▼
┌─────────────────────────────────────┐
│              Assembler              │
│                                     │
│ Lexer → Parser → Symbol Processing  │
│       → Instruction Encoding        │
│       → Relocation Generation       │
└──────────────────┬──────────────────┘
                   │
                   ▼
┌─────────────────────────────────────┐
│          Relocatable Objects        │
│                                     │
│ Sections / Symbols / Relocations    │
└──────────────────┬──────────────────┘
                   │
                   ▼
┌─────────────────────────────────────┐
│               Linker                │
│                                     │
│ Section Joining                     │
│ Symbol Resolution                   │
│ Section Placement                   │
│ Relocation                          │
└──────────────────┬──────────────────┘
                   │
                   ▼
┌─────────────────────────────────────┐
│           Executable Image          │
└──────────────────┬──────────────────┘
                   │
                   ▼
┌─────────────────────────────────────┐
│              Emulator               │
│                                     │
│ CPU / Memory / Interrupts / I/O     │
└─────────────────────────────────────┘
```

---

# Assembler Architecture

The assembler uses lexical and syntactic analysis to process assembly source code.

The main processing stages are:

```text
Assembly source
       │
       ▼
     Lexer
       │
       ▼
    Parser
       │
       ▼
  Instruction /
  Directive processing
       │
       ▼
 Symbol table + Section data
       │
       ▼
 Relocation records
       │
       ▼
 Object file
```

The lexer and parser are generated using Flex and Bison.

---

# Data Structures

Several internal structures are used to represent the assembler and linker state.

Important concepts include:

### Symbol

Stores information about an assembly symbol, including:

- name
- section
- offset/base
- size
- definition state
- global state
- external state

### Section

Represents an individual assembly section and contains:

- name
- size
- base address
- file offset
- section data

### Relocation Record

Stores information necessary to patch an unresolved reference during linking, including:

- address
- section
- relocation size
- relocation type

### Operand Information

Instruction operands are represented through information such as:

- addressing mode
- registers
- displacement
- symbol
- indirect addressing

These structures provide the bridge between parsing assembly instructions and generating machine-level output.

---

# Build System

The project is implemented in **C++17**.

The build system uses a Makefile and integrates:

- `g++`
- Flex
- Bison

Generated parser/lexer sources are compiled together with the assembler and linker implementation.

The main executables are:

```text
asembler
linker
emulator
```

---

# Technologies

- **C++17**
- **Flex**
- **Bison**
- **Make**
- **GNU g++**
- **Object files**
- **Relocation**
- **Virtual CPU**
- **Memory-mapped I/O**

---

# Project Structure

A simplified project structure is:

```text
SistemskiSoftver/
│
├── inc/
│   ├── assembler.hpp
│   ├── linker.hpp
│   ├── emulator.hpp
│   └── ...
│
├── src/
│   ├── assembler.cpp
│   ├── linker.cpp
│   ├── emulator.cpp
│   └── ...
│
├── misc/
│   ├── flex.lex
│   ├── bison.y
│   └── ...
│
├── tests/
│   └── ...
│
├── makefile
├── Postavka.pdf
└── program.hex
```

Generated Flex/Bison files are build artifacts and can be regenerated from the source grammar definitions.

---

# Development History

The project was developed incrementally through several versions.

The main development stages were:

```text
main
  │
  ▼
emulator
  │
  ▼
linker
```

### Main

Initial project version containing the basic assembler/toolchain functionality.

### Emulator

The emulator was added, introducing:

- virtual CPU
- registers
- memory
- instruction execution
- interrupts
- terminal I/O
- executable image loading

### Linker

The linker functionality was developed further to provide:

- multiple object file processing
- section merging
- section placement
- symbol resolution
- relocation
- overlap checking
- final executable generation

This evolution turned the project from an assembler implementation into a complete toolchain capable of producing and executing programs.

---

# Example Workflow

A typical workflow is:

### 1. Write an assembly program

```asm
.section text

.global main

main:
    ; program instructions
```

### 2. Assemble

The assembler translates the source program into a relocatable object file.

```text
program.asm
      ↓
   assembler
      ↓
   program.o
```

### 3. Link

The linker combines object files and resolves addresses.

```text
program.o
module.o
    │
    ▼
 linker
    │
    ▼
program.hex
```

Optional section placement can be specified during linking.

### 4. Execute

The emulator loads the resulting image:

```text
program.hex
     ↓
 emulator
     ↓
Virtual CPU
     ↓
Program execution
```

---

# Testing

The repository contains test programs used to verify different parts of the toolchain.

Testing can cover:

- assembly syntax
- instruction encoding
- symbol resolution
- forward references
- relocation
- section merging
- linker placement
- CPU instructions
- memory access
- interrupts
- terminal interaction

The separation between assembler, linker and emulator also makes it possible to test each stage independently.

---

# Key Concepts Demonstrated

This project demonstrates practical implementation of several system software concepts:

- lexical analysis
- syntax analysis
- assembly language processing
- two-pass assembly
- symbol tables
- forward references
- relocation
- object files
- section management
- linking
- address assignment
- PC-relative addressing
- executable image generation
- CPU instruction decoding
- virtual memory
- stack management
- interrupts
- memory-mapped I/O
- terminal communication

---

# Project Status

The project contains implementations for the complete toolchain:

```text
[✓] Assembly parsing
[✓] Symbol table
[✓] Sections
[✓] Instruction encoding
[✓] Relocation records
[✓] Object file generation
[✓] Multiple object files
[✓] Section merging
[✓] Symbol resolution
[✓] Section placement
[✓] Relocation
[✓] Executable image generation
[✓] CPU emulation
[✓] Memory emulation
[✓] Interrupt handling
[✓] Terminal I/O
```

The project is intended as an educational implementation of the main mechanisms behind assemblers, linkers and processor execution environments.

---

# Academic Context

Developed as part of the **System Software** course at the University of Belgrade, School of Electrical Engineering.

The project combines compiler-construction techniques with low-level system software concepts and hardware emulation.

---

# Author

**Vuk Dinić**

GitHub: [@vuk007](https://github.com/vuk007)
