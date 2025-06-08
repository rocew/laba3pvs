#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <locale.h>
#include <mpi.h>

// Bubble sort
void bubble_sort(int* arr, size_t size) {
    for (size_t i = 0; i < size - 1; ++i) {
        for (size_t j = 0; j < size - i - 1; ++j) {
            if (arr[j] > arr[j + 1]) {
                int tmp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = tmp;
            }
        }
    }
}

// Merge two sorted arrays
void merge(int* local_data, int local_size, int* partner_data, int partner_size) {
    int* merged = malloc((local_size + partner_size) * sizeof(int));
    int i = 0, j = 0, k = 0;

    while (i < local_size && j < partner_size)
        merged[k++] = (local_data[i] < partner_data[j]) ? local_data[i++] : partner_data[j++];
    while (i < local_size) merged[k++] = local_data[i++];
    while (j < partner_size) merged[k++] = partner_data[j++];

    for (i = 0; i < local_size; i++) local_data[i] = merged[i];
    free(merged);
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    size_t global_n = 0;
    int runs = 1;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    setlocale(LC_ALL, "");

    if (rank == 0) {
        printf("Enter the number of elements: ");
        scanf("%zu", &global_n);
        printf("Enter the number of runs: ");
        scanf("%d", &runs);
    }

    MPI_Bcast(&global_n, 1, MPI_UNSIGNED_LONG, 0, MPI_COMM_WORLD);
    MPI_Bcast(&runs, 1, MPI_INT, 0, MPI_COMM_WORLD);

    size_t local_n = global_n / size;
    if (rank == size - 1)
        local_n += global_n % size;

    double total_fill = 0.0, total_sort = 0.0, total_all = 0.0;

    for (int r = 0; r < runs; r++) {
        int* local_array = malloc(local_n * sizeof(int));
        int* full_array = NULL;
        int* sendcounts = NULL;
        int* displs = NULL;

        double fill_start = 0.0, fill_end = 0.0;
        double sort_start = 0.0, sort_end = 0.0;
        double total_start = 0.0, total_end = 0.0;

        if (rank == 0) {
            full_array = malloc(global_n * sizeof(int));
            sendcounts = malloc(size * sizeof(int));
            displs = malloc(size * sizeof(int));

            fill_start = MPI_Wtime();

            // Fill with random numbers (0 to 99)
            srand((unsigned int)(time(NULL) + r));
            for (size_t i = 0; i < global_n; i++) {
                full_array[i] = rand() % 100;
            }

            // Calculate send counts and displacements
            size_t base = global_n / size, offset = 0;
            for (int i = 0; i < size; i++) {
                sendcounts[i] = base + (i == size - 1 ? global_n % size : 0);
                displs[i] = offset;
                offset += sendcounts[i];
            }

            fill_end = MPI_Wtime();
        }

        MPI_Barrier(MPI_COMM_WORLD);
        total_start = MPI_Wtime();

        MPI_Scatterv(full_array, sendcounts, displs, MPI_INT,
            local_array, (int)local_n, MPI_INT,
            0, MPI_COMM_WORLD);

        sort_start = MPI_Wtime();
        bubble_sort(local_array, local_n);

        for (int step = 0; step < size; step++) {
            int partner = (step % 2 == 0)
                ? (rank % 2 == 0 ? rank + 1 : rank - 1)
                : (rank % 2 == 0 ? rank - 1 : rank + 1);

            if (partner >= 0 && partner < size) {
                MPI_Status status;
                int recv_size;

                MPI_Sendrecv(&local_n, 1, MPI_INT, partner, 0,
                    &recv_size, 1, MPI_INT, partner, 0,
                    MPI_COMM_WORLD, &status);

                int* partner_data = malloc(recv_size * sizeof(int));
                MPI_Sendrecv(local_array, (int)local_n, MPI_INT, partner, 1,
                    partner_data, recv_size, MPI_INT, partner, 1,
                    MPI_COMM_WORLD, &status);

                merge(local_array, (int)local_n, partner_data, recv_size);
                free(partner_data);
            }
            MPI_Barrier(MPI_COMM_WORLD);
        }

        sort_end = MPI_Wtime();
        MPI_Barrier(MPI_COMM_WORLD);
        total_end = MPI_Wtime();

        double fill_time = 0.0;
        if (rank == 0) fill_time = fill_end - fill_start;

        double sort_time = sort_end - sort_start;
        double total_time = total_end - total_start;

        double max_fill, max_sort, max_total;
        MPI_Reduce(&fill_time, &max_fill, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
        MPI_Reduce(&sort_time, &max_sort, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
        MPI_Reduce(&total_time, &max_total, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

        if (rank == 0) {
            printf("\n\033[1mRun %d\033[0m\n", r + 1);
            printf("Fill time:     %.6f sec\n", max_fill);
            printf("Sort time:     %.6f sec\n", max_sort);
            printf("Total time:    %.6f sec\n", max_total);
            total_fill += max_fill;
            total_sort += max_sort;
            total_all += max_total;
        }

        free(local_array);
        if (rank == 0) {
            free(full_array);
            free(sendcounts);
            free(displs);
        }
    }

    if (rank == 0) {
        printf("\n\033[1m=== AVERAGE VALUES (%d runs) ===\033[0m\n", runs);
        printf("Average fill time:     %.6f sec\n", total_fill / runs);
        printf("Average sort time:     %.6f sec\n", total_sort / runs);
        printf("Average total time:    %.6f sec\n", total_all / runs);
    }

    MPI_Finalize();
    return 0;
}
