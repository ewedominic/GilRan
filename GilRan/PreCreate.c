#include <fltKernel.h>
#include <dontuse.h>
#include <suppress.h>
#include <ntstrsafe.h>

#include "../Common/Common.h"
#include "PreCreate.h"
#include "Port.h"
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

    PORT_REQUEST PortRequest;

    status = GetFilePath(Data, PortRequest.FilePath);
    if (!NT_SUCCESS(status)) return FLT_PREOP_SUCCESS_NO_CALLBACK;

    status = GetVolumeName(FltObjects, PortRequest.VolumeName);
    if (!NT_SUCCESS(status)) return FLT_PREOP_SUCCESS_NO_CALLBACK;

    PortRequest.ProcessID = PsGetCurrentProcessId();

    BOOLEAN Access;
    status = PortSendMessage(&PortRequest, &Access);

    if (NT_SUCCESS(status) && !Access) {
        FltCancelFileOpen(FltObjects->Instance, FltObjects->FileObject);

        Data->IoStatus.Status = STATUS_ACCESS_DENIED;
        Data->IoStatus.Information = 0;

        return FLT_PREOP_COMPLETE;
    }
    return FLT_PREOP_SUCCESS_NO_CALLBACK;
}