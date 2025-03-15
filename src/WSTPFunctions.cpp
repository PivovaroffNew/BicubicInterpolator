#include <limits>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "BicubicInterpolator.h"
#include "wstp.h"
#define WSTP_RETURN_SUCCESS 0
#define WSTP_RETURN_ERROR 1
// Map for storing interpolator instances
static std::map<int, std::unique_ptr<BicubicInterpolator>> interpolators;
static int nextHandle = 0;

std::vector<std::vector<double>> transposeMatrix(
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

  matrix = transposeMatrix(matrix);

  try {
    // Create a new interpolator
    std::unique_ptr<BicubicInterpolator> interpolator =
        std::make_unique<BicubicInterpolator>(matrix);
    int handle = nextHandle++;
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