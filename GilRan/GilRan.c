/*++

Module Name:

    GilRan.c

Abstract:

    This is the main module of the GilRan miniFilter driver.

Environment:

    Kernel mode

--*/

#include <fltKernel.h>
#include <dontuse.h>
#include <suppress.h>

#include "../Common/Common.h"
#include "PreCreate.h"
#include "Port.h"

#pragma prefast(disable:__WARNING_ENCODE_MEMBER_FUNCTION_POINTER, "Not valid for kernel mode drivers")

ULONG_PTR OperationStatusCtx = 1;

/*************************************************************************
    Prototypes
*************************************************************************/

EXTERN_C_START

DRIVER_INITIALIZE DriverEntry;
NTSTATUS
DriverEntry (
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    );

NTSTATUS
GilRanUnload (
    _In_ FLT_FILTER_UNLOAD_FLAGS Flags
    );

EXTERN_C_END

//
//  Assign text sections for each routine.
//

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, GilRanUnload)
#endif

//
//  operation registration
//

CONST FLT_OPERATION_REGISTRATION Callbacks[] = {
    { IRP_MJ_CREATE,
      0,
      PreCreate,
      NULL },

    { IRP_MJ_OPERATION_END }
};

//
//  This defines what we want to filter with FltMgr
//

CONST FLT_REGISTRATION FilterRegistration = {

    sizeof( FLT_REGISTRATION ),         //  Size
    FLT_REGISTRATION_VERSION,           //  Version
    0,                                  //  Flags

    NULL,                               //  Context
    Callbacks,                          //  Operation callbacks

    GilRanUnload,                       //  MiniFilterUnload

    NULL,                               //  InstanceSetup
    NULL,                               //  InstanceQueryTeardown
    NULL,                               //  InstanceTeardownStart
    NULL,                               //  InstanceTeardownComplete

    NULL,                               //  GenerateFileName
    NULL,                               //  GenerateDestinationFileName
    NULL                                //  NormalizeNameComponent

};

/*************************************************************************
    MiniFilter initialization and unload routines.
*************************************************************************/

NTSTATUS
DriverEntry (
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    )
/*++

Routine Description:

    This is the initialization routine for this miniFilter driver.  This
    registers with FltMgr and initializes all global data structures.

Arguments:

    DriverObject - Pointer to driver object created by the system to
        represent this driver.

    RegistryPath - Unicode string identifying where the parameters for this
        driver are located in the registry.

Return Value:

    Routine can return non success error codes.

--*/
{
    NTSTATUS status;

    UNREFERENCED_PARAMETER( RegistryPath );

    //
    //  Register with FltMgr to tell it our callback routines
    //

    status = FltRegisterFilter( DriverObject,
                                &FilterRegistration,
                                &PortInformation.Filter );

    FLT_ASSERT( NT_SUCCESS( status ) );

    if (!NT_SUCCESS(status)) return status;

    UNICODE_STRING PortName;
    RtlInitUnicodeString(&PortName, PORT_NAME);

    PSECURITY_DESCRIPTOR pSecurityDescriptor;
    status = FltBuildDefaultSecurityDescriptor(&pSecurityDescriptor, FLT_PORT_ALL_ACCESS);
    if (NT_SUCCESS(status)) {
        OBJECT_ATTRIBUTES ObjectAttributes;
        InitializeObjectAttributes(
            &ObjectAttributes,
            &PortName,
            OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
            NULL,
            pSecurityDescriptor
        );

        status = FltCreateCommunicationPort(
            PortInformation.Filter,
            &PortInformation.ServerPort,
            &ObjectAttributes,
            NULL,
            ClientConnect,
            ClientDisConnect,
            NULL,
            1
        );
        FltFreeSecurityDescriptor(pSecurityDescriptor);

        if (NT_SUCCESS(status)) {
            status = FltStartFiltering(PortInformation.Filter);

            if (NT_SUCCESS(status)) return STATUS_SUCCESS;
        }
        FltCloseCommunicationPort(PortInformation.ServerPort);
    }
    FltUnregisterFilter(PortInformation.Filter);

    return status;
}

NTSTATUS
GilRanUnload (
    _In_ FLT_FILTER_UNLOAD_FLAGS Flags
    )
/*++

Routine Description:

    This is the unload routine for this miniFilter driver. This is called
    when the minifilter is about to be unloaded. We can fail this unload
    request if this is not a mandatory unload indicated by the Flags
    parameter.

Arguments:

    Flags - Indicating if this is a mandatory unload.

Return Value:

    Returns STATUS_SUCCESS.

--*/
{
    UNREFERENCED_PARAMETER( Flags );

    PAGED_CODE();

    FltCloseCommunicationPort(PortInformation.ServerPort);
    FltUnregisterFilter( PortInformation.Filter );

    return STATUS_SUCCESS;
}