#!/bin/bash

# Firedrake build script, to make integration of a remote build server easier
# The whole story is that I use OS X but need Linux to build, so I have a VM that runs Debian. My IDE then
# executes the `remote-build` target to build the kernel

if [ $# -eq 0 ]; then
	ssh "${FIREDRAKE_USER}@${FIREDRAKE_HOST}" "cd ${FIREDRAKE_PATH}; ./build.sh --build; exit"
    exit 0
fi

function configureCMake {

	mkdir -p build
	cd build

	CMAKE_LINKER="<CMAKE_LINKER> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>"
	cmake "$(pwd)/.." -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER="$(which clang)" -DCMAKE_C_COMPILER="$(which clang)" -DCMAKE_LINKER="$(which ld)" -DCMAKE_CXX_LINK_EXECUTABLE="${CMAKE_LINKER}" -DCMAKE_C_LINK_EXECUTABLE="${CMAKE_LINKER}"
}

if [ "$@" == "--build" ]; then

	configureCMake
	make

	cd .. # configureCMake moves us into the ./build folder

	./initrd.py
	./boot/image.py

	exit 0
fi

if [ "$@" == "--clean" ]; then

	configureCMake
	make clean

	exit 0
fi

QEMU_NET="-net nic,model=rtl8139 -net user"
QEMU_CPU="-cpu core2duo -smp cores=2"
QEMU_ARGS="${QEMU_CPU} ${QEMU_NET} -serial stdio"

if [ "$@" == "--run" ]; then

	BASEDIR=$(dirname $0)
	qemu-system-i386 ${QEMU_ARGS} "${BASEDIR}/boot/Firedrake.iso"

	exit 0

fi

if [ "$@" == "--debug" ]; then

	BASEDIR=$(dirname $0)
	qemu-system-i386 ${QEMU_ARGS} -s -S -D /tmp/qemu.log -d int -no-shutdown -no-reboot "${BASEDIR}/boot/Firedrake.iso" &
	sleep 1
	exit 0

fi
