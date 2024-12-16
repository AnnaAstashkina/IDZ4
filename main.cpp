#include <iostream>
#include <vector>
#include <pthread.h>
#include <random>
#include <cstdlib>
#include <ctime>
#include <unistd.h>

enum State { EMPTY, NOPROCESS, PROCESSED };

struct Garden {
  std::vector<std::vector<State>> grid; // Грядки
  pthread_mutex_t mutex;
  int n; 
  int m;
};

struct Gardener {
  int id;
  Garden* garden;
  double speed;  // Время обработки одного участка
  int startRow;  // Начальный ряд
  int startCol;  // Начальная колонка
  double time;  // Время прохода занятого или уже обработанного участка
};

void printGarden(const Garden& garden) { // Печать сада
  for (int i = 0; i < garden.n; ++i) {
    for (int j = 0; j < garden.m; ++j) {
      if (garden.grid[i][j] == EMPTY) {
        std::cout << ". ";
      } else if (garden.grid[i][j] == NOPROCESS) {
        std::cout << "X ";
      } else {
        std::cout << "P ";
      }
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

void initializeGarden(Garden& garden, int procent) {
  garden.grid.resize(garden.n, std::vector<State>(garden.m, EMPTY));
  int occupiedCells = (garden.n * garden.m * procent) / 100;  // Количество занятых для свопадения с процентом

  std::random_device rand_dev;
  std::mt19937 generator(rand_dev());
  std::uniform_int_distribution<> distrn(0, garden.n - 1);
  std::uniform_int_distribution<> distrm(0, garden.m - 1);

  for (int i = 0; i < occupiedCells; ++i) { // Генерация координат занятых
    int indexn = distrn(generator);
    int indexm = distrm(generator);
    if (garden.grid[indexn][indexm] == EMPTY) {
      garden.grid[indexn][indexm] = NOPROCESS;
    } else {
      --i;  // Повторяем, если клетка уже занята
    }
  }
}

void* gardenerWork(void* arg) { // Основная функция
  Gardener* gardener = (Gardener*)arg;
  int row = gardener->startRow;
  int col = gardener->startCol;

  while (row >= 0 && row < gardener->garden->n && col >= 0 && col < gardener->garden->m) {
    pthread_mutex_lock(&gardener->garden->mutex); // Запираем клетку

    if (gardener->garden->grid[row][col] == EMPTY) {
      // Обработка
      gardener->garden->grid[row][col] = PROCESSED;
      std::cout << "Gardener " << gardener->id << " processed State (" << row << ", " << col << ")" << std::endl;
      printGarden(*gardener->garden);  // Печать состояния сада после обработки
      sleep(gardener->speed);  // Имитация времени обработки
    } else {
      sleep(gardener->time);  // Имитация прохождения занятого поля
    }
    pthread_mutex_unlock(&gardener->garden->mutex); // Разблокируем

    // Переход к следующему участку
    if (gardener->id == 1) {  // Садовник идет вниз (первый)
      if (row % 2 == 0 && col < gardener->garden->m - 1) {
        col++;
      } else if (row % 2 == 1 && col > 0) {
        col--;
      } else if (row % 2 == 0) {
        row++;
      } else {
        row++;
      }
    } else {  // Садовник идет вверх (второй)
      if ((gardener->garden->m - 1 - col) % 2 == 0 && row > 0) {
        row--;
      } else if ((gardener->garden->m - 1 - col) % 2 == 1 && row < gardener->garden->n - 1) {
        row++;
      } else if ((gardener->garden->m - 1 - col) % 2 == 0) {
        col--;
      } else {
        col--;
      }
    }
  }
  return nullptr;
}

int main(int argc, char* argv[]) {
  if (argc != 6) {
    std::cout << ":(";
    return 1;
  }

  // Основные переменные
  int n = std::atoi(argv[1]);
  int m = std::atoi(argv[2]);
  double v1 = std::stod(argv[3]);
  double v2 = std::stod(argv[4]);
  double time = std::stod(argv[5]); // Константа прохождения участка

  std::random_device rand_dev;
  std::mt19937 generator(rand_dev());
  std::uniform_real_distribution<> distr(10, 30);
  double procent = distr(generator);  // Генерация процента занятых

  Garden garden; // Создание сада
  garden.n = n;
  garden.m = m;
  pthread_mutex_init(&garden.mutex, nullptr);

  initializeGarden(garden, procent);

  printGarden(garden);  // Печать начального состояния сада

  // Создание садовников
  Gardener gardener1 = {1, &garden, v1, 0, 0, time};           // Первый садовник (идет вниз)
  Gardener gardener2 = {2, &garden, v2, n - 1, m - 1, time};  // Второй садовник (идет вверх)

  pthread_t thread1, thread2;  // Создане потоков
  pthread_create(&thread1, nullptr, gardenerWork, &gardener1);
  pthread_create(&thread2, nullptr, gardenerWork, &gardener2);

  pthread_join(thread1, nullptr);
  pthread_join(thread2, nullptr);

  std::cout << "Final garden state:\n";
  printGarden(garden);  // Печать конечного состояния сада

  pthread_mutex_destroy(&garden.mutex);

  return 0;
}
