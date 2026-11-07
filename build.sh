#!/bin/bash
set -e
cd "$(dirname "$0")"

echo "[1/4] Compiling UEFI application..."

clang --target=x86_64-pc-windows-msvc \
    -O0 \
    -ffreestanding \
    -fno-stack-protector \
    -fno-builtin \
    -fno-exceptions \
    -mno-red-zone \
    -c menu.c -o menu.obj

echo "[2/4] Linking BOOTX64.EFI..."

lld-link \
    /subsystem:efi_application \
    /entry:efi_main \
    /nodefaultlib \
    /machine:x64 \
    /out:BOOTX64.EFI \
    menu.obj

echo "[3/4] Creating FAT32 disk..."

rm -f goose-uefi.img
dd if=/dev/zero of=goose-uefi.img bs=1M count=64 status=none
mkfs.vfat -F 32 -n GOOSE_UEFI goose-uefi.img

mmd -i goose-uefi.img ::EFI
mmd -i goose-uefi.img ::EFI/BOOT
mcopy -i goose-uefi.img BOOTX64.EFI ::EFI/BOOT/BOOTX64.EFI

echo "[4/4] Done!"
file BOOTX64.EFI
ls -lh BOOTX64.EFI goose-uefi.img
