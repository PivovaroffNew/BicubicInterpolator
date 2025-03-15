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

extern "C" DLLEXPORT double WSTPInterpolatePoint(double x, double y, int handle) {
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

// // Function to interpolate at a point
// extern int WSTPInterpolatePoint(int link) {
//   // int handle;
//   // double x, y;

//   // if (!WSGetInteger(link, &handle) || !WSGetReal(link, &x) ||
//   // !WSGetReal(link, &y)) {
//   //     WSNewPacket(link);
//   //     WSPutReal(link, std::numeric_limits<double>::quiet_NaN());
//   //     return WSTP_RETURN_ERROR;
//   // }

//   // auto it = interpolators.find(handle);
//   // double result = std::numeric_limits<double>::quiet_NaN();

//   // if (it != interpolators.end()) {
//   //     result = it->second->interpolate(x, y);
//   // }

//   // WSNewPacket(link);
//   // WSPutReal(link, result);
//   // return WSTP_RETURN_SUCCESS;
//   return 1;
// }

// Function to delete an interpolator
extern int WSTPDeleteInterpolator(int link) {
  // int handle;

  // if (!WSGetInteger(link, &handle)) {
  //     WSNewPacket(link);
  //     WSPutSymbol(link, "Failed");
  //     return WSTP_RETURN_ERROR;
  // }

  // interpolators.erase(handle);

  // WSNewPacket(link);
  // WSPutSymbol(link, "Null");
  // return WSTP_RETURN_SUCCESS;
  return 0;
}