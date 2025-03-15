#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <limits>
#include <vector>

/*!
 * \class BicubicInterpolator
 * \brief Класс для выполнения бикубической интерполяции на двумерной сетке.
 *
 * Класс позволяет интерполировать значения на двумерной сетке с использованием
 * бикубической интерполяции. Входные данные представляют собой прямоугольную
 * матрицу значений, а интерполяция выполняется в произвольной точке (x, y).
 */
class BicubicInterpolator {
    public:
        explicit BicubicInterpolator(const std::vector<std::vector<double>>& data);
        ~BicubicInterpolator();
        
        double interpolate(double x, double y) const;
    
    private:
        std::vector<std::vector<double>> values;
        int rows;
        int cols;
    
        static double cubicInterpolate(double p[4], double x);
        bool isInRange(double x, double y) const;
        int getBoundedIndex(int idx, int max) const;
    };