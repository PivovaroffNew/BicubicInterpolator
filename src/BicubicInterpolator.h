#ifndef INTERIOIRMATH_H
#define INTERIOIRMATH_H
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

/*!
 * \class FunctionNIntegratorBySimpson
 * \brief Класс для выполнения численного интегрирования методом Симпсона.
 *
 * Класс позволяет интегрировать функцию одного переменного по заданному
 * интегрвалу методом Симпсона. Задается количенство интервалов. Шаг сетки
 * постоянен и выисляется из количества инетрвалов.
 */
class FunctionNIntegratorBySimpson {
 public:
  explicit FunctionNIntegratorBySimpson(
      const std::function<double(double)>& function, int n);

  ~FunctionNIntegratorBySimpson() = default;

  double integrate(double param_start, double param_end) const;

 private:
  int n_;
  std::function<double(double)> function_;
};

class ParametricCurveIntegrator {
 public:
  ParametricCurveIntegrator(const BicubicInterpolator& interpolator,
                            std::function<double(double)> xFunc,
                            std::function<double(double)> yFunc);

  double integrate(double t_start, double t_end, int n) const;

 private:
  const BicubicInterpolator& interpolator_;
  std::function<double(double)> xFunc_;
  std::function<double(double)> yFunc_;
};
#endif

