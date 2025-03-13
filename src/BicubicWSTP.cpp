#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "BicubicInterpolator.h"
#include "wstp.h"

// Глобальное хранилище интерполяторов
std::map<int, BicubicInterpolator*> interpolators;
int next_id = 0;

// Обработчик ошибок
void sendError(WSLINK link, const std::string& msg) {
  WSPutFunction(link, "EvaluatePacket", 1);
  WSPutFunction(link, "Message", 2);
  WSPutFunction(link, "BicubicInterpolator", 0);
  WSPutString(link, msg.c_str());
  WSEndPacket(link);
  WSFlush(link);
}

// Главная функция обработки вызовов
int processPacket(WSLINK link) {
  const char* func;
  int argc;

  if (!WSGetFunction(link, &func, &argc)) return 0;
  std::string command(func);

  try {
    if (command == "CreateInterpolator") {
      // Получаем матрицу данных
      double* data;
      int* dims;
      char** heads;
      int d;
      if (!WSGetReal64Array(link, &data, &dims, &heads, &d) || d != 2) {
        sendError(link, "Invalid data format: expected 2D real array");
        return 0;
      }

      // Конвертируем в vector<vector<double>>
      std::vector<std::vector<double>> matrix(dims[0],
                                              std::vector<double>(dims[1]));
      for (int i = 0; i < dims[0]; i++) {
        for (int j = 0; j < dims[1]; j++) {
          matrix[i][j] = data[i * dims[1] + j];
        }
      }

      // Создаем интерполятор
      int id = next_id++;
      interpolators[id] = new BicubicInterpolator(matrix);

      // Отправляем ответ
      WSPutFunction(link, "ReturnPacket", 1);
      WSPutInteger32(link, id);
      WSReleaseReal64Array(link, data, dims, heads, d);
      return 1;
    } else if (command == "Interpolate") {
      // Получаем аргументы
      int id;
      double x, y;
      WSGetInteger32(link, &id);
      WSGetReal64(link, &x);
      WSGetReal64(link, &y);

      // Поиск интерполятора
      auto it = interpolators.find(id);
      if (it == interpolators.end()) {
        sendError(link, "Invalid interpolator ID");
        return 0;
      }

      // Вычисление
      double result = it->second->interpolate(x, y);

      // Отправка результата
      WSPutFunction(link, "ReturnPacket", 1);
      WSPutReal64(link, result);
      return 1;
    } else if (command == "DeleteInterpolator") {
      int id;
      WSGetInteger32(link, &id);
      auto it = interpolators.find(id);
      if (it != interpolators.end()) {
        delete it->second;
        interpolators.erase(it);
      }
      WSPutFunction(link, "ReturnPacket", 1);
      WSPutSymbol(link, "Null");
      return 1;
    }
  } catch (const std::exception& e) {
    sendError(link, e.what());
  }
  return 0;
}

// extern "C" int WSMain(int, char**[]);  // Явное объявление
// // Точка входа WSTP
// #if WINDOWS_WSTP
#include <Windows.h>
int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                      LPSTR lpCmdLine, int nCmdShow) {
  int argc = 0;
  LPWSTR* argvw = CommandLineToArgvW(GetCommandLineW(), &argc);
  char** argv = new char*[argc];

  // Конвертация аргументов в UTF-8
  for (int i = 0; i < argc; ++i) {
    int size =
        WideCharToMultiByte(CP_UTF8, 0, argvw[i], -1, NULL, 0, NULL, NULL);
    argv[i] = new char[size];
    WideCharToMultiByte(CP_UTF8, 0, argvw[i], -1, argv[i], size, NULL, NULL);
  }

  LocalFree(argvw);
  int result = WSMain(argc, argv);

  // Очистка памяти
  for (int i = 0; i < argc; ++i) delete[] argv[i];
  delete[] argv;

  return result;
}
// #else
// int main(int argc, char* argv[]) { return WSMain(argc, argv); }
// #endif