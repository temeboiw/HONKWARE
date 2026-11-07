#include "include/uefi.h"

EFI_STATUS __attribute__((ms_abi))
efi_main(EFI_HANDLE image, EFI_SYSTEM_TABLE *system)
{
    (void)image;

    static CHAR16 message[] = {
        'H','e','l','l','o',' ',
        'F','r','o','m',' ',
        'G','o','o','s','e',' ',
        'U','E','F','I','!',
        '\r','\n',
        'S','y','s','t','e','m',' ',
        'h','a','l','t','e','d','.',
        '\r','\n',0
    };

    system->ConOut->OutputString(system->ConOut, message);

    for (;;) {
        __asm__ volatile("cli; hlt");
    }

    return 0;
}
