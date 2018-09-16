#include <fltKernel.h>
#include <dontuse.h>
#include <suppress.h>
#include <ntstrsafe.h>
#include "Utils.h"

NTSTATUS
GetFilePath (
    _In_ PFLT_CALLBACK_DATA Data,
    _Out_ PWCHAR pFilePath
)
{
    PFLT_FILE_NAME_INFORMATION pFileNameInformation;
    NTSTATUS status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &pFileNameInformation);
    if (!NT_SUCCESS(status)) return status;

    status = FltParseFileNameInformation(pFileNameInformation);
    if (!NT_SUCCESS(status)) return status;

    wcscpy_s(pFilePath, pFileNameInformation->ParentDir.Length, pFileNameInformation->ParentDir.Buffer);

    FltReleaseFileNameInformation(pFileNameInformation);

    return STATUS_SUCCESS;
}

NTSTATUS
GetVolumeName(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Out_ PWCHAR pVolumeName
)
{
    NTSTATUS status;

    UNICODE_STRING VolumeName;
    VolumeName.Length = 0;
    VolumeName.MaximumLength = FltObjects->FileObject->FileName.MaximumLength + 2;

    ULONG szBufferNeeded;
    status = FltGetVolumeName(FltObjects->Volume, NULL, &szBufferNeeded);

    if (status == STATUS_BUFFER_TOO_SMALL) {
        VolumeName.MaximumLength += (USHORT)szBufferNeeded;
    }

    VolumeName.Buffer = ExAllocatePoolWithTag(NonPagedPool, VolumeName.MaximumLength, 'vLIG');
    if (VolumeName.Buffer == NULL) return STATUS_UNSUCCESSFUL;

    status = FltGetVolumeName(FltObjects->Volume, &VolumeName, &szBufferNeeded);
    if (NT_SUCCESS(status)) {
        wcscpy_s(pVolumeName, VolumeName.Length, VolumeName.Buffer);
    }

    ExFreePoolWithTag(VolumeName.Buffer, 'vLIG');
    return status;
}