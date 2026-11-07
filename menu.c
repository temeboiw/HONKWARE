#include "include/uefi.h"

#define GREEN 0x0A
#define WHITE 0x0F
#define CYAN  0x0B
#define RED   0x0C
#define YELLOW 0x0E

static EFI_SYSTEM_TABLE *ST;
static EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *OUT;
static EFI_SIMPLE_TEXT_INPUT_PROTOCOL *IN;
static EFI_HANDLE IMAGE;

static EFI_GUID LOADED_IMAGE_GUID = {
    0x5B1B31A1, 0x9562, 0x11D2,
    {0x8E,0x3F,0x00,0xA0,0xC9,0x69,0x72,0x3B}
};

static EFI_GUID SIMPLE_FS_GUID = {
    0x964E5B22, 0x6459, 0x11D2,
    {0x8E,0x39,0x00,0xA0,0xC9,0x69,0x72,0x3B}
};

static EFI_GUID DEVICE_PATH_GUID = {
    0x09576E91, 0x6D3F, 0x11D2,
    {0x8E,0x39,0x00,0xA0,0xC9,0x69,0x72,0x3B}
};

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
        char c[2] = {b[--i], 0};
        print(c);
    }
}

static UINT16 key(void)
{
    EFI_INPUT_KEY k;

    for (;;) {
        if (IN->ReadKeyStroke(IN, &k) == EFI_SUCCESS)
            return k.UnicodeChar;

        __asm__ volatile("pause");
    }
}

static void wait_key(void)
{
    print("\r\nPress any key to continue...");
    key();
}

static void clear(void)
{
    OUT->ClearScreen(OUT);
}

static void title(const char *s)
{
    clear();
    OUT->SetAttribute(OUT, GREEN);
    print("========================================\r\n");
    print("           GOOSE UEFI SYSTEM\r\n");
    print("========================================\r\n\r\n");

    OUT->SetAttribute(OUT, WHITE);
    print(s);
    print("\r\n----------------------------------------\r\n\r\n");
}

static UINTN u16len(const CHAR16 *s)
{
    UINTN n = 0;
    while (s[n]) n++;
    return n;
}

static void copy16(CHAR16 *dst, const CHAR16 *src)
{
    UINTN i = 0;
    while (src[i]) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = 0;
}

static void append16(CHAR16 *dst, const CHAR16 *src)
{
    UINTN i = u16len(dst);
    UINTN j = 0;
    while (src[j]) {
        dst[i++] = src[j++];
    }
    dst[i] = 0;
}

static int is_efi_file(const CHAR16 *name)
{
    UINTN n = u16len(name);
    if (n < 4) return 0;

    CHAR16 a = name[n - 4];
    CHAR16 b = name[n - 3];
    CHAR16 c = name[n - 2];
    CHAR16 d = name[n - 1];

    return (a == '.' &&
            (b == 'E' || b == 'e') &&
            (c == 'F' || c == 'f') &&
            (d == 'I' || d == 'i'));
}

static int get_tools_directory(
    EFI_HANDLE image,
    EFI_LOADED_IMAGE_PROTOCOL **loaded,
    EFI_FILE_PROTOCOL **directory
)
{
    void *p = 0;

    EFI_STATUS s = ST->BootServices->HandleProtocol(
        image, &LOADED_IMAGE_GUID, &p
    );

    if (EFI_ERROR(s) || !p)
        return 0;

    *loaded = (EFI_LOADED_IMAGE_PROTOCOL *)p;

    p = 0;
    s = ST->BootServices->HandleProtocol(
        (*loaded)->DeviceHandle, &SIMPLE_FS_GUID, &p
    );

    if (EFI_ERROR(s) || !p)
        return 0;

    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *fs =
        (EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *)p;

    EFI_FILE_PROTOCOL *root = 0;

    s = fs->OpenVolume(fs, &root);
    if (EFI_ERROR(s) || !root)
        return 0;

    static CHAR16 path[] = {
        '\\','E','F','I','\\','T','O','O','L','S',0
    };

    s = root->Open(root, directory, path, EFI_FILE_MODE_READ, 0);
    root->Close(root);

    return !EFI_ERROR(s) && *directory;
}

