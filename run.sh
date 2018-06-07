#!/usr/bin/env bash
clear
make clean
make qemu QEMU_EXTRA='-append "param=hola param=adios"'
make clean