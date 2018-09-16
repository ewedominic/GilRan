#pragma once

#define PORT_NAME L"\\GilRanPort"
#define PORT_BUFFER_SIZE 1024

typedef struct _PORT_REQUEST {
    HANDLE ProcessID;
    WCHAR VolumeName[PORT_BUFFER_SIZE];
    WCHAR FilePath[PORT_BUFFER_SIZE];
} PORT_REQUEST, *PPORT_REQUEST;

typedef struct _PORT_RESPONSE {
    BOOLEAN Access;
} PORT_RESPONSE, *PPORT_RESPONSE;