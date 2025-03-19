#include <limits>
#include <map>
#include <memory>
#include <vector>

#include "BicubicInterpolator.h"
#include "wstp.h"
#define WSTP_RETURN_SUCCESS 0
#define WSTP_RETURN_ERROR 1
// Map for storing interpolator instances
static std::map<int, std::unique_ptr<BicubicInterpolator>> interpolators;
static int interpolator_handle = 1;

// The actual implementation functions
extern "C" {
#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

// Function to create a new interpolator
EXPORT int WSTPCreateInterpolator(WSLINK link) {
    double* data = nullptr;
    long* dims = nullptr;
    long depth = 0;
    
    // Get the real array (expected 2D array)
    if (!WSGetRealArray(link, &data, &dims, nullptr, &depth)) {
        WSNewPacket(link);
        WSPutInteger(link, -1);  // Error code
        return WSTP_RETURN_ERROR;
    }
    
    // Check that the array is 2D
    if (depth != 2) {
        // Clean up and return error
        WSReleaseRealArray(link, data, dims, nullptr, depth);
        WSNewPacket(link);
        WSPutInteger(link, -1);
        return WSTP_RETURN_ERROR;
    }
    
    // Convert continuous array to 2D std::vector
    std::vector<std::vector<double>> matrix(dims[0], std::vector<double>(dims[1]));
    for (long i = 0; i < dims[0]; i++) {
        for (long j = 0; j < dims[1]; j++) {
            matrix[i][j] = data[i * dims[1] + j];
        }
    }
    
    // Free memory allocated by WSGetRealArray
    WSReleaseRealArray(link, data, dims, nullptr, depth);
    
    try {
        // Create a new interpolator
        std::unique_ptr<BicubicInterpolator> interpolator = 
            std::make_unique<BicubicInterpolator>(matrix);
        int handle = interpolator_handle++;
        interpolators[handle] = std::move(interpolator);
        
        WSNewPacket(link);
        WSPutInteger(link, handle);
        return WSTP_RETURN_SUCCESS;
    } catch (...) {
        WSNewPacket(link);
        WSPutInteger(link, -1);
        return WSTP_RETURN_ERROR;
    }
}

// Function to interpolate at a point
EXPORT int WSTPInterpolatePoint(WSLINK link) {
    int handle;
    double x, y;
    
    if (!WSGetInteger(link, &handle) || !WSGetReal(link, &x) || !WSGetReal(link, &y)) {
        WSNewPacket(link);
        WSPutReal(link, std::numeric_limits<double>::quiet_NaN());
        return WSTP_RETURN_ERROR;
    }
    
    auto it = interpolators.find(handle);
    double result = std::numeric_limits<double>::quiet_NaN();
    
    if (it != interpolators.end()) {
        result = it->second->interpolate(x, y);
    }
    
    WSNewPacket(link);
    WSPutReal(link, result);
    return WSTP_RETURN_SUCCESS;
}

// Function to delete an interpolator
EXPORT int WSTPDeleteInterpolator(WSLINK link) {
    int handle;
    
    if (!WSGetInteger(link, &handle)) {
        WSNewPacket(link);
        WSPutSymbol(link, "Failed");
        return WSTP_RETURN_ERROR;
    }
    
    interpolators.erase(handle);
    
    WSNewPacket(link);
    WSPutSymbol(link, "Null");
    return WSTP_RETURN_SUCCESS;
}
}