/* Construct a device path to an EFI file on the boot disk. */
static EFI_DEVICE_PATH_PROTOCOL *make_file_path(
    EFI_HANDLE image,
    EFI_LOADED_IMAGE_PROTOCOL *loaded,
    const CHAR16 *filename
)
{
    void *p = 0;

    EFI_STATUS s = ST->BootServices->HandleProtocol(
        loaded->DeviceHandle, &DEVICE_PATH_GUID, &p
    );

    if (EFI_ERROR(s) || !p)
        return 0;

    EFI_DEVICE_PATH_PROTOCOL *base =
        (EFI_DEVICE_PATH_PROTOCOL *)p;

    static UINT8 path_buffer[2048];
    UINTN offset = 0;

    /* Copy the device path, excluding its final end node. */
    EFI_DEVICE_PATH_PROTOCOL *node = base;

    for (;;) {
        UINT16 len = node->Length[0] | ((UINT16)node->Length[1] << 8);

        if (node->Type == 0x7F)
            break;

        if (len < 4 || offset + len + 512 >= sizeof(path_buffer))
            return 0;

        for (UINTN i = 0; i < len; i++)
            path_buffer[offset++] = ((UINT8 *)node)[i];

        node = (EFI_DEVICE_PATH_PROTOCOL *)((UINT8 *)node + len);
    }

    /* Media device path: File Path. */
    UINTN chars = u16len(filename) + 1;
    UINT16 node_len = (UINT16)(4 + chars * sizeof(CHAR16));

    if (offset + node_len + 4 > sizeof(path_buffer))
        return 0;

    EFI_DEVICE_PATH_PROTOCOL *file_node =
        (EFI_DEVICE_PATH_PROTOCOL *)(path_buffer + offset);

    file_node->Type = 0x04;
    file_node->SubType = 0x04;
    file_node->Length[0] = node_len & 0xFF;
    file_node->Length[1] = node_len >> 8;

    CHAR16 *dest = (CHAR16 *)(path_buffer + offset + 4);
    copy16(dest, filename);

    offset += node_len;

    EFI_DEVICE_PATH_PROTOCOL *end =
        (EFI_DEVICE_PATH_PROTOCOL *)(path_buffer + offset);

    end->Type = 0x7F;
    end->SubType = 0xFF;
    end->Length[0] = 4;
    end->Length[1] = 0;

    return (EFI_DEVICE_PATH_PROTOCOL *)path_buffer;
}

static void launch_tool(
    EFI_HANDLE image,
    EFI_LOADED_IMAGE_PROTOCOL *loaded,
    const CHAR16 *name
)
{
    CHAR16 path[256] = {
        '\\','E','F','I','\\','T','O','O','L','S','\\',0
    };

    append16(path, name);

    EFI_DEVICE_PATH_PROTOCOL *device_path =
        make_file_path(image, loaded, path);

    if (!device_path) {
        OUT->SetAttribute(OUT, RED);
        print("Could not construct EFI device path.\r\n");
        wait_key();
        return;
    }

    EFI_HANDLE child = 0;

    EFI_STATUS status = ST->BootServices->LoadImage(
        0, image, device_path, 0, 0, &child
    );

    if (EFI_ERROR(status)) {
        OUT->SetAttribute(OUT, RED);
        print("LoadImage failed. EFI status: ");
        number(status);
        print("\r\n");
        wait_key();
        return;
    }

    UINTN exit_size = 0;
    CHAR16 *exit_data = 0;

    status = ST->BootServices->StartImage(
        child, &exit_size, &exit_data
    );

    if (EFI_ERROR(status)) {
        OUT->SetAttribute(OUT, RED);
        print("Application returned EFI error: ");
        number(status);
        print("\r\n");
        wait_key();
    }

    if (exit_data)
        ST->BootServices->FreePool(exit_data);

    if (child)
        ST->BootServices->UnloadImage(child);
}

static void tools_menu(void)
{
    EFI_LOADED_IMAGE_PROTOCOL *loaded = 0;
    EFI_FILE_PROTOCOL *dir = 0;

    if (!get_tools_directory(IMAGE, &loaded, &dir)) {
        title("EFI APPLICATION LAUNCHER");
        OUT->SetAttribute(OUT, RED);
        print("Cannot open \\EFI\\TOOLS on the boot filesystem.\r\n");
        wait_key();
        return;
    }

    for (;;) {
        title("EFI APPLICATION LAUNCHER");

        OUT->SetAttribute(OUT, CYAN);
        print("Scanning \\EFI\\TOOLS\\ ...\r\n\r\n");
        OUT->SetAttribute(OUT, WHITE);

        CHAR16 names[16][128];
        UINTN count = 0;

        dir->SetPosition(dir, 0);

        for (;;) {
            UINT8 info_buffer[1024] __attribute__((aligned(8)));
            UINTN info_size = sizeof(info_buffer);

            EFI_STATUS s = dir->Read(dir, &info_size, info_buffer);

            if (EFI_ERROR(s) || info_size == 0)
                break;

            EFI_FILE_INFO *info = (EFI_FILE_INFO *)info_buffer;

            if ((info->Attribute & EFI_FILE_DIRECTORY) ||
                !is_efi_file(info->FileName))
                continue;

            if (count >= 16)
                break;

            copy16(names[count], info->FileName);

            print("  ");
            number(count + 1);
            print(". ");

            for (UINTN i = 0; names[count][i]; i++) {
                CHAR16 one[2] = {names[count][i], 0};
                OUT->OutputString(OUT, one);
            }

            print("\r\n");
            count++;
        }

        if (!count) {
            print("No EFI applications found.\r\n");
        }

        print("\r\n  0. Back to main menu\r\n\r\n");
        OUT->SetAttribute(OUT, GREEN);
        print("Select application> ");
        OUT->SetAttribute(OUT, WHITE);

        UINT16 choice = key();

        if (choice == '0') {
            dir->Close(dir);
            return;
        }

        if (choice >= '1' && choice <= '9') {
            UINTN index = choice - '1';

            if (index < count) {
                launch_tool(IMAGE, loaded, names[index]);
            }
        }
    }
}

