#include <windows.h>
#include <fltuser.h>
#include <dontuse.h>
#include <suppress.h>
#include <iostream>
#include <string>

#include "../Common/Common.h"

#define CLIENT_DEFAULT_REQUEST_COUNT 4
#define CLIENT_DEFAULT_THREAD_COUNT 4
#define CLIENT_MAX_THREAD_COUNT 64

typedef struct _THREAD_CONTEXT {
    HANDLE hCommunicationPort;
    HANDLE hIoCompletionPort;
} THREAD_CONTEXT, *PTHREAD_CONTEXT;

typedef struct _FILTER_MESSAGE {
    FILTER_MESSAGE_HEADER Header;
    PORT_REQUEST Request;
    OVERLAPPED Overlapped;
} FILTER_MESSAGE, *PFILTER_MESSAGE;

typedef struct _CLIENT_MESSAGE {
    FILTER_REPLY_HEADER Header;
    PORT_RESPONSE Response;
} CLIENT_MESSAGE, *PCLIENT_MESSAGE;

DWORD ClientWorker(
    _In_ PTHREAD_CONTEXT pThreadContext
) {
    HRESULT hResult;

    DWORD NumberOfBytesTransferred;
    ULONG_PTR CompletionKey;
    LPOVERLAPPED lpOverlapped;

    PFILTER_MESSAGE pFilterMessage;

    while (TRUE) {
        hResult = GetQueuedCompletionStatus(
            pThreadContext->hIoCompletionPort,
            &NumberOfBytesTransferred,
            &CompletionKey,
            &lpOverlapped,
            INFINITE
        );

        pFilterMessage = CONTAINING_RECORD(lpOverlapped, FILTER_MESSAGE, Overlapped);

        if (!hResult) {
            hResult = HRESULT_FROM_WIN32(GetLastError());
            break;
        }

        printf("\n\n------------------------------ \nPID: %p\nVolumeName: %ws\nFileName: %ws\n", pFilterMessage->Request.ProcessID, pFilterMessage->Request.VolumeName, pFilterMessage->Request.FilePath);

        CLIENT_MESSAGE ClientMessage;
        ClientMessage.Header.Status = 0;
        ClientMessage.Header.MessageId = pFilterMessage->Header.MessageId;
        ClientMessage.Response.Access = TRUE;

        hResult = FilterReplyMessage(
            pThreadContext->hCommunicationPort,
            (PFILTER_REPLY_HEADER)&ClientMessage.Header,
            sizeof(FILTER_REPLY_HEADER) + sizeof(PORT_RESPONSE)
        );

        if (!SUCCEEDED(hResult)) break;

        RtlZeroMemory(&pFilterMessage->Overlapped, sizeof(OVERLAPPED));
        
        hResult = FilterGetMessage(
            pThreadContext->hCommunicationPort,
            &pFilterMessage->Header,
            FIELD_OFFSET(FILTER_MESSAGE, Overlapped),
            &pFilterMessage->Overlapped
        );

        if (hResult != HRESULT_FROM_WIN32(ERROR_IO_PENDING)) break;
    }
    free(pFilterMessage);
    return hResult;
}

int _cdecl main(
    _In_ int argc,
    _In_reads_(argc) char *argv[]
)
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    HANDLE hCommunicationPort;
    HRESULT hResult = FilterConnectCommunicationPort(
        PORT_NAME,
        0,
        NULL,
        0,
        NULL,
        &hCommunicationPort
    );

    if (IS_ERROR(hResult)) return 2;

    HANDLE hIoCompletionPort = CreateIoCompletionPort(
        hCommunicationPort,
        NULL,
        0,
        CLIENT_DEFAULT_THREAD_COUNT
    );

    THREAD_CONTEXT ThreadContext;
    ThreadContext.hCommunicationPort = hCommunicationPort;
    ThreadContext.hIoCompletionPort = hIoCompletionPort;

    HANDLE hThread[CLIENT_MAX_THREAD_COUNT];
    DWORD ThreadID;

    int loop;
    for (loop = 0; loop < CLIENT_DEFAULT_THREAD_COUNT; loop++) {
        hThread[loop] = CreateThread(
            NULL,
            0,
            (LPTHREAD_START_ROUTINE)ClientWorker,
            &ThreadContext,
            0,
            &ThreadID
        );

        if (hThread[loop] == NULL) {
            CloseHandle(hCommunicationPort);
            CloseHandle(hIoCompletionPort);

            return GetLastError();
        }

        for (int joop = 0; joop < CLIENT_DEFAULT_REQUEST_COUNT; joop++) {
            PFILTER_MESSAGE pFilterMessage = (PFILTER_MESSAGE)malloc(sizeof(FILTER_MESSAGE));

            if (pFilterMessage == NULL) {
                CloseHandle(hCommunicationPort);
                CloseHandle(hIoCompletionPort);

                return ERROR_NOT_ENOUGH_MEMORY;
            }

            RtlZeroMemory(&pFilterMessage->Overlapped, sizeof(OVERLAPPED));

            hResult = FilterGetMessage(
                hCommunicationPort,
                &pFilterMessage->Header,
                FIELD_OFFSET(FILTER_MESSAGE, Overlapped),
                &pFilterMessage->Overlapped
            );

            if (hResult != HRESULT_FROM_WIN32(ERROR_IO_PENDING)) {
                free(pFilterMessage);
                CloseHandle(hCommunicationPort);
                CloseHandle(hIoCompletionPort);

                return hResult;
            }
        }
    }
    WaitForMultipleObjectsEx(loop, hThread, TRUE, INFINITE, FALSE);

    CloseHandle(hCommunicationPort);
    CloseHandle(hIoCompletionPort);

    return S_OK;
}