#include <fltKernel.h>
#include <dontuse.h>
#include <suppress.h>

#include "../Common/Common.h"
#include "Port.h"

PORT_INFORMATION PortInformation;

NTSTATUS
ClientConnect(
    _In_ PFLT_PORT ClientPort,
    _In_opt_ PVOID ServerPortCookie,
    _In_reads_bytes_opt_(SizeOfContext) PVOID ConnectionContext,
    _In_ ULONG SizeOfContext,
    _Outptr_result_maybenull_ PVOID *ConnectionCookie
)
{
    UNREFERENCED_PARAMETER(ServerPortCookie);
    UNREFERENCED_PARAMETER(ConnectionContext);
    UNREFERENCED_PARAMETER(SizeOfContext);
    UNREFERENCED_PARAMETER(ConnectionCookie);

    FLT_ASSERT(PortInformation.ClientPort == NULL);
    FLT_ASSERT(PortInformation.UserProcess == NULL);

    PortInformation.UserProcess = PsGetCurrentProcess();
    PortInformation.ClientPort = ClientPort;

    return STATUS_SUCCESS;
}

VOID
ClientDisConnect(
    _In_opt_ PVOID ConnectionCookie
)
{
    UNREFERENCED_PARAMETER(ConnectionCookie);

    PortInformation.UserProcess = NULL;
    FltCloseClientPort(PortInformation.Filter, &PortInformation.ClientPort);
}

NTSTATUS
PortSendMessage(
    _In_ PPORT_REQUEST pPortRequest,
    _Out_ PBOOLEAN Access
)
{
    ULONG SizeOfResponse = sizeof(PORT_RESPONSE);
    PPORT_RESPONSE pPortResponse = ExAllocatePoolWithTag(NonPagedPool, SizeOfResponse, 'vLIG');

    if (pPortResponse == NULL) return STATUS_UNSUCCESSFUL;

    NTSTATUS status = FltSendMessage(
        PortInformation.Filter,
        &PortInformation.ClientPort,
        pPortRequest,
        sizeof(PORT_REQUEST),
        pPortResponse,
        &SizeOfResponse,
        NULL
    );
    *Access = pPortResponse->Access;
    
    ExFreePoolWithTag(pPortResponse, 'vLIG');
    return status;
}