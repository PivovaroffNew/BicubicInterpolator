#include "BicubicInterpolator.h"

typedef std::function<double(double)> RealFuncOfOneVar;

double BicubicInterpolator::cubicInterpolate(double p[4], double x) {
  return p[1] + 0.5 * x *
                    (p[2] - p[0] +
                     x * (2.0 * p[0] - 5.0 * p[1] + 4.0 * p[2] - p[3] +
                          x * (3.0 * (p[1] - p[2]) + p[3] - p[0])));
}
/*!
 * \brief Проверяет, находится ли точка (x, y) в пределах допустимого
 * диапазона.
 * \param[in] x Координата x.
 * \param[in] y Координата y.
 * \return true, если точка находится в пределах диапазона, иначе false.
 */
bool BicubicInterpolator::isInRange(double x, double y) const {
  return x >= 0 && x < cols - 1 && y >= 0 && y < rows - 1;
}

/*!
 * \brief Возвращает индекс с учетом граничных условий.
 * \param[in] idx Исходный индекс.
 * \param[in] max Максимальное значение индекса.
 * \return Индекс, ограниченный диапазоном [0, max - 1].
 */
int BicubicInterpolator::getBoundedIndex(int idx, int max) const {
  if (idx < 0) return 0;
  if (idx >= max) return max - 1;
  return idx;
}

/*!
 * \brief Конструктор класса.
 * \param[in] data Двумерный вектор значений для интерполяции.
 * \throws std::invalid_argument Если входные данные пусты или не являются
 * прямоугольной матрицей.
 *
 * \details
 * Конструктор принимает двумерный вектор значений и проверяет, что он не пуст
 * и является прямоугольной матрицей. В случае нарушения этих условий
 * выбрасывается исключение.
 */
BicubicInterpolator::BicubicInterpolator(
    const std::vector<std::vector<double>>& data) {
  if (data.empty() || data[0].empty()) {
    throw std::invalid_argument("Input data cannot be empty");
  }

  values = data;
  rows = data.size();
  cols = data[0].size();

  // Проверка на прямоугольность матрицы
  for (const auto& row : data) {
    if (row.size() != cols) {
      throw std::invalid_argument("Input data must be a rectangular matrix");
    }
  }
}

BicubicInterpolator::~BicubicInterpolator() = default;

/*!
 * \brief Выполняет бикубическую интерполяцию в точке (x, y).
 * \param[in] x Координата x точки интерполяции.
 * \param[in] y Координата y точки интерполяции.
 * \return Интерполированное значение в точке (x, y).
 *
 * \details
 * Метод выполняет бикубическую интерполяцию в точке (x, y) на основе значений
 * из матрицы, переданной в конструкторе. Если точка (x, y) выходит за пределы
 * допустимого диапазона, координаты ограничиваются ближайшими допустимыми
 * значениями, и выводится предупреждение.
 *
 * \note
 * - Интерполяция выполняется с использованием 16 ближайших точек сетки.
 * - Если точка (x, y) выходит за пределы сетки, координаты ограничиваются
 *   ближайшими допустимыми значениями.
 *
 * \example
 * Пример использования:
 * \code
 * std::vector<std::vector<double>> data = {
 *     {1, 2, 3, 4},
 *     {5, 6, 7, 8},
 *     {9, 10, 11, 12},
 *     {13, 14, 15, 16}
 * };
 * BicubicInterpolator interpolator(data);
 * double result = interpolator.interpolate(1.5, 2.5);
 * std::cout << "Interpolated value: " << result << std::endl;
 * \endcode
 */
double BicubicInterpolator::interpolate(double xx, double yy) const {
  double x = xx;  // ERROR
  double y = yy;  // ERROR
  // Проверка границ
  if (!isInRange(x, y)) {
    std::cerr << "Warning: Interpolation point (" << x << ", " << y
              << ") is outside the data range [0, " << cols - 1 << "] x [0, "
              << rows - 1 << "]\n";

    // Ограничиваем координаты в пределах допустимого диапазона
    x = std::max(0.0, std::min(static_cast<double>(cols - 1.01), x));
    y = std::max(0.0, std::min(static_cast<double>(rows - 1.01), y));
  }

  // Находим ближайший нижний левый узел сетки
  int x0 = static_cast<int>(std::floor(x));
  int y0 = static_cast<int>(std::floor(y));

  // Относительные координаты внутри ячейки сетки
  double dx = x - x0;
  double dy = y - y0;

  // Для каждой строки выполняем кубическую интерполяцию по x
  double points[4][4];  // Матрица 4x4 окружающих точек

  for (int j = -1; j <= 2; j++) {
    for (int i = -1; i <= 2; i++) {
      int yi = getBoundedIndex(y0 + j, rows);
      int xi = getBoundedIndex(x0 + i, cols);
      points[j + 1][i + 1] = values[yi][xi];
    }
  }

  // Интерполяция по x для каждой из 4 строк
  double temp[4];
  for (int j = 0; j < 4; j++) {
    double p[4] = {points[j][0], points[j][1], points[j][2], points[j][3]};
    temp[j] = cubicInterpolate(p, dx);
  }

  // Интерполяция по y, используя результаты интерполяции по x
  return cubicInterpolate(temp, dy);
}

FunctionNIntegratorBySimpson::FunctionNIntegratorBySimpson(
    const RealFuncOfOneVar& function, int n)
    : function_(function) {
  n_ = (n % 2 != 0) ? n + 1 : n;
  if (n_ <= 0) throw std::invalid_argument("n must be positive");
}

// Реализация метода integrate
double FunctionNIntegratorBySimpson::integrate(double param_start,
                                               double param_end) const {
  if (param_start == param_end) return 0.0;

  const double h = (param_end - param_start) / n_;

  double sum = (function_(param_start)) + (function_(param_end));

  for (int i = 1; i < n_; i += 2) {
    sum += 4.0 * (function_(param_start + i * h));
  }
  for (int i = 2; i < n_; i += 2) {
    sum += 2.0 * (function_(param_start + i * h));
  }

  return sum * h / 3.0;
}

ParametricCurveIntegrator::ParametricCurveIntegrator(
    const BicubicInterpolator& interpolator,
    RealFuncOfOneVar xFunc, RealFuncOfOneVar yFunc)
    : interpolator_(interpolator),
      xFunc_(std::move(xFunc)),
      yFunc_(std::move(yFunc)) {}

double ParametricCurveIntegrator::integrate(double t_start, double t_end,
                                            int n) const {
  // Создаем обертку для функции кривой
  auto curveFunc = [this](double t) {
    const double x = this->xFunc_(t);
    const double y = this->yFunc_(t);
    return this->interpolator_.interpolate(x, y);
  };

  // Создаем интегратор и выполняем вычисления
  FunctionNIntegratorBySimpson integrator(curveFunc, n);
  return integrator.integrate(t_start, t_end);
}