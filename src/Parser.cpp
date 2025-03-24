#include <cmath>
#include <functional>
#include <map>
#include <string>
#include <exception>

#include "wstp.h"

// Вспомогательная функция для парсинга аргумента

std::vector<std::string> WM_expression_heads_{"Plus"};

void Decompose(std::string str) {
    
}

double ParseExpression(WSLINK link) {
  const char* head;
  int len;

  if (!WSGetFunction(link, &head, &len)) {
    throw std::runtime_error("Failed to parse function");
  }

  // Обработка стандартных операций
  if (std::strcmp(head, "Plus") == 0) {
    double a = ParseExpression(link);
    double b = ParseExpression(link);
    return a + b;
  } else if (std::strcmp(head, "Times") == 0) {
    double a = ParseExpression(link);
    double b = ParseExpression(link);
    return a * b;
  } else if (std::strcmp(head, "Sin") == 0) {
    double arg = ParseExpression(link);
    return sin(arg);
  }

  // Обработка Slot[1] (аргумента функции #)
  else if (std::strcmp(head, "Slot") == 0) {
    int slot_num;
    WSGetInteger(link, &slot_num);
    return std::numeric_limits<double>::quiet_NaN();  // Маркер аргумента
  }

  // Обработка числовых констант
  else if (std::strcmp(head, "Real") == 0) {
    double value;
    WSGetReal(link, &value);
    return value;
  }

  throw std::runtime_error("Unsupported operation: " + std::string(head));
}

// Основная функция парсинга
std::function<double(double)> ParseFunctionFromWSTP() {
  // 1. Получить выражение Function[...]
  const char* head;
  int len;
  WSGetFunction(stdlink, &head, &len);

  if (std::strcmp(head, "Function") != 0) {
    throw std::runtime_error("Expected Function expression");
  }

  // 2. Парсим тело функции
  double result = ParseExpression(stdlink);

  // 3. Собираем лямбда-функцию
  return [](double x) {
    // Здесь должна быть логика подстановки x вместо Slot[1]
    // В реальной реализации нужно отслеживать позиции Slot
    // Это упрощенный пример!
    return x;  // Заглушка
  };
}