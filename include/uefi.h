#ifndef GOOSE_UEFI_H
#define GOOSE_UEFI_H

typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned int UINT32;
typedef unsigned long long UINT64;
typedef UINT64 EFI_PHYSICAL_ADDRESS;
typedef unsigned long long UINTN;
typedef UINT64 EFI_STATUS;
typedef void *EFI_HANDLE;
typedef UINT16 CHAR16;
typedef UINT8 BOOLEAN;

#define EFIAPI __attribute__((ms_abi))
#define EFI_SUCCESS 0
#define EFI_NOT_READY 0x8000000000000006ULL
#define EFI_ERROR(s) (((s) >> 63) != 0)
#define EFI_FILE_MODE_READ 1ULL
#define EFI_FILE_DIRECTORY 0x10ULL

typedef struct {
    UINT32 Data1;
    UINT16 Data2;
    UINT16 Data3;
    UINT8 Data4[8];
} EFI_GUID;

typedef struct {
    UINT64 Signature;
    UINT32 Revision;
    UINT32 HeaderSize;
    UINT32 CRC32;
    UINT32 Reserved;
} EFI_TABLE_HEADER;

typedef struct {
    UINT16 ScanCode;
    CHAR16 UnicodeChar;
} EFI_INPUT_KEY;

typedef struct EFI_SIMPLE_TEXT_INPUT_PROTOCOL EFI_SIMPLE_TEXT_INPUT_PROTOCOL;
struct EFI_SIMPLE_TEXT_INPUT_PROTOCOL {
    EFI_STATUS (EFIAPI *Reset)(EFI_SIMPLE_TEXT_INPUT_PROTOCOL *, BOOLEAN);
    EFI_STATUS (EFIAPI *ReadKeyStroke)(EFI_SIMPLE_TEXT_INPUT_PROTOCOL *, EFI_INPUT_KEY *);
    void *WaitForKey;
};

typedef struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;
struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL {
    EFI_STATUS (EFIAPI *Reset)(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *, BOOLEAN);
    EFI_STATUS (EFIAPI *OutputString)(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *, CHAR16 *);
    void *TestString;
    void *QueryMode;
    void *SetMode;
    EFI_STATUS (EFIAPI *SetAttribute)(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *, UINTN);
    EFI_STATUS (EFIAPI *ClearScreen)(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *);
    void *SetCursorPosition;
    void *EnableCursor;
    void *Mode;
};

typedef struct {
    UINT32 Type;
    UINT32 Padding;
    UINT64 PhysicalStart;
    UINT64 VirtualStart;
    UINT64 NumberOfPages;
    UINT64 Attribute;
} EFI_MEMORY_DESCRIPTOR;

typedef struct {
    UINT16 Year;
    UINT8 Month, Day, Hour, Minute, Second, Pad1;
    UINT32 Nanosecond;
    UINT16 TimeZone;
    UINT8 Daylight, Pad2;
} EFI_TIME;

typedef struct {
    UINT64 Size;
    UINT64 FileSize;
    UINT64 PhysicalSize;
    EFI_TIME CreateTime;
    EFI_TIME LastAccessTime;
    EFI_TIME ModificationTime;
    UINT64 Attribute;
    CHAR16 FileName[];
} EFI_FILE_INFO;

typedef struct EFI_FILE_PROTOCOL EFI_FILE_PROTOCOL;
struct EFI_FILE_PROTOCOL {
    UINT64 Revision;
    EFI_STATUS (EFIAPI *Open)(EFI_FILE_PROTOCOL *, EFI_FILE_PROTOCOL **, CHAR16 *, UINT64, UINT64);
    EFI_STATUS (EFIAPI *Close)(EFI_FILE_PROTOCOL *);
    void *Delete;
    EFI_STATUS (EFIAPI *Read)(EFI_FILE_PROTOCOL *, UINTN *, void *);
    void *Write;
    void *GetPosition;
    EFI_STATUS (EFIAPI *SetPosition)(EFI_FILE_PROTOCOL *, UINT64);
    void *GetInfo;
    void *SetInfo;
    void *Flush;
    void *OpenEx;
    void *ReadEx;
    void *WriteEx;
    void *FlushEx;
};

typedef struct {
    UINT64 Revision;
    EFI_STATUS (EFIAPI *OpenVolume)(void *, EFI_FILE_PROTOCOL **);
} EFI_SIMPLE_FILE_SYSTEM_PROTOCOL;

