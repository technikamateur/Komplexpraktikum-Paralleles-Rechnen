#!/bin/bash

echo "Creating 7zip archive for you."
echo "Algorithm: LZMA, max compression."
7z a -t7z -m0=lzma -mx=9 Aufgabe_C.7z *.c *.sh *.py *.h pics/ Bericht/Praktikumsbericht.pdf objdump/ results/ README.md
md5sum Aufgabe_C.7z > Aufgabe_C.7z.md5
md5=$(md5sum Aufgabe_C.7z)
echo "MD5sum: $md5"
