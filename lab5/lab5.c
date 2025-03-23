#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <limits.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

// Генерация случайного ацикличного графа
void createRandomGraph(int *graphData, int *taskTimes, bool *isTerminal, int numTasks, int density)
{
    srand(time(NULL));
    int **graph = malloc(numTasks * sizeof(int *));
    for (int i = 0; i < numTasks; i++)
    {
        graph[i] = calloc(numTasks, sizeof(int));
    }

    for (int i = 0; i < numTasks; i++)
    {
        for (int j = i + 1; j < numTasks; j++)
        {
            if (rand() % 100 < density)
            {
                graph[i][j] = rand() % 10 + 1; // Вес ребра
            }
        }
    }

    for (int i = 0; i < numTasks; i++)
    {
        taskTimes[i] = rand() % 5 + 1; // Время выполнения задачи
    }

    for (int i = 0; i < numTasks; i++)
    {
        bool hasOutgoing = false;
        for (int j = 0; j < numTasks; j++)
        {
            if (graph[i][j] > 0)
            {
                hasOutgoing = true;
                break;
            }
        }
        isTerminal[i] = !hasOutgoing; // Отметить финальные задачи
    }

    for (int i = 0; i < numTasks; i++)
    {
        for (int j = 0; j < numTasks; j++)
        {
            graphData[i * numTasks + j] = graph[i][j];
        }
        free(graph[i]);
    }
    free(graph);
}

// Распределение задач между процессами
void splitTasks(int numTasks, int numProcs, int *sendSizes, int *offsets, int *rowsPerProc, int *rowStarts)
{
    int base = numTasks / numProcs, extra = numTasks % numProcs, currentOffset = 0, currentRowStart = 0;
    for (int i = 0; i < numProcs; i++)
    {
        int rows = base + (i < extra ? 1 : 0);
        sendSizes[i] = rows * numTasks;
        offsets[i] = currentOffset;
        rowsPerProc[i] = rows;
        rowStarts[i] = currentRowStart;
        currentOffset += sendSizes[i];
        currentRowStart += rows;
    }
}

// Вычисление ранних сроков завершения (EFT)
void calculateEFT(int *localGraph, int *taskTimes, int *localEFT, int *globalEFT, int *recvSizes, int *recvOffsets, int localRows, int globalRowStart, int numTasks, MPI_Comm comm)
{
    bool hasUpdates;
    do
    {
        hasUpdates = false;
        MPI_Allgatherv(localEFT, localRows, MPI_INT, globalEFT, recvSizes, recvOffsets, MPI_INT, comm);

        for (int i = 0; i < localRows; i++)
        {
            int globalTask = globalRowStart + i;
            int maxPredTime = 0;
            for (int j = 0; j < numTasks; j++)
            {
                int edgeWeight = localGraph[i * numTasks + j];
                if (edgeWeight > 0)
                {
                    maxPredTime = MAX(maxPredTime, globalEFT[j] + edgeWeight);
                }
            }
            int newEFT = maxPredTime + taskTimes[globalTask];
            if (newEFT > localEFT[i])
            {
                localEFT[i] = newEFT;
                hasUpdates = true;
            }
        }

        bool globalUpdates;
        MPI_Allreduce(&hasUpdates, &globalUpdates, 1, MPI_C_BOOL, MPI_LOR, comm);
        hasUpdates = globalUpdates;
    } while (hasUpdates);
}

// Вычисление поздних сроков завершения (LFT)
void calculateLFT(int *localGraph, int *taskTimes, int *localLFT, int *globalLFT, int *recvSizes, int *recvOffsets, int localRows, int globalRowStart, int numTasks, MPI_Comm comm)
{
    bool hasUpdates;
    do
    {
        hasUpdates = false;
        MPI_Allgatherv(localLFT, localRows, MPI_INT, globalLFT, recvSizes, recvOffsets, MPI_INT, comm);

        for (int i = 0; i < localRows; i++)
        {
            int globalTask = globalRowStart + i;
            int minSuccTime = INT_MAX;
            bool hasSuccessors = false;
            for (int j = 0; j < numTasks; j++)
            {
                int edgeWeight = localGraph[i * numTasks + j];
                if (edgeWeight > 0)
                {
                    int candidate = globalLFT[j] - edgeWeight - taskTimes[globalTask];
                    minSuccTime = MIN(minSuccTime, candidate);
                    hasSuccessors = true;
                }
            }
            if (hasSuccessors && minSuccTime < localLFT[i])
            {
                localLFT[i] = minSuccTime;
                hasUpdates = true;
            }
        }

        bool globalUpdates;
        MPI_Allreduce(&hasUpdates, &globalUpdates, 1, MPI_C_BOOL, MPI_LOR, comm);
        hasUpdates = globalUpdates;
    } while (hasUpdates);
}

