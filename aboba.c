#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <time.h>

#define PROGRESS_BAR_WIDTH 50

// Progress bar function
void print_progress(size_t current, size_t total) {
    float progress = (float)current / total;
    int filled = (int)(progress * PROGRESS_BAR_WIDTH);
    printf("\r[");
    for (int i = 0; i < PROGRESS_BAR_WIDTH; i++) {
        if (i < filled) printf("#");
        else printf("-");
    }
    printf("] %3.0f%%", progress * 100);
    fflush(stdout);
}

// Bubble sort with progress bar
void bubble_sort(int* array, size_t size) {
    for (size_t i = 0; i < size - 1; i++) {
        int swapped = 0;
        for (size_t j = 0; j < size - i - 1; j++) {
            if (array[j] > array[j + 1]) {
                int temp = array[j];
                array[j] = array[j + 1];
                array[j + 1] = temp;
                swapped = 1;
            }
        }
        if (!swapped) break; // если не было обменов — выходим
    }
}


int main() {
    setlocale(LC_ALL, "");

    size_t SIZE;
    int NUM_RUNS;

    printf("Введите количество элементов в массиве: ");
    scanf("%zu", &SIZE);

    if (SIZE <= 0) {
        printf("Ошибка: количество элементов должно быть положительным!\n");
        return 1;
    }

    printf("Введите количество запусков: ");
    scanf("%d", &NUM_RUNS);

    if (NUM_RUNS <= 0) {
        printf("Ошибка: количество запусков должно быть положительным!\n");
        return 1;
    }

    double total_fill_time = 0.0;
    double total_sort_time = 0.0;
    double total_total_time = 0.0;

    for (int run = 0; run < NUM_RUNS; run++) {
        clock_t total_start = clock();

        clock_t fill_start = clock();
        int* array = (int*)malloc(SIZE * sizeof(int));
        if (array == NULL) {
            printf("Ошибка: не удалось выделить память!\n");
            return 1;
        }

        srand((unsigned int)(time(NULL) + run));
        for (size_t i = 0; i < SIZE; i++) {
            array[i] = rand() % 100;
        }
        clock_t fill_end = clock();

        clock_t sort_start = clock();
        printf("Сортировка...\n");
        bubble_sort(array, SIZE);
        clock_t sort_end = clock();

        clock_t total_end = clock();

        double fill_time = (double)(fill_end - fill_start) / CLOCKS_PER_SEC;
        double sort_time = (double)(sort_end - sort_start) / CLOCKS_PER_SEC;
        double total_time = (double)(total_end - total_start) / CLOCKS_PER_SEC;

        total_fill_time += fill_time;
        total_sort_time += sort_time;
        total_total_time += total_time;

        free(array);

        printf("\033[1mЗапуск %d\033[0m\n", run + 1);
        printf("Время заполнения: %.6f сек\n", fill_time);
        printf("Время сортировки: %.6f сек\n", sort_time);
        printf("Общее время: %.6f сек\n\n", total_time);
    }

    printf("\033[1m=== СРЕДНИЕ ЗНАЧЕНИЯ (из %d запусков) ===\033[0m\n", NUM_RUNS);
    printf("Среднее время заполнения: %.6f сек\n", total_fill_time / NUM_RUNS);
    printf("Среднее время сортировки: %.6f сек\n", total_sort_time / NUM_RUNS);
    printf("Среднее общее время: %.6f сек\n", total_total_time / NUM_RUNS);

    return 0;
}
