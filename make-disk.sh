#!/bin/bash
set -e

cd "$(dirname "$0")"

IMAGE="goose-uefi.img"
SIZE_MB=64

# Required tools
for tool in mkfs.vfat mcopy mmd mdir; do
    command -v "$tool" >/dev/null || {
        echo "Missing $tool. Install dosfstools and mtools."
        exit 1
    }
done

# Ensure the UEFI executable exists
if [ ! -f BOOTX64.EFI ]; then
    echo "BOOTX64.EFI not found. Build it first."
    exit 1
fi

# Create a blank 64 MB disk image
rm -f "$IMAGE"
dd if=/dev/zero of="$IMAGE" bs=1M count="$SIZE_MB" status=progress

# Format as FAT32
mkfs.vfat -F 32 -n GOOSE_UEFI "$IMAGE"

# Create the standard UEFI boot directory
mmd -i "$IMAGE" ::EFI
mmd -i "$IMAGE" ::EFI/BOOT

# Install the bootloader
mcopy -i "$IMAGE" BOOTX64.EFI ::EFI/BOOT/BOOTX64.EFI

echo
echo "UEFI disk image created:"
ls -lh "$IMAGE"

echo
echo "Disk contents:"
mdir -i "$IMAGE" ::/
mdir -i "$IMAGE" ::/EFI/BOOT
