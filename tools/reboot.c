#include "common.h"

EFI_STATUS EFIAPI efi_main(EFI_HANDLE image, EFI_SYSTEM_TABLE *system)
{
    (void)image;
    init(system, "SYSTEM REBOOT");

    print("The machine will reboot when you press a key.\r\n");
    pause_key();

    ST->RuntimeServices->ResetSystem(0, EFI_SUCCESS, 0, 0);

    for (;;)
        __asm__ volatile("cli; hlt");
}
