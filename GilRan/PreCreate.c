#include "PreCreate.h"

FLT_PREOP_CALLBACK_STATUS
PreCreate(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID *CompletionContext
)
{
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);

    NTSTATUS status;

    PFLT_FILE_NAME_INFORMATION pFileNameInformation;
    status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &pFileNameInformation);

    UNICODE_STRING FileName, VolumeName;
    if (NT_SUCCESS(status)) {
        status = FltParseFileNameInformation(pFileNameInformation);

        if (NT_SUCCESS(status)) {
            FileName.Length = 0;
            FileName.MaximumLength = NTSTRSAFE_UNICODE_STRING_MAX_CCH * sizeof(WCHAR);
            FileName.Buffer = ExAllocatePoolWithTag(NonPagedPool, FileName.MaximumLength, 'FLIG');

            if (FileName.Buffer != NULL) {
                RtlUnicodeStringCopy(&FileName, &(pFileNameInformation->Name));

                VolumeName.Length = 0;
                VolumeName.MaximumLength = FltObjects->FileObject->FileName.MaximumLength + 2;

                ULONG BufferSizeNeeded;
                status = FltGetVolumeName(FltObjects->Volume, NULL, &BufferSizeNeeded);

                if (status == STATUS_BUFFER_TOO_SMALL) {
                    VolumeName.MaximumLength += (USHORT)BufferSizeNeeded;
                }

                VolumeName.Buffer = ExAllocatePoolWithTag(NonPagedPool, VolumeName.MaximumLength, 'VLIG');
                if (VolumeName.Buffer != NULL) {
                    status = FltGetVolumeName(FltObjects->Volume, &VolumeName, &BufferSizeNeeded);

                    if (NT_SUCCESS(status)) {
                        DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_INFO_LEVEL, "FilePath: %ws%ws\n", VolumeName.Buffer, FileName.Buffer);
                    }
                    ExFreePoolWithTag(VolumeName.Buffer, 'VLIG');
                }
                ExFreePoolWithTag(FileName.Buffer, 'FLIG');
            }
        }
        FltReleaseFileNameInformation(pFileNameInformation);
    }

    return FLT_PREOP_SUCCESS_NO_CALLBACK;
}