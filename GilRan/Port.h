#pragma once
typedef struct _PORT_INFORMATION {
    PDRIVER_OBJECT DriverObject;
    PFLT_FILTER Filter;
    PFLT_PORT ServerPort;
    PEPROCESS UserProcess;
    PFLT_PORT ClientPort;
} PORT_INFORMATION, *PPORT_INFORMATION;

extern PORT_INFORMATION PortInformation;

NTSTATUS
ClientConnect(
    _In_ PFLT_PORT ClientPort,
    _In_opt_ PVOID ServerPortCookie,
    _In_reads_bytes_opt_(SizeOfContext) PVOID ConnectionContext,
    _In_ ULONG SizeOfContext,
    _Outptr_result_maybenull_ PVOID *connectionCookie
);

VOID
ClientDisConnect(
    _In_opt_ PVOID ConnectionCookie
);

NTSTATUS
PortSendMessage(
    _In_ PPORT_REQUEST pPortRequest,
    _Out_ PBOOLEAN Acces
);