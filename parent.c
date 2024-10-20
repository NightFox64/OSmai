#include <windows.h>
#include <string.h>
#include <stdio.h>
#include "demidovStdio.h"

#define BUFFER_SIZE 1024

int main() {
    HANDLE hPipe1, hPipe2;
    char pipeName1[] = "\\\\.\\pipe\\Pipe1";
    char pipeName2[] = "\\\\.\\pipe\\Pipe2";
    char fileName[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];
    DWORD bytesRead, bytesWritten;

    hPipe1 = CreateNamedPipe(pipeName1, PIPE_ACCESS_OUTBOUND, PIPE_TYPE_BYTE | PIPE_WAIT, 1, 0, 0, 0, NULL);
    if (hPipe1 == INVALID_HANDLE_VALUE) {
        demidov_printf("Failed to create named pipe");
        return ERROR_CREATE_PIPE;
    }

    hPipe2 = CreateNamedPipe(pipeName2, PIPE_ACCESS_INBOUND, PIPE_TYPE_BYTE | PIPE_WAIT, 1, 0, 0, 0, NULL);
    if (hPipe2 == INVALID_HANDLE_VALUE) {
        CloseHandle(hPipe1);
        demidov_printf("Failed to create named pipe");
        return ERROR_CREATE_PIPE;
    }

    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), "Enter file name: ", strlen("Enter file name: "), &bytesWritten, NULL);
    ReadFile(GetStdHandle(STD_INPUT_HANDLE), fileName, BUFFER_SIZE, &bytesRead, NULL);
    fileName[bytesRead - 2] = '\0';

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    char cmdLine[BUFFER_SIZE];
    snprintf(cmdLine, BUFFER_SIZE, "child.exe %s", fileName);

    if (!CreateProcess(NULL, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        CloseHandle(hPipe1);
        CloseHandle(hPipe2);
        demidov_printf("Failed to create process");
        return ERROR_CREATE_CHILD_PROCESS;
    }

    ConnectNamedPipe(hPipe1, NULL);

    ConnectNamedPipe(hPipe2, NULL);

    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), "Enter command: ", strlen("Enter command: "), &bytesWritten, NULL);
    ReadFile(GetStdHandle(STD_INPUT_HANDLE), buffer, BUFFER_SIZE, &bytesRead, NULL);

    WriteFile(hPipe1, buffer, bytesRead, &bytesWritten, NULL);

    char response[BUFFER_SIZE];
    ReadFile(hPipe2, response, BUFFER_SIZE, &bytesRead, NULL);
    if (strcmp(response, "DIVIDE_BY_ZERO") == 0) {
        WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), "Division by zero detected. Exiting...\n", strlen("Division by zero detected. Exiting...\n"), &bytesWritten, NULL);
    }

    CloseHandle(hPipe1);
    CloseHandle(hPipe2);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return SUCCESS;
}