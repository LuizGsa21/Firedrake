## Overview
Firedrake is a very simple kernel for the i386 architecture, without any deeper intention besides learning a bit about low level and system programming. Its designed by an absolute newling to kernel hacking, so it might include some stupid/uncommong design decisions! Although there are no plans to make a *nix like system, some of API follows the POSIX specification and there are plans to widen this out in the future, but some things might have their own API which isn't POSIX conform.

## Features
Currently implemented into Firedrake are the following features:

  * Interrupt handling
  * Memory management (virtual and physical)
  * Preemptive multitasking
  * Support for processes and multithreading (can be loaded from ELF binaries)
  * Support for syscalls
  * Kernel wide unit tests

## Compiling
### Prerequisites
You probably have the most succes with compiling Firedrake if you are running some kind Linux flavor with the following packages installed:

  * llvm
  * clang
  * make

If you want to create mountable ISO files, you also need the following packages:

  * grub-rescue-pc (only needed if you want to create mountable ISOs)
  * xorriso (only needed if you want to create mountable ISOs)

You also need to mark the `/boot/CreateImage.sh` as executable by running `chmod +x ./boot/CreateImage.sh`.

### Make
To make a complete build of Firedrake, you only need to run `make`, which will compile the kernel as well as all default libraries and programs. If you want to create a bootable ISO file, you need to run the `make install` target. 
You can also run the `help` target which prints a list of targets that are available together with an description of what they do.

## Executing
If you want to run Firedrake but don't want to reboot your PC for it, you can use an emulator. Firedrake has been tested in `qemu` and `VMWare Fusion`, but should also run in `Virtual Box`, `Bochs` and almost every other emulator which emulates a x86 CPU and a PC BIOS.