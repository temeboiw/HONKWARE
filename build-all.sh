#!/bin/bash
set -e
cd "$(dirname "$0")"

echo "================================"
echo "       GOOSE UEFI BUILD"
echo "================================"

CFLAGS=(
    --target=x86_64-pc-windows-msvc
    -O2
    -ffreestanding
    -fno-stack-protector
    -fno-builtin
    -fno-exceptions
    -fno-asynchronous-unwind-tables
    -mno-red-zone
    -mno-stack-arg-probe
)

echo "[1/3] Compiling boot menu..."
clang "${CFLAGS[@]}" -c menu.c -o menu.obj

echo "[2/3] Compiling freestanding runtime..."
clang "${CFLAGS[@]}" -c runtime.c -o runtime.obj

echo "[3/3] Linking UEFI application..."
lld-link \
    /subsystem:efi_application \
    /entry:efi_main \
    /nodefaultlib \
    /machine:x64 \
    /out:BOOTX64.EFI \
    menu.obj runtime.obj

echo "[4/4] Building EFI utilities..."
./build-tools.sh

echo "Verifying boot image..."
file BOOTX64.EFI tools/*.EFI

echo
echo "GOOSE UEFI BUILD COMPLETE"
ls -lh BOOTX64.EFI goose-uefi.img tools/*.EFI
