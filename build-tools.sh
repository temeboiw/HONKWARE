#!/bin/bash
set -e
cd "$(dirname "$0")"

for tool in clang lld-link mkfs.vfat mmd mcopy; do
    command -v "$tool" >/dev/null || {
        echo "Missing tool: $tool"
        exit 1
    }
done

test -f BOOTX64.EFI || {
    echo "BOOTX64.EFI missing. Build your main menu first."
    exit 1
}

CFLAGS=(
    --target=x86_64-pc-windows-msvc
    -O2
    -ffreestanding
    -fno-stack-protector
    -fno-builtin
    -fno-exceptions
    -mno-red-zone
)

build_app() {
    NAME="$1"
    SOURCE="$2"

    echo "Building $NAME..."
    clang "${CFLAGS[@]}" -c "$SOURCE" -o "tools/$NAME.obj"

    lld-link \
        /subsystem:efi_application \
        /entry:efi_main \
        /nodefaultlib \
        /machine:x64 \
        "/out:tools/$NAME.EFI" \
        "tools/$NAME.obj"
}

build_app MEMTEST tools/memtest.c
build_app SYSINFO tools/sysinfo.c
build_app REBOOT tools/reboot.c
build_app POWEROFF tools/poweroff.c

echo "Creating disk image..."
rm -f goose-uefi.img
dd if=/dev/zero of=goose-uefi.img bs=1M count=64 status=none
mkfs.vfat -F 32 -n GOOSE_UEFI goose-uefi.img

mmd -i goose-uefi.img ::EFI
mmd -i goose-uefi.img ::EFI/BOOT
mmd -i goose-uefi.img ::EFI/TOOLS

mcopy -i goose-uefi.img BOOTX64.EFI ::EFI/BOOT/BOOTX64.EFI

for app in MEMTEST SYSINFO REBOOT POWEROFF; do
    mcopy -i goose-uefi.img "tools/$app.EFI" "::EFI/TOOLS/$app.EFI"
done

echo
echo "===== GOOSE UEFI TOOLS ====="
file tools/*.EFI
echo
mdir -i goose-uefi.img ::EFI/TOOLS
echo
ls -lh goose-uefi.img
