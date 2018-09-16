#pragma once
NTSTATUS GetFilePath(
    _In_ PFLT_CALLBACK_DATA Data,
    _Out_ PWCHAR pFilePath
);

NTSTATUS GetVolumeName(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Out_ PWCHAR pVolumeName
);