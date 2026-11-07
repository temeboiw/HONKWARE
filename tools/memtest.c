#include "common.h"

EFI_STATUS EFIAPI efi_main(EFI_HANDLE image, EFI_SYSTEM_TABLE *system)
{
    (void)image;
    init(system, "MEMTEST - ALLOCATED MEMORY TEST");

    const UINTN megabytes = 8;
    const UINTN pages = megabytes * 256;
    EFI_PHYSICAL_ADDRESS address = 0;

    print("Allocating ");
    number(megabytes);
    print(" MB of test memory...\r\n");

    EFI_STATUS status = ST->BootServices->AllocatePages(
        0, 2, pages, &address
    );

    if (EFI_ERROR(status)) {
        print("ERROR: Could not allocate test memory.\r\n");
        pause_key();
        return status;
    }

    volatile UINT64 *mem = (volatile UINT64 *)(UINTN)address;
    UINTN words = (megabytes * 1024 * 1024) / sizeof(UINT64);
    const UINT64 patterns[] = {
        0x0000000000000000ULL,
        0xFFFFFFFFFFFFFFFFULL,
        0xAAAAAAAAAAAAAAAAULL,
        0x5555555555555555ULL
    };

    UINTN errors = 0;

    for (UINTN p = 0; p < 4; p++) {
        print("Testing pattern ");
        number(p + 1);
        print("/4... ");

        for (UINTN i = 0; i < words; i++)
            mem[i] = patterns[p];

        for (UINTN i = 0; i < words; i++) {
            if (mem[i] != patterns[p]) {
                errors++;
                if (errors == 1) {
                    print("Mismatch at word ");
                    number(i);
                    print("\r\n");
                }
            }
        }

        if (!errors)
            print("PASS\r\n");
        else
            print("ERRORS DETECTED\r\n");
    }

    print("\r\nTested memory: ");
    number(megabytes);
    print(" MB\r\nErrors: ");
    number(errors);
    print("\r\n");

    ST->BootServices->FreePages(address, pages);

    if (errors) {
        OUT->SetAttribute(OUT, 0x0C);
        print("MEMTEST FAILED\r\n");
    } else {
        OUT->SetAttribute(OUT, 0x0A);
        print("MEMTEST PASSED\r\n");
    }

    pause_key();
    return EFI_SUCCESS;
}
