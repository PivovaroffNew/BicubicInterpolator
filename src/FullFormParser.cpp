#include <cmath>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

// Чисто виртуальный класс для представления выражения
class Expression {
public:
    virtual ~Expression() = default;
    virtual std::function<double(const std::unordered_map<std::string, double>&)>
        compile() const = 0;
    virtual std::string toString() const = 0;
};

// Константа
class Constant : public Expression {
private:
    double value;

public:
    explicit Constant(double val) : value(val) {}

    std::function<double(const std::unordered_map<std::string, double>&)>
        compile() const override {
        double val = value;
        return [val](const std::unordered_map<std::string, double>&) -> double {
            return val;
            };
    }

    std::string toString() const override { return std::to_string(value); }
};

// Переменная
class Variable : public Expression {
private:
    std::string name;

public:
    explicit Variable(std::string n) : name(std::move(n)) {}

    std::function<double(const std::unordered_map<std::string, double>&)>
        compile() const override {
        std::string local_name = name;
        return [local_name](
            const std::unordered_map<std::string, double>& vars) -> double {
                auto it = vars.find(local_name);
                if (it == vars.end()) {
                    std::cerr << "Warning: Variable not found: " << local_name
                        << ", using 0.0 as default value" << std::endl;
                    return 0.0;  // Возвращаем 0.0 по умолчанию вместо исключения
                }
                return it->second;
            };
    }

    std::string toString() const override { return name; }
};

// Рациональное число
class Rational : public Expression {
private:
    int numerator;
    int denominator;

public:
    Rational(int num, int denom) : numerator(num), denominator(denom) {}

    std::function<double(const std::unordered_map<std::string, double>&)>
        compile() const override {
        return [this](const std::unordered_map<std::string, double>&) -> double {
            return static_cast<double>(numerator) / denominator;
            };
    }

    std::string toString() const override {
        return std::to_string(numerator) + "/" + std::to_string(denominator);
    }
};

// Функция с несколькими аргументами (или одним)
class Function : public Expression {
private:
    std::string head;
    std::vector<std::shared_ptr<Expression>> args;

public:
    Function(std::string h, std::vector<std::shared_ptr<Expression>> a)
        : head(std::move(h)), args(std::move(a)) {}

    std::function<double(const std::unordered_map<std::string, double>&)>
        compile() const override {
        if (head == "Plus") {
            return compilePlus();
        }
        else if (head == "Times") {
            return compileTimes();
        }
        else if (head == "Power") {
            return compilePower();
        }
        else if (head == "Sin") {
            return compileSin();
        }
        else if (head == "Cos") {
            return compileCos();
        }
        else if (head == "Exp") {
            return compileExp();
        }
        else if (head == "Log") {
            return compileLog();
        }
        else if (head == "Tan") {
            return compileTan();
        }
        else if (head == "Sqrt") {
            return compileSqrt();
        }
        else {
            std::cerr << "Warning: Unsupported function: " << head
                << ", returning 0.0" << std::endl;
            return [](const std::unordered_map<std::string, double>&) -> double {
                return 0.0;  // Возвращаем 0.0 по умолчанию для неподдерживаемых функций
                };
        }
    }

    std::string toString() const override {
        std::stringstream ss;
        ss << head << "[";
        for (size_t i = 0; i < args.size(); ++i) {
            if (i > 0) ss << ", ";
            ss << args[i]->toString();
        }
        ss << "]";
        return ss.str();
    }

private:
    std::function<double(const std::unordered_map<std::string, double>&)>
        compilePlus() const {
        std::vector<
            std::function<double(const std::unordered_map<std::string, double>&)>>
            compiledArgs;
        for (const auto& arg : args) {
            compiledArgs.push_back(arg->compile());
        }

        return [compiledArgs](
            const std::unordered_map<std::string, double>& vars) -> double {
                double sum = 0.0;
                for (const auto& func : compiledArgs) {
                    sum += func(vars);
                }
                return sum;
            };
    }

