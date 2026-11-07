#ifndef GOOSE_TOOLS_COMMON_H
#define GOOSE_TOOLS_COMMON_H

#include "../include/uefi.h"

static EFI_SYSTEM_TABLE *ST;
static EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *OUT;
static EFI_SIMPLE_TEXT_INPUT_PROTOCOL *IN;

static void print(const char *s)
{
    CHAR16 b[512];
    UINTN i = 0;
    while (s[i] && i < 510) {
        b[i] = (UINT8)s[i];
        i++;
    }
    b[i] = 0;
    OUT->OutputString(OUT, b);
}

static void number(UINT64 n)
{
    char b[32];
    UINTN i = 0;
    if (!n) {
        print("0");
        return;
    }
    while (n && i < 31) {
        b[i++] = '0' + n % 10;
        n /= 10;
    }
    while (i) {
        char c[2] = { b[--i], 0 };
        print(c);
    }
}

static void pause_key(void)
{
    EFI_INPUT_KEY k;
    print("\r\nPress any key to exit...");
    while (IN->ReadKeyStroke(IN, &k) == EFI_NOT_READY)
        __asm__ volatile("pause");
}

static void init(EFI_SYSTEM_TABLE *s, const char *title)
{
    ST = s;
    OUT = s->ConOut;
    IN = s->ConIn;
    OUT->SetAttribute(OUT, 0x0A);
    OUT->ClearScreen(OUT);
    print("========================================\r\n");
    print("             GOOSE UEFI TOOLS\r\n");
    print("========================================\r\n\r\n");
    OUT->SetAttribute(OUT, 0x0F);
    print(title);
    print("\r\n----------------------------------------\r\n\r\n");
}

#endif