static void terminal(void)
{
    title("GOOSE KEYBOARD TERMINAL");
    print("Type text. ESC returns to the menu.\r\n\r\ngoose> ");

    for (;;) {
        UINT16 c = key();

        if (c == 27)
            return;

        if (c == 13) {
            print("\r\n");
            continue;
        }

        if (c == 8 || c == 127) {
            print("\b \b");
            continue;
        }

        if (c >= 32 && c <= 126) {
            CHAR16 b[2] = {c, 0};
            OUT->OutputString(OUT, b);
        }
    }
}

static void memory_info(void)
{
    title("UEFI MEMORY INFORMATION");

    static UINT8 buffer[131072] __attribute__((aligned(8)));

    UINTN size = sizeof(buffer);
    UINTN map_key = 0;
    UINTN desc_size = 0;
    UINT32 version = 0;

    EFI_STATUS s = ST->BootServices->GetMemoryMap(
        &size,
        (EFI_MEMORY_DESCRIPTOR *)buffer,
        &map_key,
        &desc_size,
        &version
    );

    if (EFI_ERROR(s) || desc_size < sizeof(EFI_MEMORY_DESCRIPTOR)) {
        print("Unable to retrieve memory map.\r\n");
        wait_key();
        return;
    }

    UINTN entries = size / desc_size;
    UINT64 conventional_pages = 0;

    for (UINTN i = 0; i < entries; i++) {
        EFI_MEMORY_DESCRIPTOR *d =
            (EFI_MEMORY_DESCRIPTOR *)(buffer + i * desc_size);

        if (d->Type == 7)
            conventional_pages += d->NumberOfPages;
    }

    print("Memory map entries: ");
    number(entries);
    print("\r\nConventional memory: ");
    number((conventional_pages * 4096) / (1024 * 1024));
    print(" MB\r\n");

    wait_key();
}

static void screen_settings(void)
{
    title("SCREEN SETTINGS");

    print("1. White\r\n2. Green\r\n3. Cyan\r\n");
    print("4. Red\r\n5. Yellow\r\n");
    print("\r\nChoose color: ");

    switch (key()) {
        case '1': OUT->SetAttribute(OUT, WHITE); break;
        case '2': OUT->SetAttribute(OUT, GREEN); break;
        case '3': OUT->SetAttribute(OUT, CYAN); break;
        case '4': OUT->SetAttribute(OUT, RED); break;
        case '5': OUT->SetAttribute(OUT, YELLOW); break;
        default: break;
    }

    print("\r\nColor changed.");
    wait_key();
}

EFI_STATUS EFIAPI efi_main(EFI_HANDLE image, EFI_SYSTEM_TABLE *system)
{
    IMAGE = image;
    ST = system;
    OUT = system->ConOut;
    IN = system->ConIn;

    for (;;) {
        title("GOOSE UEFI BOOT MENU");

        OUT->SetAttribute(OUT, WHITE);
        print("  1. Keyboard Terminal\r\n");
        print("  2. Memory Information\r\n");
        print("  3. Screen Settings\r\n");
        print("  4. EFI Application Launcher\r\n");
        print("  5. Halt System\r\n\r\n");

        OUT->SetAttribute(OUT, GREEN);
        print("goose> ");
        OUT->SetAttribute(OUT, WHITE);

        switch (key()) {
            case '1':
                terminal();
                break;
            case '2':
                memory_info();
                break;
            case '3':
                screen_settings();
                break;
            case '4':
                tools_menu();
                break;
            case '5':
                title("SYSTEM HALTED");
                print("GOOSE UEFI has halted.");
                for (;;)
                    __asm__ volatile("cli; hlt");
            default:
                break;
        }
    }

    return EFI_SUCCESS;
}