    std::function<double(const std::unordered_map<std::string, double>&)>
        compileTimes() const {
        std::vector<
            std::function<double(const std::unordered_map<std::string, double>&)>>
            compiledArgs;
        for (const auto& arg : args) {
            compiledArgs.push_back(arg->compile());
        }

        return [compiledArgs](
            const std::unordered_map<std::string, double>& vars) -> double {
                double product = 1.0;
                for (const auto& func : compiledArgs) {
                    product *= func(vars);
                }
                return product;
            };
    }

    std::function<double(const std::unordered_map<std::string, double>&)>
        compilePower() const {
        if (args.size() != 2) {
            std::cerr << "Warning: Power requires exactly 2 arguments, but got "
                << args.size() << std::endl;
            return [](const std::unordered_map<std::string, double>&) -> double {
                return 0.0;
                };
        }

        auto base = args[0]->compile();
        auto exponent = args[1]->compile();

        return [base, exponent](const auto& vars) -> double {
            double b = base(vars);
            double e = exponent(vars);

            // Обработка случая 0^negative
            if (b == 0.0 && e < 0.0) {
                return std::numeric_limits<double>::infinity();
            }

            // Обработка отрицательных оснований с дробными степенями
            if (b < 0.0 && std::trunc(e) != e) {
                std::cerr << "Warning: Negative base with non-integer exponent\n";
                return std::numeric_limits<double>::quiet_NaN();
            }

            return std::pow(b, e);
            };
    }

    std::function<double(const std::unordered_map<std::string, double>&)>
        compileSin() const {
        if (args.size() != 1) {
            std::cerr << "Warning: Sin requires exactly 1 argument, but got "
                << args.size() << std::endl;
            return [](const std::unordered_map<std::string, double>&) -> double {
                return 0.0;
                };
        }

        auto arg = args[0]->compile();

        return
            [arg](const std::unordered_map<std::string, double>& vars) -> double {
            return std::sin(arg(vars));
            };
    }

    std::function<double(const std::unordered_map<std::string, double>&)>
        compileCos() const {
        if (args.size() != 1) {
            std::cerr << "Warning: Cos requires exactly 1 argument, but got "
                << args.size() << std::endl;
            return [](const std::unordered_map<std::string, double>&) -> double {
                return 0.0;
                };
        }

        auto arg = args[0]->compile();

        return
            [arg](const std::unordered_map<std::string, double>& vars) -> double {
            return std::cos(arg(vars));
            };
    }

    std::function<double(const std::unordered_map<std::string, double>&)>
        compileTan() const {
        if (args.size() != 1) {
            std::cerr << "Warning: Tan requires exactly 1 argument, but got "
                << args.size() << std::endl;
            return [](const std::unordered_map<std::string, double>&) -> double {
                return 0.0;
                };
        }

        auto arg = args[0]->compile();

        return
            [arg](const std::unordered_map<std::string, double>& vars) -> double {
            return std::tan(arg(vars));
            };
    }

    std::function<double(const std::unordered_map<std::string, double>&)>
        compileSqrt() const {
        if (args.size() != 1) {
            std::cerr << "Warning: Sqrt requires exactly 1 argument, but got "
                << args.size() << std::endl;
            return [](const std::unordered_map<std::string, double>&) -> double {
                return 0.0;
                };
        }

        auto arg = args[0]->compile();

        return
            [arg](const std::unordered_map<std::string, double>& vars) -> double {
            return std::sqrt(arg(vars));
            };
    }

    std::function<double(const std::unordered_map<std::string, double>&)>
        compileExp() const {
        if (args.size() != 1) {
            std::cerr << "Warning: Exp requires exactly 1 argument, but got "
                << args.size() << std::endl;
            return [](const std::unordered_map<std::string, double>&) -> double {
                return 0.0;
                };
        }

        auto arg = args[0]->compile();

        return
            [arg](const std::unordered_map<std::string, double>& vars) -> double {
            return std::exp(arg(vars));
            };
    }

