/* MusicKeyboard's Windows NT Native API Wrapper header.
 *
 * This header emulates <winternl.h> for the poor compilers without this
 * header, and allows for the GetProcAddress of some functions in ntdll.dll,
 * which allows for on demand beep start and stop.
 */

#pragma once
#ifndef id3F62391D_3432_4F1C_A9E03EA4B121A1A5
#define id3F62391D_3432_4F1C_A9E03EA4B121A1A5

typedef LONG NTSTATUS;

#ifndef _WINIOCTL_
    typedef struct _BEEP_PARAM {
        ULONG Frequency;
        ULONG Duration;
    } BEEP_PARAM, *PBEEP_PARAM, *LPBEEP_PARAM;

    #define CTL_CODE( DeviceType, Function, Method, Access ) (\
        ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
    )
    #define FILE_DEVICE_BEEP 0x00000001
    #define METHOD_BUFFERED 0
    #define FILE_ANY_ACCESS 0
    #define IOCTL_BEEP_SET CTL_CODE(FILE_DEVICE_BEEP,0,METHOD_BUFFERED,FILE_ANY_ACCESS)
#endif /* _WINIOCTL_ */

#ifndef _WINTERNL_
    typedef struct _UNICODE_STRING {
        USHORT Length;
        USHORT MaximumLength;
        PWSTR  Buffer;
    } UNICODE_STRING, *PUNICODE_STRING;
    typedef const UNICODE_STRING *PCUNICODE_STRING;

    typedef struct _OBJECT_ATTRIBUTES {
        ULONG Length;
        HANDLE RootDirectory;
        PUNICODE_STRING ObjectName;
        ULONG Attributes;
        PVOID SecurityDescriptor;
        PVOID SecurityQualityOfService;
    } OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;

    typedef struct _IO_STATUS_BLOCK {
        union {
            NTSTATUS Status;
            PVOID Pointer;
        } DUMMYUNIONNAME;
        ULONG_PTR Information;
    } IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;
#endif /* _WINTERNL_ */

#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#endif

#ifndef FILE_OPEN_IF
#define FILE_OPEN_IF 0x00000003
#endif

#ifndef InitializeObjectAttributes
#define InitializeObjectAttributes( p, n, a, r, s ) { \
    (p)->Length = sizeof( OBJECT_ATTRIBUTES );          \
    (p)->RootDirectory = r;                             \
    (p)->Attributes = a;                                \
    (p)->ObjectName = n;                                \
    (p)->SecurityDescriptor = s;                        \
    (p)->SecurityQualityOfService = NULL;               \
    }
#endif

#ifndef NTAPI
#   define NTAPI WINAPI
#endif

typedef VOID (NTAPI *T_RtlInitUnicodeString)(PUNICODE_STRING DestinationString, PCWSTR SourceString);
typedef NTSTATUS (NTAPI *T_NtCreateFile)(PHANDLE FileHandle, ACCESS_MASK DesiredAccess,
            POBJECT_ATTRIBUTES ObjectAttributes, PIO_STATUS_BLOCK IoStatusBlock,
            PLARGE_INTEGER AllocationSize OPTIONAL, ULONG FileAttributes,
            ULONG ShareAccess, ULONG CreateDisposition, ULONG CreateOptions,
            PVOID EaBuffer OPTIONAL, ULONG EaLength);
#endif
