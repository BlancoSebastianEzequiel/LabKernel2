#!/usr/bin/env bash
clear
make clean
make qemu QEMU_EXTRA='-append "param1=hola param2=adios"'
#make qemu QEMU_EXTRA="-m 256"
#make qemu-gdb
make clean