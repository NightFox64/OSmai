#include <windows.h>
#include <string.h>
#include <stdio.h>
#include "demidovStdio.h"

#define BUFFER_SIZE 1024

int main() {
    char fileName[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];
    DWORD bytesWritten;

    HANDLE hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, BUFFER_SIZE, "SharedMemory");
    if (hMapFile == NULL) {
        demidov_printf("Failed to create file mapping\n");
        return ERROR_CREATE_FILE_MAPPING;
    }

    HANDLE hSemParent = CreateSemaphore(NULL, 1, 1, "SemaphoreParent");
    HANDLE hSemChild = CreateSemaphore(NULL, 0, 1, "SemaphoreChild");
    if (hSemParent == NULL || hSemChild == NULL) {
        CloseHandle(hMapFile);
        demidov_printf("Failed to create semaphore\n");
        return ERROR_CREATE_SEMAPHORE;
    }

    char* pBuf = (char*) MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, BUFFER_SIZE);
    if (pBuf == NULL) {
        CloseHandle(hMapFile);
        CloseHandle(hSemParent);
        CloseHandle(hSemChild);
        demidov_printf("Failed to map view of file\n");
        return ERROR_MAP_VIEW_OF_FILE;
    }

    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), "Enter file name: ", strlen("Enter file name: "), &bytesWritten, NULL);
    ReadFile(GetStdHandle(STD_INPUT_HANDLE), fileName, BUFFER_SIZE, &bytesWritten, NULL);
    fileName[bytesWritten - 2] = '\0';

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    char cmdLine[BUFFER_SIZE];
    snprintf(cmdLine, BUFFER_SIZE, "child.exe %s", fileName);

    if (!CreateProcess(NULL, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        UnmapViewOfFile(pBuf);
        CloseHandle(hMapFile);
        CloseHandle(hSemParent);
        CloseHandle(hSemChild);
        demidov_printf("Failed to create process\n");
        return ERROR_CREATE_CHILD_PROCESS;
    }

    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), "Enter command: ", strlen("Enter command: "), &bytesWritten, NULL);
    ReadFile(GetStdHandle(STD_INPUT_HANDLE), buffer, BUFFER_SIZE, &bytesWritten, NULL);

    WaitForSingleObject(hSemParent, INFINITE);
    strcpy(pBuf, buffer);
    ReleaseSemaphore(hSemChild, 1, NULL);

    WaitForSingleObject(hSemParent, INFINITE);
    if (strcmp(pBuf, "DIVIDE_BY_ZERO") == 0) {
        WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), "Division by zero detected. Exiting...\n", strlen("Division by zero detected. Exiting...\n"), &bytesWritten, NULL);
    }

    UnmapViewOfFile(pBuf);
    CloseHandle(hMapFile);
    CloseHandle(hSemParent);
    CloseHandle(hSemChild);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return SUCCESS;
}
