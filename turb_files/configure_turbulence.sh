#!/bin/bash
gcc configure_turbulence.c -o test
echo "Compiled ^^"
./test
rm test