    std::function<double(const std::unordered_map<std::string, double>&)>
        compileLog() const {
        if (args.size() == 1) {
            auto arg = args[0]->compile();
            return
                [arg](const std::unordered_map<std::string, double>& vars) -> double {
                return std::log(arg(vars));
                };
        }
        else if (args.size() == 2) {
            auto base = args[0]->compile();
            auto arg = args[1]->compile();
            return
                [base,
                arg](const std::unordered_map<std::string, double>& vars) -> double {
                return std::log(arg(vars)) / std::log(base(vars));
                };
        }
        else {
            std::cerr << "Warning: Log requires 1 or 2 arguments, but got "
                << args.size() << std::endl;
            return [](const std::unordered_map<std::string, double>&) -> double {
                return 0.0;
                };
        }
    }
};

// Lambda-функция (например, Sin[#]&)
class Lambda : public Expression {
private:
    std::shared_ptr<Expression> body;

public:
    explicit Lambda(std::shared_ptr<Expression> b) : body(std::move(b)) {}

    std::function<double(const std::unordered_map<std::string, double>&)>
        compile() const override {
        auto compiledBody = body->compile();

        return [compiledBody](
            const std::unordered_map<std::string, double>& vars) -> double {
                return compiledBody(vars);
            };
    }

    // Создаем функцию одной переменной
    std::function<double(double)> compileUnary() const {
        auto compiledBody = body->compile();

        return [compiledBody](double x) -> double {
            // Создаем карту с "#" в качестве ключа для лямбда-параметра
            std::unordered_map<std::string, double> vars = { {"#", x} };
            return compiledBody(vars);
            };
    }

    std::string toString() const override { return body->toString() + "&"; }
};

class MathematicaParser {
private:
    std::string input;
    size_t pos = 0;

    char peek() const {
        if (pos >= input.size()) return '\0';
        return input[pos];
    }

    char consume() {
        if (pos >= input.size()) return '\0';
        return input[pos++];
    }

    void skipWhitespace() {
        while (pos < input.size() && std::isspace(input[pos])) {
            ++pos;
        }
    }

    bool consumeIf(char c) {
        skipWhitespace();
        if (peek() == c) {
            consume();
            return true;
        }
        return false;
    }

    bool isIdentifierChar(char c) {
        return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
    }

    std::string parseIdentifier() {
        skipWhitespace();
        // Если встречаем разделитель, сигнализируем об ошибке или возвращаем
        // специальное значение
        char c = peek();
        if (c == ']' || c == ',' || c == '&') {
            throw std::runtime_error(
                "Unexpected delimiter when expecting identifier at position " +
                std::to_string(pos));
        }

        // Если первый символ – '#' (лямбда-параметр), сразу возвращаем его
        if (c == '#') {
            consume();
            return "#";
        }

        std::string result;
        if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
            result += consume();
            while (true) {
                c = peek();
                // Разрешаем только буквы, цифры и '_'
                if (std::isalnum(static_cast<unsigned char>(c)) || c == '_') {
                    result += consume();
                }
                else {
                    break;
                }
            }
        }
        else {
            throw std::runtime_error(
                "Unexpected character when expecting identifier: " +
                std::string(1, c));
        }
        return result;
    }

    double parseNumber() {
        skipWhitespace();
        std::string numStr;
        bool hasDecimal = false;
        bool hasExponent = false;

        if (peek() == '-') {
            numStr += consume();
        }

        while (true) {
            char c = peek();
            if (std::isdigit(c) || (!hasDecimal && c == '.') ||
                (!hasExponent && (c == 'e' || c == 'E'))) {
                if (c == '.') hasDecimal = true;
                if (c == 'e' || c == 'E') hasExponent = true;
                numStr += consume();
            }
            else {
                break;
            }
        }

        if (numStr.empty() || numStr == "-" || numStr == ".") {
            throw std::runtime_error("Invalid number format");
        }

        return std::stod(numStr);
    }

