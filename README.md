# KFS (Kernel From Scratch)
Implementing a very simplistic kernel from scratch to discover the world of OS development for the 32 bits architecture i386.

## Overview
This project aims to build a minimalistic kernel from scratch using C and x86 assembly.
It requires the installation of an i386 cross compiler to work correctly. The installation can be done using the ``cross-compiler.sh`` script, provided in this repo.

## Compile and Run
After checking the cross compiler is available (run the installation script if you don't have it), you simply have to run:

```
make
```

It will create the kernel binary. If you prefer to have the iso file you have to run this:
```
make iso
```

You can now run the kernel using Qemu.
```bash
# Needs Qemu installed
make run_kernel
```
If Qemu is not available on your system, you can run the kernel with any virtual machine manager by using iso or directly the binary

## Features Implemented
Despite its minimalist nature, the kernel has a number of features that are as useful for debugging as they are for comfort.
### VGA Screen Handling
  - Foreground and Background color handling
  - Vertical scrolling and cursor support
  - Multiple screens like tty available (use ``LCTRL`` + ``<NUMBER>`` to switch)
### Input Handling
  - Full control of cursor using arrows, home and end keys
  - Insertion and deleting characters at any place on line
  - Support ``QWERTY`` and ``AZERTY`` keyboard layout (``LCTRL`` + ``F1`` to switch)
  - French accentuation management with azerty layout
### Minimalistic Shell
  Commands implemented for debugging purposes:
  - reboot (reboot the system)
  - halt (halt the system until next interruption)
  - stack (dump the kernel stack content)
  - dump ``adr`` (dump the content located at ``adr``)
  - int ``num`` (trigger the interruption number ``num``)
### Memory Management
  - Using pagination to correctly handle write and allocation on heap
  - Higher Half Kernel
  - Using Slab Allocator design
  - Panic if kernel runs out of memory
  - Physical Memory Helpers Functions:
    - kmalloc (return phyiscal contigous block of memory)
    - ksize (return physical size of a variable)
  - Virtual Memory Helpers Functions:
    - vmalloc (return virtual contigous block of memory)
    - vsize (return virtual size of a variable)
### Interruptions Handling
  - Handling all of 19 i386 CPU exceptions (Div by 0, Double Fault, ...)
  - Interrupt driven keyboard
  - PIT (Programmable Interval Timer)
  - Handle Kernel Panic when necessary
### Minimalistic syscall handler
  - Interrupt number 69
  - Register EAX contains syscall number and return value
  - Registers EBX, ECX, EDX, ESI, EDI contains arguments
  - Write syscall implemented as POC
