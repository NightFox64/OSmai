#include <windows.h>
#include <string.h>
#include <stdio.h>
#include "demidovStdio.h"

#define BUFFER_SIZE 1024

int main(int argc, char* argv[]) {
    if (argc < 2) {
        demidov_printf("Wrong args count");
        return ERROR_ARGS_COUNT;
    }

    char* fileName = argv[1];
    HANDLE hFile;

    HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "SharedMemory");
    HANDLE hSemParent = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "SemaphoreParent");
    HANDLE hSemChild = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "SemaphoreChild");
    if (hMapFile == NULL || hSemParent == NULL || hSemChild == NULL) {
        demidov_printf("Failed to open shared memory or semaphore\n");
        return ERROR_OPEN_FILE_MAPPING;
    }

    char* pBuf = (char*) MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, BUFFER_SIZE);
    if (pBuf == NULL) {
        CloseHandle(hMapFile);
        CloseHandle(hSemParent);
        CloseHandle(hSemChild);
        demidov_printf("Failed to map view of file\n");
        return ERROR_MAP_VIEW_OF_FILE;
    }

    WaitForSingleObject(hSemChild, INFINITE);
    char buffer[BUFFER_SIZE];
    strcpy(buffer, pBuf);

    hFile = CreateFile(fileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        UnmapViewOfFile(pBuf);
        CloseHandle(hMapFile);
        CloseHandle(hSemParent);
        CloseHandle(hSemChild);
        demidov_printf("Failed to open file");
        return ERROR_OPEN_FILE;
    }

    int numbers[BUFFER_SIZE];
    int count = 0;
    char* token = strtok(buffer, " ");
    while (token != NULL) {
        numbers[count++] = atoi(token);
        token = strtok(NULL, " ");
    }

    int result = numbers[0];
    char writeBuffer[BUFFER_SIZE];

    for (int i = 1; i < count; i++) {
        if (numbers[i] == 0) {
            strcpy(pBuf, "DIVIDE_BY_ZERO");
            ReleaseSemaphore(hSemParent, 1, NULL);
            UnmapViewOfFile(pBuf);
            CloseHandle(hMapFile);
            CloseHandle(hSemParent);
            CloseHandle(hSemChild);
            CloseHandle(hFile);
            return ERROR_DEV_ZERO;
        }
        snprintf(writeBuffer, BUFFER_SIZE, "%d ", result / numbers[i]);
        DWORD bytesWritten;
        WriteFile(hFile, writeBuffer, strlen(writeBuffer), &bytesWritten, NULL);
    }

    strcpy(pBuf, "COMPLETED");
    ReleaseSemaphore(hSemParent, 1, NULL);

    UnmapViewOfFile(pBuf);
    CloseHandle(hMapFile);
    CloseHandle(hSemParent);
    CloseHandle(hSemChild);
    CloseHandle(hFile);

    return SUCCESS;
}
