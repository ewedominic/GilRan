#include <fltKernel.h>
#include <dontuse.h>
#include <suppress.h>
#include <ntstrsafe.h>

#include "PreCreate.h"
#include "Utils.h"

FLT_PREOP_CALLBACK_STATUS
PreCreate(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID *CompletionContext
)
{
    UNREFERENCED_PARAMETER(CompletionContext);

    NTSTATUS status;

    WCHAR FilePath[1024], VolumeName[1024];

    status = GetFilePath(Data, FilePath);
    if (!NT_SUCCESS(status)) return FLT_PREOP_COMPLETE;

    status = GetVolumeName(FltObjects, VolumeName);
    if (!NT_SUCCESS(status)) return FLT_PREOP_COMPLETE;

    DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_INFO_LEVEL, "FilePath: %ws%ws\n", VolumeName, FilePath);

    return FLT_PREOP_SUCCESS_NO_CALLBACK;
}