public:
    explicit MathematicaParser(std::string in) : input(std::move(in)) {}

    std::shared_ptr<Expression> parse() {
        skipWhitespace();
        auto expr = parseExpression();
        skipWhitespace();

        // Проверка на лямбда-функцию (например, Sin[#]&)
        if (consumeIf('&')) {
            return std::make_shared<Lambda>(expr);
        }

        return expr;
    }

    std::shared_ptr<Expression> parseExpression() {
        skipWhitespace();
        // Если следующий символ является разделителем, то ничего читать не надо
        char c = peek();
        if (c == ']' || c == ',' || c == '&') {
            throw std::runtime_error(
                "Unexpected delimiter when expecting an expression at position " +
                std::to_string(pos));
        }
        // Если константа
        if (std::isdigit(c) || c == '-' || c == '.') {
            return std::make_shared<Constant>(parseNumber());
        }
        // Проверяем что мдет дальше (функция или переменна)
        std::string identifier = parseIdentifier();
        if (identifier.empty()) {
            throw std::runtime_error("Expected identifier or number at position " +
                std::to_string(pos));
        }
        // Если за идентификатором следует открывающая скобка – это функция
        if (consumeIf('[')) {
            std::vector<std::shared_ptr<Expression>> args;
            skipWhitespace();
            // Если сразу закрывающая скобка – аргументов нет
            if (peek() != ']') {
                // Первый аргумент
                args.push_back(parseExpression());
                while (true) {
                    skipWhitespace();
                    // Если следующий символ – закрывающая скобка, выходим из цикла
                    if (peek() == ']') {
                        break;
                    }
                    // Если запятая – пропускаем её и парсим следующий аргумент
                    if (consumeIf(',')) {
                        skipWhitespace();
                        // Если после запятой сразу закрывающая скобка, значит аргумент
                        // отсутствует
                        if (peek() == ']') {
                            break;  // либо можно бросить исключение, если это считать ошибкой
                        }
                        args.push_back(parseExpression());
                    }
                    else {
                        throw std::runtime_error("Expected ',' or ']' at position " +
                            std::to_string(pos));
                    }
                }
            }
            if (!consumeIf(']')) {
                throw std::runtime_error("Expected closing bracket at position " +
                    std::to_string(pos));
            }
            // Специальная обработка для Rational
            if (identifier == "Rational" && args.size() == 2) {
                auto numExpr = std::dynamic_pointer_cast<Constant>(args[0]);
                auto denomExpr = std::dynamic_pointer_cast<Constant>(args[1]);
                if (numExpr && denomExpr) {
                    int num = static_cast<int>(numExpr->compile()({}));
                    int denom = static_cast<int>(denomExpr->compile()({}));
                    return std::make_shared<Rational>(num, denom);
                }
            }
            return std::make_shared<Function>(identifier, args);
        }
        // Иначе – переменная
        return std::make_shared<Variable>(identifier);
    }

    // Метод для преобразования строки FullForm в std::function
    static std::function<double(double)> parseFunction(
        const std::string& fullFormStr) {
        MathematicaParser parser(fullFormStr);
        auto expr = parser.parse();

        // Если это лямбда, то возвращаем функцию одной переменной
        if (auto lambda = std::dynamic_pointer_cast<Lambda>(expr)) {
            return lambda->compileUnary();
        }

        // Иначе предполагаем, что есть одна переменная 'x'
        auto compiled = expr->compile();
        return [compiled](double x) -> double {
            std::unordered_map<std::string, double> vars = { {"x", x} };
            return compiled(vars);
            };
    }

    // Метод для преобразования строки FullForm в std::function, принимающую
    // произвольное количество аргументов
    static std::function<double(const std::unordered_map<std::string, double>&)>
        parseMultiVarFunction(const std::string& fullFormStr) {
        MathematicaParser parser(fullFormStr);
        auto expr = parser.parse();
        return expr->compile();
    }
};

// Вспомогательная функция для проверки равенства значений с плавающей точкой
bool almostEqual(double a, double b, double epsilon = 1e-10) {
    return std::abs(a - b) < epsilon;
}

