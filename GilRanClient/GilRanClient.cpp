#include <windows.h>
#include <fltuser.h>
#include <dontuse.h>
#include <suppress.h>

#include <list>
#include <iomanip>
#include <iostream>
#include <string>
#include <future>
#include <thread>

#include "../Common/Common.h"
#include "Utils.h"

#define CLIENT_DEFAULT_REQUEST_COUNT 4
#define CLIENT_DEFAULT_THREAD_COUNT 4
#define CLIENT_MAX_THREAD_COUNT 64

typedef struct _THREAD_CONTEXT {
    HANDLE hCommunicationPort;
    HANDLE hIoCompletionPort;
    std::list<PWCHAR> arrFolderPath;
} THREAD_CONTEXT, *PTHREAD_CONTEXT;

typedef struct _FILTER_MESSAGE {
    FILTER_MESSAGE_HEADER Header;
    PORT_REQUEST Request;
    OVERLAPPED Overlapped;
} FILTER_MESSAGE, *PFILTER_MESSAGE;

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

        PORT_RESPONSE Response;
        Response.Header.Status = 0;
        Response.Header.MessageId = pFilterMessage->Header.MessageId;
        Response.Access = TRUE;

        hResult = FilterReplyMessage(
            pThreadContext->hCommunicationPort,
            (PFILTER_REPLY_HEADER)&Response,
            sizeof(PORT_RESPONSE)
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
    
    THREAD_CONTEXT ThreadContext;
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
    
    int Command = TRUE;
    while (Command) {
        std::wcout << std::endl;
        std::wcout << "================================================================================" << std::endl;
        std::wcout << "|    GilRan Ransomware Solution v0.1a                                          |" << std::endl;
        std::wcout << "================================================================================" << std::endl;
        std::wcout << "|    [1] DMZ Folder -- SHOW                                                     |" << std::endl;
        std::wcout << "|    [2] DMZ Folder --  ADD                                                     |" << std::endl;
        std::wcout << "|    [0] Exit                                                                   |" << std::endl;
        std::wcout << "================================================================================" << std::endl;
        std::wcout << "Select: ";
        
        std::cin >> Command;
        if (!std::cin) {
            std::cin.clear();
            std::cin.ignore(INT_MAX, '\n');
            Command = TRUE;
        }
        
        switch (Command) {
            case 1:
                for (auto FolderPath : ThreadContext.arrFolderPath) {
                    std::wcout << FolderPath << std::endl;
                }
                break;
            case 2:
                PWCHAR Path = new WCHAR[MAX_PATH];

                std::wcout << L"Input folder path to make DMZ Folder" << std::endl;
                std::wcout << L">";
                std::wcin >> Path;

                ThreadContext.arrFolderPath.push_back(Path);
                break;
        }
    }

    CloseHandle(hCommunicationPort);
    CloseHandle(hIoCompletionPort);
    
    return S_OK;
}