// Вывод результатов
void printLSTResults(int *lst, int numTasks)
{
    printf("\nLFT for all tasks:\n");
    for (int i = 0; i < numTasks; i++)
    {
        printf("Task %d, val: %d\n", i, lst[i]);
    }
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int rank, numProcs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);

    int numTasks = 1000; // Количество задач
    int density = 10;    // Плотность графа

    int *taskTimes = malloc(numTasks * sizeof(int));
    bool *isTerminal = malloc(numTasks * sizeof(bool));
    int *graphData = malloc(numTasks * numTasks * sizeof(int));

    if (rank == 0)
    {
        createRandomGraph(graphData, taskTimes, isTerminal, numTasks, density);
    }

    MPI_Bcast(taskTimes, numTasks, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(isTerminal, numTasks, MPI_C_BOOL, 0, MPI_COMM_WORLD);

    int *sendSizes = malloc(numProcs * sizeof(int));
    int *offsets = malloc(numProcs * sizeof(int));
    int *rowsPerProc = malloc(numProcs * sizeof(int));
    int *rowStarts = malloc(numProcs * sizeof(int));

    splitTasks(numTasks, numProcs, sendSizes, offsets, rowsPerProc, rowStarts);

    int localRows = rowsPerProc[rank];
    int *localGraph = malloc(localRows * numTasks * sizeof(int));
    MPI_Scatterv(graphData, sendSizes, offsets, MPI_INT, localGraph, localRows * numTasks, MPI_INT, 0, MPI_COMM_WORLD);

    int globalRowStart = rowStarts[rank];

    double startTime = 0, endTime;
    if (rank == 0)
    {
        startTime = MPI_Wtime();
    }

    int *localEFT = calloc(localRows, sizeof(int));
    int *globalEFT = malloc(numTasks * sizeof(int));
    calculateEFT(localGraph, taskTimes, localEFT, globalEFT, rowsPerProc, rowStarts, localRows, globalRowStart, numTasks, MPI_COMM_WORLD);

    int *localLFT = malloc(localRows * sizeof(int));
    for (int i = 0; i < localRows; i++)
    {
        localLFT[i] = INT_MAX;
    }
    for (int i = 0; i < localRows; i++)
    {
        int globalTask = globalRowStart + i;
        if (isTerminal[globalTask])
        {
            int maxEFT = 0;
            for (int j = 0; j < numTasks; j++)
            {
                maxEFT = MAX(maxEFT, globalEFT[j]);
            }
            localLFT[i] = maxEFT;
        }
    }

    int *globalLFT = malloc(numTasks * sizeof(int));
    calculateLFT(localGraph, taskTimes, localLFT, globalLFT, rowsPerProc, rowStarts, localRows, globalRowStart, numTasks, MPI_COMM_WORLD);

    int *finalLFT = malloc(numTasks * sizeof(int));
    MPI_Allgatherv(localLFT, localRows, MPI_INT, finalLFT, rowsPerProc, rowStarts, MPI_INT, MPI_COMM_WORLD);

    int *finalLST = malloc(numTasks * sizeof(int));
    for (int i = 0; i < numTasks; i++)
    {
        finalLST[i] = finalLFT[i] - taskTimes[i];
    }

    if (rank == 0)
    {
        printLSTResults(finalLST, numTasks);
        endTime = MPI_Wtime();
        printf("\nTime spent: %.2f ms\n", (endTime - startTime) * 1000);
    }

    free(taskTimes);
    free(isTerminal);
    free(graphData);
    free(sendSizes);
    free(offsets);
    free(rowsPerProc);
    free(rowStarts);
    free(localGraph);
    free(localEFT);
    free(globalEFT);
    free(localLFT);
    free(globalLFT);
    free(finalLFT);
    free(finalLST);

    MPI_Finalize();
    return 0;
}