#docker build -f Dockerfile --platform=linux/arm64 -t mpich-5 .

# Используем базовый образ Ubuntu с поддержкой MPICH
FROM ubuntu:22.04

ENV F_NAME=lab5.c
ENV N=10
# Устанавливаем необходимые зависимости
RUN apt-get update && apt-get install -y \
    build-essential \
    mpich \
    && apt-get clean

# Устанавливаем рабочую директорию
WORKDIR /app

# Копируем исходный код
COPY ${F_NAME} .


# Указываем команду для запуска программы через mpiexec
CMD ["sh", "-c", "mpicc $F_NAME -o main && mpiexec -n $N ./main"]
