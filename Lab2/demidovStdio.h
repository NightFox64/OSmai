#define INITIAL_BUFFER_SIZE 128
#define MAX_THREADS 100000

#include <windows.h>
#include <stdlib.h>
#include <time.h>

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
    ERROR_MALLOC,       //Failed to malloc
    ERROR_CREATE_SEMAPHORE, //Failed to create semaphore
    ERROR_ZERO_LENGTH_ARRAY,    //Array is empty
    ERROR_ZERO_THREADS,     //Zero threads, cannot do anything
    ERROR_CREATE_THREAD,    //Failed to create a thread
};

//THE MOST POWERFUL PRINTF IN HISTORY
void demidov_printf(const char *format, ...) {
    va_list args;
    va_start(args, format);

    char buffer[1024];
    char* buf_ptr = buffer;
    const char* fmt_ptr = format;
    int buffer_size = sizeof(buffer);

    while (*fmt_ptr) {
        if (*fmt_ptr == '%') {
            fmt_ptr++;
            switch (*fmt_ptr) {
                case 'd': {
                    int value = va_arg(args, int);
                    char num_buffer[20];
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
    *buf_ptr = '\0';

    va_end(args);

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD bytesWritten;
    WriteConsoleA(hConsole, buffer, (DWORD)(buf_ptr - buffer), &bytesWritten, NULL);
}

//Printf for files
int demidov_file_printf(HANDLE fileHandle, const char *format, ...) {
    va_list args;
    va_start(args, format);

    char buffer[1024];
    char* buf_ptr = buffer;
    const char* fmt_ptr = format;
    int buffer_size = sizeof(buffer);

    while (*fmt_ptr) {
        if (*fmt_ptr == '%') {
            fmt_ptr++;
            switch (*fmt_ptr) {
                case 'd': {
                    int value = va_arg(args, int);
                    char num_buffer[20];
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
    *buf_ptr = '\0';

    DWORD bytesWritten;
    WriteFile(fileHandle, buffer, (DWORD)(buf_ptr - buffer), &bytesWritten, NULL);

    va_end(args);
}