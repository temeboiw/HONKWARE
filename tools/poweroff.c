#include "common.h"

EFI_STATUS EFIAPI efi_main(EFI_HANDLE image, EFI_SYSTEM_TABLE *system)
{
    (void)image;
    init(system, "SYSTEM SHUTDOWN");

    print("Requesting firmware shutdown...\r\n");
    pause_key();

    ST->RuntimeServices->ResetSystem(2, EFI_SUCCESS, 0, 0);

    for (;;)
        __asm__ volatile("cli; hlt");
}