typedef struct {
    UINT32 Revision;
    EFI_HANDLE ParentHandle;
    void *SystemTable;
    EFI_HANDLE DeviceHandle;
    void *FilePath;
    void *Reserved;
    UINT32 LoadOptionsSize;
    void *LoadOptions;
    void *ImageBase;
    UINT64 ImageSize;
    UINT32 ImageCodeType;
    UINT32 ImageDataType;
    void *Unload;
} EFI_LOADED_IMAGE_PROTOCOL;

typedef struct {
    UINT8 Type;
    UINT8 SubType;
    UINT8 Length[2];
} EFI_DEVICE_PATH_PROTOCOL;

typedef struct {
    EFI_TABLE_HEADER Hdr;
    EFI_STATUS (EFIAPI *RaiseTPL)(UINTN);
    EFI_STATUS (EFIAPI *RestoreTPL)(UINTN);
    EFI_STATUS (EFIAPI *AllocatePages)(UINT32, UINT32, UINTN, UINT64 *);
    EFI_STATUS (EFIAPI *FreePages)(UINT64, UINTN);
    EFI_STATUS (EFIAPI *GetMemoryMap)(UINTN *, EFI_MEMORY_DESCRIPTOR *, UINTN *, UINTN *, UINT32 *);
    EFI_STATUS (EFIAPI *AllocatePool)(UINT32, UINTN, void **);
    EFI_STATUS (EFIAPI *FreePool)(void *);
    void *CreateEvent;
    void *SetTimer;
    void *WaitForEvent;
    void *SignalEvent;
    void *CloseEvent;
    void *CheckEvent;
    void *InstallProtocolInterface;
    void *ReinstallProtocolInterface;
    void *UninstallProtocolInterface;
    EFI_STATUS (EFIAPI *HandleProtocol)(EFI_HANDLE, EFI_GUID *, void **);
    void *Reserved;
    void *RegisterProtocolNotify;
    void *LocateHandle;
    void *LocateDevicePath;
    void *InstallConfigurationTable;
    EFI_STATUS (EFIAPI *LoadImage)(BOOLEAN, EFI_HANDLE, EFI_DEVICE_PATH_PROTOCOL *, void *, UINTN, EFI_HANDLE *);
    EFI_STATUS (EFIAPI *StartImage)(EFI_HANDLE, UINTN *, CHAR16 **);
    EFI_STATUS (EFIAPI *Exit)(EFI_HANDLE, EFI_STATUS, UINTN, CHAR16 *);
    EFI_STATUS (EFIAPI *UnloadImage)(EFI_HANDLE);
    void *ExitBootServices;
    void *GetNextMonotonicCount;
    void *Stall;
    void *SetWatchdogTimer;
    void *ConnectController;
    void *DisconnectController;
    void *OpenProtocol;
    void *CloseProtocol;
    void *OpenProtocolInformation;
    void *ProtocolsPerHandle;
    void *LocateHandleBuffer;
    void *LocateProtocol;
    void *InstallMultipleProtocolInterfaces;
    void *UninstallMultipleProtocolInterfaces;
    void *CalculateCrc32;
    void *CopyMem;
    void *SetMem;
    void *CreateEventEx;
} EFI_BOOT_SERVICES;

typedef struct {
    EFI_TABLE_HEADER Hdr;
    void *GetTime;
    void *SetTime;
    void *GetWakeupTime;
    void *SetWakeupTime;
    void *SetVirtualAddressMap;
    void *ConvertPointer;
    void *GetVariable;
    void *GetNextVariableName;
    void *SetVariable;
    void *GetNextHighMonotonicCount;
    void (EFIAPI *ResetSystem)(UINT32, EFI_STATUS, UINTN, CHAR16 *);
} EFI_RUNTIME_SERVICES;

typedef struct {
    EFI_TABLE_HEADER Hdr;
    CHAR16 *FirmwareVendor;
    UINT32 FirmwareRevision;
    UINT32 Padding;
    EFI_HANDLE ConsoleInHandle;
    EFI_SIMPLE_TEXT_INPUT_PROTOCOL *ConIn;
    EFI_HANDLE ConsoleOutHandle;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConOut;
    EFI_HANDLE StandardErrorHandle;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *StdErr;
    EFI_RUNTIME_SERVICES *RuntimeServices;
    EFI_BOOT_SERVICES *BootServices;
    UINTN NumberOfTableEntries;
    void *ConfigurationTable;
} EFI_SYSTEM_TABLE;

#endif
