// TODO: Dialog�� ���� ��� ������ ������, Dialog�� ���� ��� Thread�� ������ ����. (SHBrowseFolder, IFileDialog)
// TODK: ������ �ذ��� ���ϸ� ���ڿ��� ��θ� �ް� ���� ������ �ִ��� Ȯ������ ���.
/*
#include <windows.h>
#include <shobjidl.h>
#include <shlobj_core.h>

#include <iostream>

#include "Utils.h"

PWCHAR GetFolderPath() {
    IFileDialog *pIFileDialog = NULL;
    
    CoInitialize(NULL);

    CoCreateInstance(
        CLSID_FileOpenDialog,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pIFileDialog)
    );

    if (!pIFileDialog) NULL;

    pIFileDialog->SetOptions(
        FOS_NOCHANGEDIR | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM |
        FOS_PATHMUSTEXIST | FOS_FILEMUSTEXIST | FOS_DONTADDTORECENT
    );
    pIFileDialog->SetTitle(
        L"DMZ Folder"
    );
    
    HRESULT hResult = pIFileDialog->Show(NULL);
    

    if (SUCCEEDED(hResult)) {
        IShellItem* pIShellItem = NULL;

        if (SUCCEEDED(pIFileDialog->GetResult(&pIShellItem))) {
            PIDLIST_ABSOLUTE pIDListAbsolute = NULL;

            SHGetIDListFromObject(pIShellItem, &pIDListAbsolute);
            pIShellItem->Release();
            pIFileDialog->Release();
            CoUninitialize();
            
            PWCHAR Path = new WCHAR[MAX_PATH];
            if (SHGetPathFromIDList(pIDListAbsolute, Path)) {
                return Path;
            }
            delete Path;
        }
    }
    CoUninitialize();
    pIFileDialog->Release();

    return NULL;
}
*/