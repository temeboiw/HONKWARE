#include "common.h"

static void print_vendor(CHAR16 *vendor)
{
    if (!vendor) {
        print("Unknown");
        return;
    }

    CHAR16 b[256];
    UINTN i = 0;
    while (vendor[i] && i < 254) {
        b[i] = vendor[i];
        i++;
    }
    b[i] = 0;
    OUT->OutputString(OUT, b);
}

EFI_STATUS EFIAPI efi_main(EFI_HANDLE image, EFI_SYSTEM_TABLE *system)
{
    (void)image;
    init(system, "SYSTEM INFORMATION");

    print("Firmware vendor: ");
    print_vendor(ST->FirmwareVendor);
    print("\r\nFirmware revision: ");
    number(ST->FirmwareRevision);
    print("\r\nUEFI specification revision: ");
    number(ST->Hdr.Revision >> 16);
    print(".");
    number(ST->Hdr.Revision & 0xFFFF);
    print("\r\n");

    static UINT8 buffer[131072] __attribute__((aligned(8)));
    UINTN size = sizeof(buffer);
    UINTN key = 0;
    UINTN desc_size = 0;
    UINT32 version = 0;

    EFI_STATUS status = ST->BootServices->GetMemoryMap(
        &size,
        (EFI_MEMORY_DESCRIPTOR *)buffer,
        &key,
        &desc_size,
        &version
    );

    if (!EFI_ERROR(status) && desc_size >= sizeof(EFI_MEMORY_DESCRIPTOR)) {
        UINTN count = size / desc_size;
        UINT64 total_pages = 0;
        UINT64 conventional_pages = 0;

        for (UINTN i = 0; i < count; i++) {
            EFI_MEMORY_DESCRIPTOR *d =
                (EFI_MEMORY_DESCRIPTOR *)(buffer + i * desc_size);
            total_pages += d->NumberOfPages;
            if (d->Type == 7)
                conventional_pages += d->NumberOfPages;
        }

        print("\r\nMemory map entries: ");
        number(count);
        print("\r\nDescriptor size: ");
        number(desc_size);
        print(" bytes\r\nTotal described memory: ");
        number((total_pages * 4096) / (1024 * 1024));
        print(" MB\r\nConventional memory: ");
        number((conventional_pages * 4096) / (1024 * 1024));
        print(" MB\r\n");
    } else {
        print("\r\nCould not retrieve memory map.\r\n");
    }

    pause_key();
    return EFI_SUCCESS;
}
