#include "demidovStdio.h"

typedef struct {
    int* array;
    int start;
    int end;
} ThreadData;

HANDLE semaphore;

void merge(int* array, int start, int mid, int end) {
    int i, j, k;
    int n1 = mid - start + 1;
    int n2 = end - mid;

    int* L = (int*)malloc(n1 * sizeof(int));
    int* R = (int*)malloc(n2 * sizeof(int));

    for (i = 0; i < n1; i++)
        L[i] = array[start + i];
    for (j = 0; j < n2; j++)
        R[j] = array[mid + 1 + j];

    i = 0;
    j = 0;
    k = start;
    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            array[k] = L[i];
            i++;
        } else {
            array[k] = R[j];
            j++;
        }
        k++;
    }

    while (i < n1) {
        array[k] = L[i];
        i++;
        k++;
    }

    while (j < n2) {
        array[k] = R[j];
        j++;
        k++;
    }

    free(L);
    free(R);
}

void mergeSort(int* array, int start, int end) {
    if (start < end) {
        int mid = start + (end - start) / 2;
        mergeSort(array, start, mid);
        mergeSort(array, mid + 1, end);
        merge(array, start, mid, end);
    }
}

DWORD WINAPI mergeSortThread(LPVOID lpParam) {
    ThreadData* data = (ThreadData*)lpParam;
    mergeSort(data->array, data->start, data->end);

    ReleaseSemaphore(semaphore, 1, NULL);
    return SUCCESS;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        demidov_printf("Usage: %s <array_size> <max_threads>\n", argv[0]);
        return ERROR_ARGS_COUNT;
    }

    int arraySize = atoi(argv[1]);
    int maxThreads = atoi(argv[2]);

    if (arraySize == 0) {
        demidov_printf("The array is empty. Cannot sort a void\n");
        return ERROR_ZERO_LENGTH_ARRAY;
    }

    if (maxThreads == 0) {
        demidov_printf("There are no threads. Cannot do anything\n");
        return ERROR_ZERO_THREADS;
    }

    if (maxThreads > MAX_THREADS || maxThreads > arraySize) {
        demidov_printf("\nMax threads limit exceeded. Setting to %d\n\n", arraySize);
        maxThreads = arraySize;
    }

    int* array = (int*)malloc(arraySize * sizeof(int));
    if (!array) {
        demidov_printf("Memory allocation error occured\n");
        return ERROR_MALLOC;
    }

    srand(time(NULL));
    for (int i = 0; i < arraySize; i++) {
        array[i] = rand() % 1000;
    }

    for (int i = 0; i < arraySize; i++) {
        demidov_printf("%d ", array[i]);
    }
    demidov_printf("\n\n");

    semaphore = CreateSemaphore(NULL, maxThreads, maxThreads, NULL);
    if (semaphore == NULL) {
        demidov_printf("CreateSemaphore error occured\n");
        return ERROR_CREATE_SEMAPHORE;
    }

    HANDLE* threads = (HANDLE*)malloc(maxThreads * sizeof(HANDLE));
    if (!threads) {
        demidov_printf("Failed to malloc for threads\n");
        return ERROR_MALLOC;
    }

    ThreadData* threadData = (ThreadData*)malloc(maxThreads * sizeof(ThreadData));
    if (!threadData) {
        demidov_printf("Failed to malloc for threadData\n");
        return ERROR_MALLOC;
    }

    clock_t start = clock();

    int chunkSize = arraySize / maxThreads;
    for (int i = 0; i < maxThreads; i++) {
        threadData[i].array = array;
        threadData[i].start = i * chunkSize;
        threadData[i].end = (i == maxThreads - 1) ? arraySize - 1 : (i + 1) * chunkSize - 1;

        WaitForSingleObject(semaphore, INFINITE);

        threads[i] = CreateThread(NULL, 0, mergeSortThread, &threadData[i], 0, NULL);
        if (threads[i] == NULL) {
            demidov_printf("CreateThread error occured\n");
            return ERROR_CREATE_THREAD;
        }
    }

    WaitForMultipleObjects(maxThreads, threads, TRUE, INFINITE);

    for (int i = 1; i < maxThreads; i++) {
        merge(array, 0, i * chunkSize - 1, (i + 1) * chunkSize - 1);
    }

    clock_t end = clock();
    float seconds = (float)(end - start) / CLOCKS_PER_SEC;
    demidov_printf("Time: %f src\n\n", seconds);

    for (int i = 0; i < arraySize - 1; i++) {
        if (array[i] > array[i + 1]) {
            demidov_printf("\nSorting failed at index %d\n\n", i);
            break;
        }
    }

    for (int i = 0; i < arraySize; i++) {
        demidov_printf("%d ", array[i]);
    }
    demidov_printf("\n");

    CloseHandle(semaphore);
    for (int i = 0; i < maxThreads; i++) {
        CloseHandle(threads[i]);
    }

    free(threads);
    free(threadData);
    free(array);

    demidov_printf("Sorting completed successfully\n");
    
    return 0;
}