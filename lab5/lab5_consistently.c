#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

#define OPERATIONS 1000 // Количество операций (вершин)
#define DENSITY 10      // Плотность графа (вероятность создания ребра в процентах)

// Структура для хранения данных о графе
typedef struct
{
    int graph[OPERATIONS][OPERATIONS]; // Матрица смежности
    int durations[OPERATIONS];         // Длительности операций
    int isFinal[OPERATIONS];           // Флаги финальных операций
} GraphData;

// Функции
void createRandomGraph(GraphData *data);
void calculateEarlyTimes(const GraphData *data, int earlyTimes[OPERATIONS]);
void calculateLateTimes(const GraphData *data, int lateTimes[OPERATIONS], int maxEarlyTime);
void displayResults(const int lateStartTimes[OPERATIONS], double executionTime);

int main()
{
    GraphData data = {0};             // Инициализация структуры нулями
    int earlyTimes[OPERATIONS] = {0}; // Ранние сроки завершения
    int lateTimes[OPERATIONS];        // Поздние сроки завершения
    int lateStartTimes[OPERATIONS];   // Поздние сроки начала

    srand(time(NULL)); // Инициализация генератора случайных чисел

    createRandomGraph(&data); // Генерация графа

    clock_t start = clock();

    // Вычисление ранних сроков завершения
    calculateEarlyTimes(&data, earlyTimes);

    // Нахождение максимального раннего срока завершения
    int maxEarlyTime = 0;
    for (int i = 0; i < OPERATIONS; i++)
    {
        if (earlyTimes[i] > maxEarlyTime)
        {
            maxEarlyTime = earlyTimes[i];
        }
    }

    // Инициализация поздних сроков завершения
    for (int i = 0; i < OPERATIONS; i++)
    {
        lateTimes[i] = INT_MAX;
        if (data.isFinal[i])
        {
            lateTimes[i] = maxEarlyTime;
        }
    }

    // Вычисление поздних сроков завершения
    calculateLateTimes(&data, lateTimes, maxEarlyTime);

    // Вычисление поздних сроков начала
    for (int i = 0; i < OPERATIONS; i++)
    {
        lateStartTimes[i] = lateTimes[i] - data.durations[i];
    }

    clock_t end = clock();
    double executionTime = (double)(end - start) / CLOCKS_PER_SEC * 1000.0;

    // Вывод результатов
    displayResults(lateStartTimes, executionTime);

    return 0;
}

// Генерация случайного ацикличного графа
void createRandomGraph(GraphData *data)
{
    for (int i = 0; i < OPERATIONS; i++)
    {
        for (int j = i + 1; j < OPERATIONS; j++)
        {
            if (rand() % 100 < DENSITY)
            {
                data->graph[i][j] = rand() % 10 + 1; // Случайный вес ребра
            }
        }
        data->durations[i] = rand() % 5 + 1; // Случайная длительность операции
    }

    // Определение финальных операций (без исходящих рёбер)
    for (int i = 0; i < OPERATIONS; i++)
    {
        int hasOutgoing = 0;
        for (int j = 0; j < OPERATIONS; j++)
        {
            if (data->graph[i][j] > 0)
            {
                hasOutgoing = 1;
                break;
            }
        }
        data->isFinal[i] = !hasOutgoing;
    }
}

// Вычисление ранних сроков завершения (EFT)
void calculateEarlyTimes(const GraphData *data, int earlyTimes[OPERATIONS])
{
    int changed;
    do
    {
        changed = 0;
        for (int i = 0; i < OPERATIONS; i++)
        {
            int maxPredecessor = 0;
            for (int j = 0; j < OPERATIONS; j++)
            {
                if (data->graph[j][i] > 0)
                {
                    int candidate = earlyTimes[j] + data->graph[j][i];
                    if (candidate > maxPredecessor)
                    {
                        maxPredecessor = candidate;
                    }
                }
            }
            int newEarlyTime = maxPredecessor + data->durations[i];
            if (newEarlyTime > earlyTimes[i])
            {
                earlyTimes[i] = newEarlyTime;
                changed = 1;
            }
        }
    } while (changed);
}

// Вычисление поздних сроков завершения (LFT)
void calculateLateTimes(const GraphData *data, int lateTimes[OPERATIONS], int maxEarlyTime)
{
    int changed;
    do
    {
        changed = 0;
        for (int i = OPERATIONS - 1; i >= 0; i--)
        {
            int minSuccessor = INT_MAX;
            int hasSuccessor = 0;
            for (int j = 0; j < OPERATIONS; j++)
            {
                if (data->graph[i][j] > 0)
                {
                    int candidate = lateTimes[j] - data->graph[i][j] - data->durations[i];
                    if (candidate < minSuccessor)
                    {
                        minSuccessor = candidate;
                    }
                    hasSuccessor = 1;
                }
            }
            if (hasSuccessor && minSuccessor < lateTimes[i])
            {
                lateTimes[i] = minSuccessor;
                changed = 1;
            }
        }
    } while (changed);
}

// Вывод результатов
void displayResults(const int lateStartTimes[OPERATIONS], double executionTime)
{
    printf("\nLFT for all tasks:\n");
    for (int i = 0; i < OPERATIONS; i++)
    {
        printf("Task %d, val: %d\n", i, lateStartTimes[i]);
    }
    printf("\nTime spent: %.2f ms\n", executionTime);
}