// Пример использования
int main() {
    try {
        // Пример: Plus[Times[Power[5, Rational[-1, 2]], x], Power[y, 2], Power[z, -1]]
        std::string fullForm =
            "Plus[Times[Power[5, Rational[-1, 2]], x], Power[y, 2], Power[z, -1]]";
        MathematicaParser parser(fullForm);
        auto expr = parser.parse();
        std::cout << "Parsed expression: " << expr->toString() << std::endl;

        // Компилируем выражение в функцию
        auto func = expr->compile();

        // Вычисляем значение функции для конкретных значений переменных
        std::unordered_map<std::string, double> vars = {
            {"x", 2.0}, {"y", 3.0}, {"z", 4.0} };
        double result = func(vars);
        std::cout << "Evaluation result: " << result << std::endl;

        // Проверяем правильность вычислений
        double expected = 2.0 / std::sqrt(5.0) + 3.0 * 3.0 + 1.0 / 4.0;
        std::cout << "Expected result: " << expected << std::endl;
        std::cout << "Is correct: "
            << (almostEqual(result, expected) ? "Yes" : "No") << std::endl;

        std::string fullFormSin = "Sin[x]";
        auto SinNaive = MathematicaParser::parseFunction(fullFormSin);
        double resultSin = SinNaive(0.5);
        std::cout << "Evaluation SinNaive result: " << resultSin << std::endl;
        std::cout << "Expected SinNaive result: " << std::sin(0.5) << std::endl;

        //// Тест на Sin[Cos[x]]
        // std::string fullFormSin = "Sin[Cos[x]]";
        // MathematicaParser parserSin(fullFormSin);
        // auto exprSin = parserSin.parse();
        // std::cout << "Parsed SinCos expression: " << exprSin->toString() <<
        // std::endl; auto funcSin = exprSin->compile();
        // std::unordered_map<std::string, double> varsSin = { {"x", 0.5} };
        // double resultSin = funcSin(varsSin);
        // std::cout << "Evaluation SinCos result: " << resultSin << std::endl;
        // std::cout << "Expected SinCos result: " << std::sin(cos(0.5)) <<
        // std::endl;

        // Sin[Plus[Times[2,x],Cos[y]]]

        /*std::string fullFormSin = "Sin[Plus[Times[2,x],Cos[y]]]";
        MathematicaParser parserSin(fullFormSin);
        auto exprSin = parserSin.parse();
        std::cout << "Parsed Sin[Plus[Times[2,x],Cos[y]]] expression: " <<
        exprSin->toString() << std::endl; auto funcSin = exprSin->compile();
        std::unordered_map<std::string, double> varsSin = {{"x", 0.5}, {"y", 1}};
        double resultSin = funcSin(varsSin);
        std::cout << "Evaluation Sin[Plus[Times[2,x],Cos[y]]] result: " << resultSin
        << std::endl; std::cout << "Expected Sin[Plus[Times[2,x],Cos[y]]] result: "
        << std::sin(2* 0.5 + std::cos(1.0)) << std::endl;*/

        // Пример для лямбда-функции Sin[#]&
        std::string lambdaStr = "Sin[#]&";

        auto sinFunc = MathematicaParser::parseFunction(lambdaStr);
        std::cout << "Sin(0.5) = " << sinFunc(0.5)
            << " (expected: " << std::sin(0.5) << ")" << std::endl;

        // Тест на отсутствующую переменную
        std::unordered_map<std::string, double> incompleteVars = { {"x", 2.0},
                                                                  {"y", 3.0} };
        double resultWithMissingVar = func(incompleteVars);
        std::cout << "Result with missing 'z': " << resultWithMissingVar
            << std::endl;

        // Пример для более сложной лямбда-функции
        std::string complexLambdaStr =
            "Plus[Times[2, Power[#, 2]], Times[3, #], 1]&";
        auto quadraticFunc = MathematicaParser::parseFunction(complexLambdaStr);
        std::cout << "2x^2 + 3x + 1 at x=2: " << quadraticFunc(2.0)
            << " (expected: " << 2 * 2 * 2 + 3 * 2 + 1 << ")" << std::endl;

        // Пример для неподдерживаемой функции
        std::string unsupportedStr = "Gamma[x]";
        auto unsupportedFunc = MathematicaParser::parseFunction(unsupportedStr);
        std::cout << "Unsupported function result: " << unsupportedFunc(2.0)
            << std::endl;

    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
