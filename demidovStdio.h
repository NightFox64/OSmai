#define INITIAL_BUFFER_SIZE 128

#include <windows.h>
#include <stdarg.h>
#include <string.h>

enum ret_type_t{
    SUCCESS,    //Successful end
    ERROR_ARGS_COUNT,    //Wrong args number
    ERROR_CREATE_PIPE,  //Failed to create a new pipeline
    ERROR_CREATE_CHILD_PROCESS, //Failed to create a child process
    ERROR_READ, //Failed to read from pipe
    ERROR_DEV_ZERO, //Devision by zero detected
    ERROR_FULL, //Overflow
    ERROR_OPEN_FILE,    //Error with file opening
    ERROR_CLOSE_FILE,   //Error with closing file
    ERROR_FILE_WRITE,   //Error with file writing
    ERROR_HANDLER_INHERITED, //Error handler reading
    ERROR_PIPE_WRITE,   //Failed to write smth in the pipe
    ERROR_HEAP,         //Failed to malloc
};

//Use to get
char* demidov_getline(HANDLE hFile) {
    int bufferSize = INITIAL_BUFFER_SIZE;
    char* buffer = (char*)HeapAlloc(GetProcessHeap(), 0, bufferSize);
    if (buffer == NULL) {
        return NULL;
    }

    int pos = 0;
    while (1) {
        if (pos >= bufferSize - 1) {
            bufferSize *= 2;
            char* newBuffer = (char*)HeapReAlloc(GetProcessHeap(), 0, buffer, bufferSize);
            if (newBuffer == NULL) {
                HeapFree(GetProcessHeap(), 0, buffer);
                return NULL;
            }
            buffer = newBuffer;
        }

        DWORD bytesRead;
        if (!ReadFile(hFile, buffer + pos, 1, &bytesRead, NULL)) {
            HeapFree(GetProcessHeap(), 0, buffer);
            return NULL;
        }

        if (bytesRead == 0) {
            if (pos == 0) {
                HeapFree(GetProcessHeap(), 0, buffer);
                return NULL;
            }
            buffer[pos] = '\0';
            return buffer;
        }

        if (buffer[pos] == '\n') {
            buffer[pos] = '\0';
            return buffer;
        }

        pos++;
    }
}

//THE MOST POWERFUL PRINTF IN HISTORY
void demidov_printf(const char *format, ...) {
    va_list args;
    va_start(args, format);

    char buffer[1024]; // Buffer to hold the formatted string
    char* buf_ptr = buffer;
    const char* fmt_ptr = format;
    int buffer_size = sizeof(buffer);

    while (*fmt_ptr) {
        if (*fmt_ptr == '%') {
            fmt_ptr++;
            switch (*fmt_ptr) {
                case 'd': {
                    int value = va_arg(args, int);
                    char num_buffer[20]; // Buffer to hold the integer as a string
                    char* num_ptr = num_buffer;
                    if (value < 0) {
                        *buf_ptr++ = '-';
                        value = -value;
                    }
                    do {
                        *num_ptr++ = (char)((value % 10) + '0');
                        value /= 10;
                    } while (value > 0);
                    while (num_ptr > num_buffer) {
                        *buf_ptr++ = *--num_ptr;
                    }
                    break;
                }
                case 's': {
                    char* str = va_arg(args, char*);
                    while (*str) {
                        *buf_ptr++ = *str++;
                    }
                    break;
                }
                case 'c': {
                    char ch = (char)va_arg(args, int);
                    *buf_ptr++ = ch;
                    break;
                }
                case '%': {
                    *buf_ptr++ = '%';
                    break;
                }
                default:
                    *buf_ptr++ = *fmt_ptr;
                    break;
            }
        } else {
            *buf_ptr++ = *fmt_ptr;
        }
        fmt_ptr++;
    }
    *buf_ptr = '\0'; // Null-terminate the buffer

    va_end(args);

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD bytesWritten;
    WriteConsoleA(hConsole, buffer, (DWORD)(buf_ptr - buffer), &bytesWritten, NULL);
}

//Printf for files
int demidov_file_printf(HANDLE fileHandle, const char *format, ...) {
    va_list args;
    va_start(args, format);

    char buffer[1024]; // Buffer to hold the formatted string
    char* buf_ptr = buffer;
    const char* fmt_ptr = format;
    int buffer_size = sizeof(buffer);

    while (*fmt_ptr) {
        if (*fmt_ptr == '%') {
            fmt_ptr++;
            switch (*fmt_ptr) {
                case 'd': {
                    int value = va_arg(args, int);
                    char num_buffer[20]; // Buffer to hold the integer as a string
                    char* num_ptr = num_buffer;
                    if (value < 0) {
                        *buf_ptr++ = '-';
                        value = -value;
                    }
                    do {
                        *num_ptr++ = (char)((value % 10) + '0');
                        value /= 10;
                    } while (value > 0);
                    while (num_ptr > num_buffer) {
                        *buf_ptr++ = *--num_ptr;
                    }
                    break;
                }
                case 's': {
                    const char* str = va_arg(args, const char*);
                    while (*str) {
                        *buf_ptr++ = *str++;
                    }
                    break;
                }
                case 'c': {
                    char ch = (char)va_arg(args, int);
                    *buf_ptr++ = ch;
                    break;
                }
                case '%': {
                    *buf_ptr++ = '%';
                    break;
                }
                default:
                    *buf_ptr++ = *fmt_ptr;
                    break;
            }
        } else {
            *buf_ptr++ = *fmt_ptr;
        }
        fmt_ptr++;
    }
    *buf_ptr = '\0'; // Null-terminate the buffer

    // Open the file for writing
    DWORD bytesWritten;
    WriteFile(fileHandle, buffer, (DWORD)(buf_ptr - buffer), &bytesWritten, NULL);

    va_end(args);
    //CloseHandle(fileHandle);
}