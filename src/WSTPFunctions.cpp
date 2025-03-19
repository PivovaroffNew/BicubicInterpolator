#include <limits>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "BicubicInterpolator.h"
#include "wstp.h"
#define WSTP_RETURN_SUCCESS 0
#define WSTP_RETURN_ERROR 1

typedef std::function<double(double)> RealFuncOfOneVar;

// Map for storing interpolator instances
static std::map<int, std::unique_ptr<BicubicInterpolator>> interpolators;
static std::map<int, std::unique_ptr<FunctionNIntegratorBySimpson>>
    simpsonIntegrators;
static std::map<int, std::unique_ptr<ParametricCurveIntegrator>>
    curveIntegrators;

static int nextIntegratorHandle = 0;
static int interpolator_handle = 0;

std::vector<std::vector<double>> TransposeMatrix(
    const std::vector<std::vector<double>>& matrix) {
  if (matrix.empty() || matrix[0].empty()) return {};

  size_t rows = matrix.size();
  size_t cols = matrix[0].size();
  std::vector<std::vector<double>> transposed(cols, std::vector<double>(rows));

  for (size_t i = 0; i < rows; ++i) {
    for (size_t j = 0; j < cols; ++j) {
      transposed[j][i] = matrix[i][j];
    }
  }

  return transposed;
}

void AddZeroPad(std::vector<std::vector<double>>& matrix) {
  if (matrix.empty()) return;

  const size_t rows = matrix.size();
  const size_t cols = matrix[0].size();

  for (auto& row : matrix) {
    row.insert(row.begin(), 0.0);
    row.push_back(0.0);
  }

  std::vector<double> zero_row(cols + 2, 0.0);
  matrix.insert(matrix.begin(), zero_row);
  matrix.push_back(zero_row);
}

RealFuncOfOneVar ParseFunctionFromWSTP();
// ==================================================
// ОБЕРТКИ ДЛЯ BicubicInterpolator
// ==================================================

// Function to create a new interpolator
extern void WSTPCreateInterpolator(void) {
  int num_rows;
  const char* head;
  if (!WSGetFunction(stdlink, &head, &num_rows) ||
      std::strcmp(head, "List") != 0) {
    WSPutSymbol(stdlink, "$Failed");
  }

  std::vector<std::vector<double>> matrix;
  matrix.reserve(num_rows);

  // Read each row of the matrix
  for (size_t i = 0; i < num_rows; i++) {
    double* row_data = nullptr;
    long num_cols = 0;
    // WSGetRealList reads a list of real numbers from the current position
    if (!WSGetRealList(stdlink, &row_data, &num_cols)) {
      WSPutSymbol(stdlink, "$Failed");
    }

    std::vector<double> row(row_data, row_data + num_cols);
    matrix.push_back(row);
    WSReleaseRealList(stdlink, row_data, num_cols);
  }

  matrix = TransposeMatrix(matrix);
  AddZeroPad(matrix);

  try {
    // Create a new interpolator
    std::unique_ptr<BicubicInterpolator> interpolator =
        std::make_unique<BicubicInterpolator>(matrix);
    int handle = interpolator_handle++;
    interpolators[handle] = std::move(interpolator);

    WSNewPacket(stdlink);
    WSPutInteger(stdlink, handle);
  } catch (...) {
    WSNewPacket(stdlink);
    WSPutInteger(stdlink, -1);
  }
}

extern double WSTPInterpolatePoint(double x, double y, int handle) {
  double result = std::numeric_limits<double>::quiet_NaN();

  auto it = interpolators.find(handle);
  if (it != interpolators.end()) {
    try {
      result = it->second->interpolate(x, y);
    } catch (...) {
      result = std::numeric_limits<double>::quiet_NaN();
    }
  }

  return result;
}

// Function to delete an interpolator
extern void WSTPDeleteInterpolator(int handle) {
  // Если интерполятор с заданным идентификатором не найден, отправляем $Failed
  if (interpolators.find(handle) == interpolators.end()) {
    WSPutSymbol(stdlink, "DeleteInterpolator::notfound");
    WSNewPacket(stdlink);
    return;
  }

  // Удаляем интерполятор
  interpolators.erase(handle);

  // Возвращаем Null в качестве успешного результата
  WSNewPacket(stdlink);
  WSPutSymbol(stdlink, "Success");
}

// ==================================================
// ОБЕРТКИ ДЛЯ FunctionNIntegratorBySimpson
// ==================================================

extern void WSTPCreateSimpsonIntegrator() {
  // 1. Получить функцию и параметр n из WSTP
  int n;
  WSGetInteger(stdlink, &n);

  // 2. Получить функцию (требуется реализация парсера)
  // Предположим, функция передается как MathLink-выражение
  RealFuncOfOneVar func = ParseFunctionFromWSTP();  // How to do??

  try {
    auto integrator = std::make_unique<FunctionNIntegratorBySimpson>(func, n);
    int handle = nextIntegratorHandle++;
    simpsonIntegrators[handle] = std::move(integrator);

    WSPutInteger(stdlink, handle);
  } catch (...) {
    WSPutSymbol(stdlink, "$Failed");
  }
}

extern double WSTPIntegrateSimpson(int handle, double a, double b) {
  auto it = simpsonIntegrators.find(handle);
  if (it == simpsonIntegrators.end())
    return std::numeric_limits<double>::quiet_NaN();

  try {
    return it->second->integrate(a, b);
  } catch (...) {
    return std::numeric_limits<double>::quiet_NaN();
  }
}

// ==================================================
// ОБЕРТКИ ДЛЯ ParametricCurveIntegrator
// ==================================================

extern void WSTPCreateCurveIntegrator(int interpolatorHandle) {
  // 1. Проверить существование интерполятора
  if (interpolators.find(interpolatorHandle) == interpolators.end()) {
    WSPutSymbol(stdlink, "$Failed");
    return;
  }

  // 2. Получить xFunc и yFunc из WSTP
  auto xFunc = ParseFunctionFromWSTP();  // How to do??
  auto yFunc = ParseFunctionFromWSTP();  // How to do??

  try {
    auto integrator = std::make_unique<ParametricCurveIntegrator>(
        *interpolators[interpolatorHandle], xFunc, yFunc);

    int handle = nextIntegratorHandle++;
    curveIntegrators[handle] = std::move(integrator);
    WSPutInteger(stdlink, handle);
  } catch (...) {
    WSPutSymbol(stdlink, "$Failed");
  }
}

extern double WSTPIntegrateCurve(int handle, double t0, double t1, int n) {
  auto it = curveIntegrators.find(handle);
  if (it == curveIntegrators.end())
    return std::numeric_limits<double>::quiet_NaN();

  try {
    return it->second->integrate(t0, t1, n);
  } catch (...) {
    return std::numeric_limits<double>::quiet_NaN();
  }
}