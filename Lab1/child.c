#include <windows.h>
#include <string.h>
#include <stdio.h>
#include "demidovStdio.h"

#define BUFFER_SIZE 1024

int main(int argc, char* argv[]) {
    HANDLE hPipe1, hPipe2;
    char pipeName1[] = "\\\\.\\pipe\\Pipe1";
    char pipeName2[] = "\\\\.\\pipe\\Pipe2";
    char buffer[BUFFER_SIZE];
    DWORD bytesRead, bytesWritten;
    char* fileName = (char*)malloc(BUFFER_SIZE * sizeof(char*));

    if (fileName == NULL){
        demidov_printf("Failed to malloc\n");
        return ERROR_MALLOC;
    }


    HANDLE hFile;

    if (argc < 2) {
        demidov_printf("Wrong args count");
        return ERROR_ARGS_COUNT;
    }

    //snprintf(fileName, BUFFER_SIZE, argv[1]);
    fileName = argv[1];

    hPipe1 = CreateFile(pipeName1, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hPipe1 == INVALID_HANDLE_VALUE) {
        demidov_printf("Failed to create named pipe");
        return ERROR_CREATE_PIPE;
    }

    hPipe2 = CreateFile(pipeName2, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hPipe2 == INVALID_HANDLE_VALUE) {
        CloseHandle(hPipe1);
        demidov_printf("Failed to create named pipe");
        return ERROR_CREATE_PIPE;
    }

    hFile = CreateFile(((fileName)), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        CloseHandle(hPipe1);
        CloseHandle(hPipe2);
        demidov_printf("Failed to open file");
        return ERROR_OPEN_FILE;
    }

    ReadFile(hPipe1, buffer, BUFFER_SIZE, &bytesRead, NULL);

    int numbers[BUFFER_SIZE];
    int count = 0;
    char* token = strtok(buffer, " ");
    while (token != NULL) {
        numbers[count++] = atoi(token);
        token = strtok(NULL, " ");
    }

    int result = numbers[0];
    char* write = (char*)malloc(BUFFER_SIZE * sizeof(char*));

    for (int i = 1; i < count; i++) {
        if (numbers[i] == 0) {
            WriteFile(hPipe2, "DIVIDE_BY_ZERO", strlen("DIVIDE_BY_ZERO"), &bytesWritten, NULL);
            CloseHandle(hPipe1);
            CloseHandle(hPipe2);
            CloseHandle(hFile);
            return ERROR_DEV_ZERO;
        }
        snprintf(write, BUFFER_SIZE, "%d ", result / numbers[i]);
        demidov_file_printf(hFile, write, strlen(write), &bytesWritten, NULL);
    }

    CloseHandle(hPipe1);
    CloseHandle(hPipe2);
    CloseHandle(hFile);

    return SUCCESS;
}