#pragma once
#ifndef MATH_ENGINE_H
#define MATH_ENGINE_H
#endif

#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <cmath>
#include <stdexcept>
#include <cctype>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <map>
#include <set>
#include <numeric>
#include "JHCOMMAND3.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

double CalculateMathFunction(const std::string& func, double value);

std::string DecToHex(unsigned int k);
std::string HexToDec(const std::string& hex);
bool IsHexString(const std::string& str);

std::string DecToHex(unsigned int k) {
    if (k == 0) return "0x0";

    std::string a;
    unsigned int n = k;

    while (n > 0) {
        int remainder = n % 16;
        if (remainder < 10)
            a += static_cast<char>('0' + remainder);
        else
            a += static_cast<char>('A' + remainder - 10);
        n /= 16;
    }

    std::reverse(a.begin(), a.end());

    return "0x" + a;
}

std::string HexToDec(const std::string& hex) {
    std::string hexStr = hex;

    if (hexStr.size() >= 2 && (hexStr[0] == '0' && (hexStr[1] == 'x' || hexStr[1] == 'X'))) {
        hexStr = hexStr.substr(2);
    }

    hexStr.erase(remove_if(hexStr.begin(), hexStr.end(), ::isspace), hexStr.end());

    if (hexStr.empty()) return "0";

    unsigned int result = 0;
    for (char c : hexStr) {
        result *= 16;
        if (c >= '0' && c <= '9') {
            result += (c - '0');
        }
        else if (c >= 'A' && c <= 'F') {
            result += (c - 'A' + 10);
        }
        else if (c >= 'a' && c <= 'f') {
            result += (c - 'a' + 10);
        }
        else {
            return "错误：无效的十六进制字符 '" + std::string(1, c) + "'";
        }
    }

    return std::to_string(result);
}

bool IsHexString(const std::string& str) {
    std::string s = str;
    if (s.size() >= 2 && (s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))) {
        s = s.substr(2);
    }
    if (s.empty()) return false;
    for (char c : s) {
        if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))) {
            return false;
        }
    }
    return true;
}

int GetOperatorPriority(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    if (op == '^') return 3;
    return 0;
}

double ApplyOperator(double a, double b, char op) {
    switch (op) {
    case '+': return a + b;
    case '-': return a - b;
    case '*': return a * b;
    case '/':
        if (b == 0) throw std::runtime_error("除数不能为0");
        return a / b;
    case '^': return pow(a, b);
    default: throw std::runtime_error("未知运算符");
    }
}

bool IsOperator(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '^';
}

bool IsDigitOrDot(char c) {
    return isdigit(c) || c == '.';
}

double EvaluateExpression(const std::string& expr) {
    std::stack<double> values;
    std::stack<char> operators;

    size_t i = 0;
    int len = expr.length();

    while (i < len) {
        char c = expr[i];

        if (isspace(c)) {
            i++;
            continue;
        }

        if (IsDigitOrDot(c)) {
            std::string numStr;
            bool hasDot = false;

            while (i < len && IsDigitOrDot(expr[i])) {
                if (expr[i] == '.') {
                    if (hasDot) throw std::runtime_error("数字格式错误：多个小数点");
                    hasDot = true;
                }
                numStr += expr[i];
                i++;
            }

            double num = std::stod(numStr);
            values.push(num);
            continue;
        }

        if (c == '(') {
            operators.push(c);
            i++;
            continue;
        }

        if (c == ')') {
            while (!operators.empty() && operators.top() != '(') {
                char op = operators.top(); operators.pop();
                if (values.size() < 2) throw std::runtime_error("表达式错误");
                double b = values.top(); values.pop();
                double a = values.top(); values.pop();
                values.push(ApplyOperator(a, b, op));
            }
            if (operators.empty()) throw std::runtime_error("括号不匹配");
            operators.pop();
            i++;
            continue;
        }

        if (c == '-' && (i == 0 || expr[i - 1] == '(' || IsOperator(expr[i - 1]))) {
            i++;
            std::string numStr;
            if (i < len && expr[i] == '(') {
                values.push(0);
                operators.push('-');
                continue;
            }
            while (i < len && IsDigitOrDot(expr[i])) {
                numStr += expr[i];
                i++;
            }
            if (numStr.empty()) throw std::runtime_error("表达式错误");
            double num = -std::stod(numStr);
            values.push(num);
            continue;
        }

        if (IsOperator(c)) {
            while (!operators.empty() && GetOperatorPriority(operators.top()) >= GetOperatorPriority(c)) {
                char op = operators.top(); operators.pop();
                if (values.size() < 2) throw std::runtime_error("表达式错误");
                double b = values.top(); values.pop();
                double a = values.top(); values.pop();
                values.push(ApplyOperator(a, b, op));
            }
            operators.push(c);
            i++;
            continue;
        }

        throw std::runtime_error(std::string("非法字符: ") + c);
    }

    while (!operators.empty()) {
        char op = operators.top(); operators.pop();
        if (values.size() < 2) throw std::runtime_error("表达式错误");
        double b = values.top(); values.pop();
        double a = values.top(); values.pop();
        values.push(ApplyOperator(a, b, op));
    }

    if (values.size() != 1) throw std::runtime_error("表达式错误");
    return values.top();
}

struct Frac {
    long long n, d;
    Frac(long long n_ = 0, long long d_ = 1) {
        if (d_ == 0) d_ = 1;
        if (d_ < 0) { n_ = -n_; d_ = -d_; }
        long long g = gcd(llabs(n_), llabs(d_));
        n = n_ / g;
        d = d_ / g;
    }
    static long long gcd(long long a, long long b) {
        while (b) { auto t = b; b = a % b; a = t; }
        return a;
    }
    Frac operator+(const Frac& o) const { return Frac(n * o.d + o.n * d, d * o.d); }
    Frac operator-(const Frac& o) const { return Frac(n * o.d - o.n * d, d * o.d); }
    Frac operator*(const Frac& o) const { return Frac(n * o.n, d * o.d); }
    Frac operator/(const Frac& o) const {
        if (o.n == 0) throw std::runtime_error("除零");
        return Frac(n * o.d, d * o.n);
    }
    bool operator==(const Frac& o) const { return n == o.n && d == o.d; }
    double toDouble() const { return (double)n / d; }
    std::string toString() const {
        if (d == 1) return std::to_string(n);
        return std::to_string(n) + "/" + std::to_string(d);
    }
};

std::ostream& operator<<(std::ostream& os, const Frac& f) {
    if (f.d == 1) return os << f.n;
    return os << f.n << "/" << f.d;
}

std::string removeSpacesEq(const std::string& s) {
    std::string r;
    for (char c : s) if (!isspace(c)) r += c;
    return r;
}

struct TermEq {
    Frac c; char v;
    TermEq(Frac c_ = {}, char v_ = 0) : c(c_), v(v_) {}
};

Frac parseNumberEq(const std::string& s, size_t& p) {
    long long a = 0, b = 1;
    while (p < s.size() && isdigit(s[p])) {
        a = a * 10 + s[p++] - '0';
    }
    if (p < s.size() && s[p] == '.') {
        p++;
        long long f = 0, base = 1;
        while (p < s.size() && isdigit(s[p])) {
            f = f * 10 + s[p++] - '0';
            base *= 10;
        }
        a = a * base + f;
        b = base;
    }
    if (p < s.size() && s[p] == '/') {
        p++;
        long long c = 0;
        while (p < s.size() && isdigit(s[p])) {
            c = c * 10 + s[p++] - '0';
        }
        if (c > 0) b = c;
    }
    return Frac(a, b);
}

std::vector<TermEq> expandBracketsEq(const std::string& expr) {
    std::vector<TermEq> result;
    std::string e = removeSpacesEq(expr);
    size_t i = 0;

    while (i < e.size()) {
        int sign = 1;
        if (e[i] == '+') { i++; }
        else if (e[i] == '-') { sign = -1; i++; }

        Frac coeff(1, 1);
        bool hasCoeff = false;

        if (i < e.size() && (isdigit(e[i]) || e[i] == '.')) {
            coeff = parseNumberEq(e, i);
            hasCoeff = true;
        }

        if (i < e.size() && e[i] == '(') {
            i++;
            size_t j = i;
            int depth = 1;
            while (j < e.size() && depth > 0) {
                if (e[j] == '(') depth++;
                else if (e[j] == ')') depth--;
                j++;
            }
            std::string inner = e.substr(i, j - i - 1);
            std::vector<TermEq> innerTerms = expandBracketsEq(inner);
            for (auto& term : innerTerms) {
                term.c = term.c * coeff;
                term.c = Frac(term.c.n * sign, term.c.d);
                result.push_back(term);
            }
            i = j;
        }
        else if (i < e.size() && (e[i] == 'x' || e[i] == 'y' || e[i] == 'z')) {
            char var = e[i];
            i++;
            if (!hasCoeff) coeff = Frac(1, 1);
            if (i < e.size() && e[i] == '/') {
                i++;
                Frac den(1, 1);
                if (i < e.size() && (isdigit(e[i]) || e[i] == '.')) {
                    den = parseNumberEq(e, i);
                }
                coeff = coeff / den;
            }
            coeff = Frac(coeff.n * sign, coeff.d);
            result.push_back(TermEq(coeff, var));
        }
        else if (hasCoeff) {
            coeff = Frac(coeff.n * sign, coeff.d);
            result.push_back(TermEq(coeff, 0));
        }
        else {
            i++;
        }
    }
    return result;
}

void combineEq(std::vector<TermEq>& t, Frac& x, Frac& y, Frac& c) {
    x = y = c = Frac(0, 1);
    for (auto& term : t) {
        if (term.v == 'x') x = x + term.c;
        else if (term.v == 'y') y = y + term.c;
        else c = c + term.c;
    }
}

bool parseEq(const std::string& eq, Frac& a, Frac& b, Frac& c) {
    size_t pos = eq.find('=');
    if (pos == std::string::npos) return false;

    std::string left = eq.substr(0, pos);
    std::string right = eq.substr(pos + 1);

    std::vector<TermEq> lt = expandBracketsEq(left);
    std::vector<TermEq> rt = expandBracketsEq(right);

    Frac lx, ly, lc, rx, ry, rc;
    combineEq(lt, lx, ly, lc);
    combineEq(rt, rx, ry, rc);

    a = lx - rx;
    b = ly - ry;
    c = rc - lc;

    return true;
}

struct Term {
    Frac c; char v;
    Term(Frac c_ = {}, char v_ = 0) : c(c_), v(v_) {}
};

Frac parseNumber(const std::string& s, size_t& p) {
    long long intPart = 0;
    long long fracPart = 0;
    long long fracBase = 1;
    bool hasDecimal = false;

    while (p < s.size() && isdigit(s[p])) {
        intPart = intPart * 10 + (s[p] - '0');
        p++;
    }

    if (p < s.size() && s[p] == '.') {
        hasDecimal = true;
        p++;
        while (p < s.size() && isdigit(s[p])) {
            fracPart = fracPart * 10 + (s[p] - '0');
            fracBase *= 10;
            p++;
        }
    }

    long long numerator = intPart;
    long long denominator = 1;

    if (hasDecimal) {
        numerator = intPart * fracBase + fracPart;
        denominator = fracBase;
    }

    if (p < s.size() && s[p] == '/') {
        p++;
        long long denom = 0;
        while (p < s.size() && isdigit(s[p])) {
            denom = denom * 10 + (s[p] - '0');
            p++;
        }
        if (denom > 0) {
            if (hasDecimal) {
                numerator = numerator * denom;
                denominator = denominator * denom;
            }
            else {
                numerator = intPart;
                denominator = denom;
            }
        }
    }

    return Frac(numerator, denominator);
}

std::string removeSpaces(const std::string& s) {
    std::string r;
    for (char c : s) if (!isspace(c)) r += c;
    return r;
}

std::vector<Term> expandBrackets(const std::string& expr) {
    std::vector<Term> result;
    std::string e = removeSpaces(expr);
    size_t i = 0;

    while (i < e.size()) {
        int sign = 1;
        if (e[i] == '+') {
            i++;
        }
        else if (e[i] == '-') {
            sign = -1;
            i++;
        }

        Frac coeff(1, 1);
        bool hasCoeff = false;

        if (i < e.size() && (isdigit(e[i]) || e[i] == '.')) {
            coeff = parseNumber(e, i);
            hasCoeff = true;
        }

        if (i < e.size() && e[i] == '(') {
            i++;
            size_t j = i;
            int depth = 1;
            while (j < e.size() && depth > 0) {
                if (e[j] == '(') depth++;
                else if (e[j] == ')') depth--;
                j++;
            }
            std::string inner = e.substr(i, j - i - 1);
            std::vector<Term> innerTerms = expandBrackets(inner);
            for (auto& term : innerTerms) {
                term.c = term.c * coeff;
                term.c = Frac(term.c.n * sign, term.c.d);
                result.push_back(term);
            }
            i = j;
        }
        else if (i < e.size() && e[i] == 'x') {
            char var = e[i];
            i++;
            if (!hasCoeff) coeff = Frac(1, 1);

            if (i < e.size() && e[i] == '/') {
                i++;
                if (i < e.size() && (isdigit(e[i]) || e[i] == '.')) {
                    Frac divisor = parseNumber(e, i);
                    coeff = coeff / divisor;
                }
            }

            coeff = Frac(coeff.n * sign, coeff.d);
            result.push_back(Term(coeff, var));
        }
        else if (hasCoeff) {
            coeff = Frac(coeff.n * sign, coeff.d);
            result.push_back(Term(coeff, 0));
        }
        else {
            i++;
        }
    }
    return result;
}

void combine(std::vector<Term>& t, Frac& x, Frac& c) {
    x = c = Frac(0, 1);
    for (auto& term : t) {
        if (term.v == 'x') x = x + term.c;
        else c = c + term.c;
    }
}

bool parseLinearEq(const std::string& eq, Frac& a, Frac& c) {
    size_t pos = eq.find('=');
    if (pos == std::string::npos) return false;

    std::string left = eq.substr(0, pos);
    std::string right = eq.substr(pos + 1);

    std::vector<Term> lt = expandBrackets(left);
    std::vector<Term> rt = expandBrackets(right);

    Frac lx, lc, rx, rc;
    combine(lt, lx, lc);
    combine(rt, rx, rc);

    a = lx - rx;
    c = rc - lc;

    return true;
}

std::vector<std::string> g_scriptLines;
size_t g_currentLine = 0;

void LoadScript(const std::string& scriptPath) {
    g_scriptLines.clear();
    std::ifstream file(scriptPath);
    std::string line;
    while (std::getline(file, line)) {
        g_scriptLines.push_back(line);
    }
    g_currentLine = 0;
}


void combineEq3(std::vector<TermEq>& t, Frac& x, Frac& y, Frac& z, Frac& c) {
    x = y = z = c = Frac(0, 1);
    for (auto& term : t) {
        if (term.v == 'x') x = x + term.c;
        else if (term.v == 'y') y = y + term.c;
        else if (term.v == 'z') z = z + term.c;
        else c = c + term.c;
    }
}

bool parseEq3(const std::string& eq, Frac& a, Frac& b, Frac& c, Frac& d) {
    size_t pos = eq.find('=');
    if (pos == std::string::npos) return false;

    std::string left = eq.substr(0, pos);
    std::string right = eq.substr(pos + 1);

    std::vector<TermEq> lt = expandBracketsEq(left);
    std::vector<TermEq> rt = expandBracketsEq(right);

    Frac lx, ly, lz, lc, rx, ry, rz, rc;
    combineEq3(lt, lx, ly, lz, lc);
    combineEq3(rt, rx, ry, rz, rc);

    a = lx - rx;
    b = ly - ry;
    c = lz - rz;
    d = rc - lc;

    return true;
}

void SolveQuadraticEquation(double a, double b, double c) {
    if (a == 0) {
        if (b == 0) {
            if (c == 0)
                std::cout << "无穷多解（方程恒成立）\n";
            else
                std::cout << "无解（方程矛盾）\n";
        }
        else {
            double x = -c / b;
            std::cout << "这是一元一次方程，解为: x = " << x << "\n";
        }
        return;
    }

    double delta = b * b - 4 * a * c;

    std::cout << "\n方程: " << a << "x^2 + " << b << "x + " << c << " = 0\n";
    std::cout << "判别式 ^ = b^2 - 4ac = " << delta << "\n\n";

    if (delta > 0) {
        double sqrtDelta = sqrt(delta);
        double x1 = (-b + sqrtDelta) / (2 * a);
        double x2 = (-b - sqrtDelta) / (2 * a);
        std::cout << "有两个不相等的实数根:\n";
        std::cout << "  x1 = " << x1 << "\n";
        std::cout << "  x2 = " << x2 << "\n";
    }
    else if (delta == 0) {
        double x = -b / (2 * a);
        std::cout << "有两个相等的实数根（重根）:\n";
        std::cout << "  x = " << x << "\n";
    }
    else {
        double realPart = -b / (2 * a);
        double imagPart = sqrt(-delta) / (2 * a);
        std::cout << "有两个共轭复数根:\n";
        std::cout << "  x1 = " << realPart << " + " << imagPart << "i\n";
        std::cout << "  x2 = " << realPart << " - " << imagPart << "i\n";
    }
}

bool ParseQuadraticEquation(const std::string& eq, double& a, double& b, double& c) {
    a = b = c = 0;

    std::string expr = removeSpaces(eq);

    if (expr.size() >= 2 && expr.substr(expr.size() - 2) == "=0") {
        expr = expr.substr(0, expr.size() - 2);
    }
    else if (expr.size() >= 3 && expr.substr(expr.size() - 3) == "=0") {
        expr = expr.substr(0, expr.size() - 3);
    }
    else if (expr.find('=') != std::string::npos) {
        size_t eqPos = expr.find('=');
        std::string left = expr.substr(0, eqPos);
        std::string right = expr.substr(eqPos + 1);

        try {
            double rightVal = EvaluateExpression(right);

        }
        catch (...) {}
    }

    size_t i = 0;
    int termIndex = 0;
    int sign = 1;
    bool hasCoeff = false;
    double coeff = 0;

    while (i < expr.size()) {
        char c = expr[i];

        if (c == '+' || c == '-') {
            if (hasCoeff) {
                if (termIndex == 0) a += sign * coeff;
                else if (termIndex == 1) b += sign * coeff;
                else c += sign * coeff;
                hasCoeff = false;
                coeff = 0;
            }
            sign = (c == '+') ? 1 : -1;
            i++;
            continue;
        }

        if (isdigit(c) || c == '.') {
            std::string numStr;
            bool hasDot = false;
            while (i < expr.size() && (isdigit(expr[i]) || expr[i] == '.')) {
                if (expr[i] == '.') {
                    if (hasDot) break;
                    hasDot = true;
                }
                numStr += expr[i];
                i++;
            }
            coeff = std::stod(numStr);
            hasCoeff = true;
            continue;
        }

        if (c == 'x' || c == 'X') {
            i++;
            termIndex = 1;
            if (i < expr.size() && expr[i] == '^') {
                if (expr[i] == '^') {
                    i++;
                    if (i < expr.size() && expr[i] == '2') {
                        i++;
                    }
                }
                else if (expr[i] == '^' && expr[i + 1] == '2') {
                    i++;
                }
                termIndex = 0;
            }

            if (!hasCoeff) {
                coeff = 1;
                hasCoeff = true;
            }
            continue;
        }

        i++;
    }

    if (hasCoeff) {
        if (termIndex == 0) a += sign * coeff;
        else if (termIndex == 1) b += sign * coeff;
        else c += sign * coeff;
    }

    if (fabs(a) < 1e-10) a = 0;
    if (fabs(b) < 1e-10) b = 0;
    if (fabs(c) < 1e-10) c = 0;

    return true;
}

void QuadraticEquationSolver() {
    if (!IsRunningAsAdmin()) {
        SetConsoleTitleA("一元二次方程求解");
    }
    else {
        SetConsoleTitleA("管理员: 一元二次方程求解");
    }
    std::cout << "\n========== 一元二次方程求解器 ==========\n";
    std::cout << "支持的格式:\n";
    std::cout << "   标准形式: 2x^2+3x-5=0\n";
    std::cout << "   简写形式: x^2-4=0, 2x^2=8\n";
    std::cout << "   缺省系数: x^2+2x=0, x^2-1=0\n";
    std::cout << "   小数系数: 1.5x^2-2.3x+0.5=0\n";
    std::cout << "输入 'exit' 退出求解器\n\n";

    std::string eq;
    while (true) {
        SetConsoleTitle(L"一元二次方程求解");
        std::cout << "方程:> ";
        std::getline(std::cin, eq);

        if (eq == "exit" || eq == "quit") break;
        if (eq.empty()) continue;

        double a = 0, b = 0, c = 0;

        if (ParseQuadraticEquation(eq, a, b, c)) {
            SolveQuadraticEquation(a, b, c);
        }
        else {
            std::cout << "格式错误！请使用格式如: 2x^2+3x-5=0\n";
        }
        std::cout << "\n";
    }
    std::cout << "退出求解器\n\n";
}

long long EvaluateArithmeticExpression(const std::string& expr) {
    std::string e;
    for (char c : expr) if (!isspace(c)) e += c;

    auto getPriority = [](char op) -> int {
        switch (op) {
        case '|': return 1;
        case '^': return 2;
        case '&': return 3;
        case '+': case '-': return 4;
        case '*': case '/': case '%': return 5;
        case '<': case '>': return 6;
        default: return 0;
        }
        };

    auto applyOp = [](long long a, long long b, char op) -> long long {
        switch (op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/':
            if (b == 0) throw std::runtime_error("除零错误");
            return a / b;
        case '%': return a % b;
        case '&': return a & b;
        case '|': return a | b;
        case '^': return a ^ b;
        default: throw std::runtime_error("未知运算符");
        }
        };

    for (size_t i = 0; i < e.size(); i++) {
        if (e[i] == '<' && i + 1 < e.size() && e[i + 1] == '<') {
            e.replace(i, 2, "L");
        }
        else if (e[i] == '>' && i + 1 < e.size() && e[i + 1] == '>') {
            e.replace(i, 2, "R");
        }
    }

    std::stack<long long> values;
    std::stack<char> ops;

    for (size_t i = 0; i < e.size(); i++) {
        char c = e[i];

        if (isdigit(c)) {
            long long num = 0;
            while (i < e.size() && isdigit(e[i])) {
                num = num * 10 + (e[i] - '0');
                i++;
            }
            values.push(num);
            i--;
        }
        else if (c == '(') {
            ops.push(c);
        }
        else if (c == ')') {
            while (!ops.empty() && ops.top() != '(') {
                long long b = values.top(); values.pop();
                long long a = values.top(); values.pop();
                char op = ops.top(); ops.pop();
                values.push(applyOp(a, b, op));
            }
            if (!ops.empty()) ops.pop();
        }
        else if (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' ||
            c == '&' || c == '|' || c == '^' || c == 'L' || c == 'R') {
            while (!ops.empty() && ops.top() != '(' &&
                getPriority(ops.top()) >= getPriority(c)) {
                long long b = values.top(); values.pop();
                long long a = values.top(); values.pop();
                char op = ops.top(); ops.pop();

                if (op == 'L') values.push(a << b);
                else if (op == 'R') values.push(a >> b);
                else values.push(applyOp(a, b, op));
            }
            ops.push(c);
        }
        else {
            throw std::runtime_error(std::string("非法字符: ") + c);
        }
    }

    while (!ops.empty()) {
        long long b = values.top(); values.pop();
        long long a = values.top(); values.pop();
        char op = ops.top(); ops.pop();
        if (op == 'L') values.push(a << b);
        else if (op == 'R') values.push(a >> b);
        else values.push(applyOp(a, b, op));
    }

    return values.top();
}

std::string ProcessMathCommand(const std::string& input) {
    std::string expr = input;
    size_t start = expr.find_first_not_of(' ');
    if (start == std::string::npos) {
        return "错误: 请输入函数名和参数";
    }
    expr = expr.substr(start);

    size_t spacePos = expr.find(' ');
    if (spacePos == std::string::npos) {
        return "错误: 请使用格式: 函数名 参数 (例如: sin 30)";
    }

    std::string funcName = expr.substr(0, spacePos);
    std::string paramStr = expr.substr(spacePos + 1);

    paramStr.erase(0, paramStr.find_first_not_of(' '));
    paramStr.erase(paramStr.find_last_not_of(' ') + 1);

    if (paramStr.empty()) {
        return "错误: 请输入参数";
    }

    try {
        if (funcName == "fmod") {
            size_t secondSpace = paramStr.find(' ');
            if (secondSpace == std::string::npos) {
                return "错误: fmod 需要两个参数，请使用: fmod a b";
            }
            std::string aStr = paramStr.substr(0, secondSpace);
            std::string bStr = paramStr.substr(secondSpace + 1);
            double a = std::stod(aStr);
            double b = std::stod(bStr);
            if (b == 0) {
                return "错误: 除数不能为0";
            }
            double result = std::fmod(a, b);
            std::stringstream ss;
            ss << std::fixed << std::setprecision(10);
            ss << result;
            std::string res = ss.str();
            res.erase(res.find_last_not_of('0') + 1, std::string::npos);
            if (res.back() == '.') res.pop_back();
            return "fmod(" + aStr + ", " + bStr + ") = " + res;
        }

        double value;
        size_t slashPos = paramStr.find('/');
        if (slashPos != std::string::npos) {
            std::string numPart = paramStr.substr(0, slashPos);
            std::string denPart = paramStr.substr(slashPos + 1);
            double numerator = std::stod(numPart);
            double denominator = std::stod(denPart);
            if (denominator == 0) {
                return "错误: 分母不能为0";
            }
            value = numerator / denominator;
        }
        else {
            try {
                value = EvaluateExpression(paramStr);
            }
            catch (...) {
                value = std::stod(paramStr);
            }
        }

        double result;
        bool isTrig = (funcName == "sin" || funcName == "cos" || funcName == "tan");
        bool isInvTrig = (funcName == "asin" || funcName == "acos" || funcName == "atan");

        if (isTrig) {
            result = CalculateMathFunction(funcName, value);
            std::stringstream ss;
            ss << std::fixed << std::setprecision(10);
            ss << result;
            std::string res = ss.str();
            res.erase(res.find_last_not_of('0') + 1, std::string::npos);
            if (res.back() == '.') res.pop_back();

            double degResult = CalculateMathFunction(funcName, value * M_PI / 180.0);
            std::stringstream ss2;
            ss2 << std::fixed << std::setprecision(10);
            ss2 << degResult;
            std::string degRes = ss2.str();
            degRes.erase(degRes.find_last_not_of('0') + 1, std::string::npos);
            if (degRes.back() == '.') degRes.pop_back();

            return funcName + "(" + paramStr + " rad) = " + res + "\n" +
                funcName + "(" + paramStr + "度) = " + degRes + " (角度模式)";
        }
        else if (isInvTrig) {
            result = CalculateMathFunction(funcName, value);
            std::stringstream ss;
            ss << std::fixed << std::setprecision(10);
            ss << result;
            std::string res = ss.str();
            res.erase(res.find_last_not_of('0') + 1, std::string::npos);
            if (res.back() == '.') res.pop_back();

            double degResult = result * 180.0 / M_PI;
            std::stringstream ss2;
            ss2 << std::fixed << std::setprecision(6);
            ss2 << degResult;
            std::string degRes = ss2.str();
            degRes.erase(degRes.find_last_not_of('0') + 1, std::string::npos);
            if (degRes.back() == '.') degRes.pop_back();

            return funcName + "(" + paramStr + ") = " + res + " rad\n" +
                "                = " + degRes + "度";
        }
        else {
            result = CalculateMathFunction(funcName, value);
            std::stringstream ss;
            ss << std::fixed << std::setprecision(10);
            ss << result;
            std::string res = ss.str();
            res.erase(res.find_last_not_of('0') + 1, std::string::npos);
            if (res.back() == '.') res.pop_back();

            std::string displayName = funcName;
            if (funcName == "log") displayName = "ln";
            else if (funcName == "log10") displayName = "log??";
            else if (funcName == "log2") displayName = "log?";
            else if (funcName == "abs") displayName = "|";

            if (funcName == "abs") {
                return "|" + paramStr + "| = " + res;
            }
            return displayName + "(" + paramStr + ") = " + res;
        }
    }
    catch (const std::exception& e) {
        return "错误: " + std::string(e.what());
    }
}

double CalculateMathFunction(const std::string& func, double value) {
    if (func == "sin") return std::sin(value);
    if (func == "cos") return std::cos(value);
    if (func == "tan") return std::tan(value);
    if (func == "asin") {
        if (value < -1 || value > 1)
            throw std::runtime_error("asin 的自变量必须在 [-1, 1] 范围内");
        return std::asin(value);
    }
    if (func == "acos") {
        if (value < -1 || value > 1)
            throw std::runtime_error("acos 的自变量必须在 [-1, 1] 范围内");
        return std::acos(value);
    }
    if (func == "atan") return std::atan(value);
    if (func == "exp") return std::exp(value);
    if (func == "log") {
        if (value <= 0)
            throw std::runtime_error("log 的自变量必须大于 0");
        return std::log(value);
    }
    if (func == "log10") {
        if (value <= 0)
            throw std::runtime_error("log10 的自变量必须大于 0");
        return std::log10(value);
    }
    if (func == "log2") {
        if (value <= 0)
            throw std::runtime_error("log2 的自变量必须大于 0");
        return std::log2(value);
    }
    if (func == "abs") return std::abs(value);
    if (func == "ceil") return std::ceil(value);
    if (func == "floor") return std::floor(value);
    if (func == "round") return std::round(value);
    if (func == "sqrt") {
        if (value < 0)
            throw std::runtime_error("sqrt 的自变量必须 >= 0");
        return std::sqrt(value);
    }
    if (func == "cbrt") return std::cbrt(value);
    throw std::runtime_error("未知函数: " + func);
}

struct InequalityResult {
    std::string variable;
    std::string relation;
    std::string solution;
    bool allRealNumbers;
    bool noSolution;
    bool isSpecialCase;
    std::string specialMessage;
};

InequalityResult SolveLinearInequality(const std::string& ineq) {
    InequalityResult result;
    result.allRealNumbers = false;
    result.noSolution = false;
    result.isSpecialCase = false;

    std::string expr = removeSpacesEq(ineq);
    std::string relation;
    size_t pos = std::string::npos;

    if (expr.find(">=") != std::string::npos) {
        relation = ">=";
        pos = expr.find(">=");
    }
    else if (expr.find("<=") != std::string::npos) {
        relation = "<=";
        pos = expr.find("<=");
    }
    else if (expr.find('>') != std::string::npos) {
        relation = ">";
        pos = expr.find('>');
    }
    else if (expr.find('<') != std::string::npos) {
        relation = "<";
        pos = expr.find('<');
    }
    else {
        throw std::runtime_error("未找到不等号，请使用 >, <, >=, <=");
    }

    result.relation = relation;

    std::string left = expr.substr(0, pos);
    std::string right = expr.substr(pos + relation.size());

    if (left.empty() || right.empty()) {
        throw std::runtime_error("不等式格式错误：等号两边不能为空");
    }

    std::string tempEq = left + "-(" + right + ")=0";

    Frac a, c;
    if (!parseLinearEq(tempEq, a, c)) {
        throw std::runtime_error("表达式解析失败");
    }

    Frac rhs = c;

    bool aIsZero = (a.n == 0);

    if (aIsZero) {
        bool rhsPositive = (rhs.n > 0);
        bool rhsNegative = (rhs.n < 0);
        bool rhsZero = (rhs.n == 0);

        if (rhsZero) {
            if (relation == ">" || relation == "<") {
                result.noSolution = true;
                result.isSpecialCase = true;
                result.specialMessage = "无解（0 " + relation + " 0 不成立）";
            }
            else {
                result.allRealNumbers = true;
                result.isSpecialCase = true;
                result.specialMessage = "全体实数（0 " + relation + " 0 恒成立）";
            }
        }
        else if (rhsPositive) {
            if (relation == ">" || relation == ">=") {
                result.noSolution = true;
                result.isSpecialCase = true;
                result.specialMessage = "无解（0 " + relation + " 正数 不成立）";
            }
            else {
                result.allRealNumbers = true;
                result.isSpecialCase = true;
                result.specialMessage = "全体实数（0 " + relation + " 正数 恒成立）";
            }
        }
        else {
            if (relation == "<" || relation == "<=") {
                result.noSolution = true;
                result.isSpecialCase = true;
                result.specialMessage = "无解（0 " + relation + " 负数 不成立）";
            }
            else {
                result.allRealNumbers = true;
                result.isSpecialCase = true;
                result.specialMessage = "全体实数（0 " + relation + " 负数 恒成立）";
            }
        }
        return result;
    }

    result.variable = "x";
    bool flip = false;

    if (a.n < 0) {
        flip = true;
        a = Frac(-a.n, a.d);
        rhs = Frac(-rhs.n, rhs.d);
    }

    Frac x = rhs / a;

    std::string rel = relation;
    if (flip) {
        if (relation == ">") rel = "<";
        else if (relation == "<") rel = ">";
        else if (relation == ">=") rel = "<=";
        else if (relation == "<=") rel = ">=";
    }

    std::string exactStr = x.toString();
    std::string approxStr;
    if (x.d != 1) {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(6);
        ss << x.toDouble();
        approxStr = ss.str();
        approxStr.erase(approxStr.find_last_not_of('0') + 1, std::string::npos);
        if (approxStr.back() == '.') approxStr.pop_back();
    }

    if (approxStr.empty()) {
        result.solution = "x " + rel + " " + exactStr;
    }
    else {
        result.solution = "x " + rel + " " + exactStr + " (≈ " + approxStr + ")";
    }

    bool isGreater = (rel == ">" || rel == ">=");
    bool isStrict = (rel == ">" || rel == "<");
    std::string leftBracket = isStrict ? "(" : "[";
    std::string rightBracket = isStrict ? ")" : "]";

    if (isGreater) {
        result.solution += "\n区间: " + leftBracket + exactStr + ", +∞)";
    }
    else {
        result.solution += "\n区间: (-∞, " + exactStr + rightBracket;
    }

    return result;
}

std::string SimplifySquareRoot(long long n) {
    if (n < 0) {
        long long absN = std::abs(n);
        unsigned long long sqrtN = static_cast<unsigned long long>(std::sqrt(absN));
        if (sqrtN * sqrtN == static_cast<unsigned long long>(absN)) {
            return std::to_string(sqrtN) + "i";
        }
        long long outside = 1;
        long long inside = absN;
        for (long long i = 2; i * i <= inside; ++i) {
            while (inside % (i * i) == 0) {
                outside *= i;
                inside /= (i * i);
            }
        }
        if (outside == 1) {
            return "√" + std::to_string(absN) + "i";
        }
        else if (inside == 1) {
            return std::to_string(outside) + "i";
        }
        else {
            return std::to_string(outside) + "√" + std::to_string(inside) + "i";
        }
    }
    if (n == 0) return "0";
    if (n == 1) return "1";

    unsigned long long sqrtN = static_cast<unsigned long long>(std::sqrt(n));
    if (sqrtN * sqrtN == static_cast<unsigned long long>(n)) {
        return std::to_string(sqrtN);
    }

    long long outside = 1;
    long long inside = n;

    for (long long i = 2; i * i <= inside; ++i) {
        while (inside % (i * i) == 0) {
            outside *= i;
            inside /= (i * i);
        }
    }

    if (outside == 1) {
        return "√" + std::to_string(n);
    }
    else if (inside == 1) {
        return std::to_string(outside);
    }
    else {
        return std::to_string(outside) + "√" + std::to_string(inside);
    }
}

std::string CalculateSquareRoot(double n) {
    if (n < 0) {
        long long intN = static_cast<long long>(n);
        if (n == intN) {
            return SimplifySquareRoot(intN);
        }
        return std::to_string(n) + " 的平方根是虚数";
    }
    if (n == 0) return "0";

    if (n == static_cast<long long>(n)) {
        long long intN = static_cast<long long>(n);
        std::string exact = SimplifySquareRoot(intN);
        if (exact.find('√') == std::string::npos && exact.find('i') == std::string::npos) {
            return exact;
        }
        double result = std::sqrt(n);
        std::stringstream ss;
        ss << std::fixed << std::setprecision(6);
        ss << result;
        std::string approx = ss.str();
        approx.erase(approx.find_last_not_of('0') + 1, std::string::npos);
        if (approx.back() == '.') approx.pop_back();
        return exact + " = " + approx + "...";
    }

    double result = std::sqrt(n);
    std::stringstream ss;
    ss << std::fixed << std::setprecision(6);
    ss << result;
    std::string approx = ss.str();
    approx.erase(approx.find_last_not_of('0') + 1, std::string::npos);
    if (approx.back() == '.') approx.pop_back();
    return approx + "...";
}

std::string SimplifyCubeRoot(long long n) {
    if (n == 0) return "0";
    if (n == 1) return "1";
    if (n == -1) return "-1";

    bool isNegative = n < 0;
    long long absN = std::abs(n);

    long long cbrtN = static_cast<long long>(std::round(std::cbrt(absN)));
    if (cbrtN * cbrtN * cbrtN == absN) {
        return isNegative ? std::to_string(-cbrtN) : std::to_string(cbrtN);
    }

    long long outside = 1;
    long long inside = absN;

    for (long long i = 2; i * i * i <= inside; ++i) {
        while (inside % (i * i * i) == 0) {
            outside *= i;
            inside /= (i * i * i);
        }
    }

    std::string result;
    if (isNegative) {
        result = "-";
        if (outside > 1) result += std::to_string(outside);
    }
    else {
        if (outside > 1) result += std::to_string(outside);
    }

    if (inside > 1) {
        if (outside > 1) result += "3√";
        else result = (isNegative ? "-" : "") + '3√' + std::to_string(inside);
    }

    return result.empty() ? "0" : result;
}

std::string CalculateCubeRoot(double n) {
    if (n == 0) return "0";

    if (n == static_cast<long long>(n)) {
        long long intN = static_cast<long long>(n);
        std::string exact = SimplifyCubeRoot(intN);
        if (exact.find("3√") == std::string::npos) {
            return exact;
        }
        double result = std::cbrt(n);
        std::stringstream ss;
        ss << std::fixed << std::setprecision(6);
        ss << result;
        std::string approx = ss.str();
        approx.erase(approx.find_last_not_of('0') + 1, std::string::npos);
        if (approx.back() == '.') approx.pop_back();
        return exact + " = " + approx + "...";
    }

    double result = std::cbrt(n);
    std::stringstream ss;
    ss << std::fixed << std::setprecision(6);
    ss << result;
    std::string approx = ss.str();
    approx.erase(approx.find_last_not_of('0') + 1, std::string::npos);
    if (approx.back() == '.') approx.pop_back();
    return approx + "...";
}

// JHmath.h
#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdint>

// ========== 大数基础运算 ==========

std::vector<uint32_t> big_mul(const std::vector<uint32_t>& a, const std::vector<uint32_t>& b) {
    std::vector<uint32_t> res(a.size() + b.size(), 0);
    for (size_t i = 0; i < a.size(); ++i) {
        uint64_t carry = 0;
        for (size_t j = 0; j < b.size(); ++j) {
            uint64_t cur = (uint64_t)a[i] * b[j] + res[i + j] + carry;
            res[i + j] = (uint32_t)(cur & 0xFFFFFFFF);
            carry = cur >> 32;
        }
        if (carry) {
            res[i + b.size()] += (uint32_t)carry;
        }
    }
    while (res.size() > 1 && res.back() == 0) res.pop_back();
    return res;
}

bool big_ge(const std::vector<uint32_t>& a, const std::vector<uint32_t>& b) {
    if (a.size() != b.size()) return a.size() > b.size();
    for (int i = (int)a.size() - 1; i >= 0; --i) {
        if (a[i] != b[i]) return a[i] > b[i];
    }
    return true;
}

void big_sub(std::vector<uint32_t>& a, const std::vector<uint32_t>& b) {
    uint64_t borrow = 0;
    for (size_t i = 0; i < b.size() || borrow; ++i) {
        uint64_t sub = (i < b.size() ? b[i] : 0) + borrow;
        uint64_t cur = (i < a.size() ? a[i] : 0);
        if (cur < sub) {
            a[i] = (uint32_t)(cur + 0x100000000ULL - sub);
            borrow = 1;
        }
        else {
            a[i] = (uint32_t)(cur - sub);
            borrow = 0;
        }
    }
    while (a.size() > 1 && a.back() == 0) a.pop_back();
}

std::vector<uint32_t> big_add(const std::vector<uint32_t>& a, const std::vector<uint32_t>& b) {
    std::vector<uint32_t> res((std::max)(a.size(), b.size()) + 1, 0);
    uint64_t carry = 0;
    for (size_t i = 0; i < res.size(); ++i) {
        uint64_t sum = carry;
        if (i < a.size()) sum += a[i];
        if (i < b.size()) sum += b[i];
        res[i] = (uint32_t)(sum & 0xFFFFFFFF);
        carry = sum >> 32;
    }
    while (res.size() > 1 && res.back() == 0) res.pop_back();
    return res;
}

std::vector<uint32_t> big_div10(const std::vector<uint32_t>& a, uint32_t& rem) {
    std::vector<uint32_t> res(a.size(), 0);
    uint64_t carry = 0;
    for (int i = (int)a.size() - 1; i >= 0; --i) {
        uint64_t cur = (carry << 32) | a[i];
        res[i] = (uint32_t)(cur / 10);
        carry = cur % 10;
    }
    rem = (uint32_t)carry;
    while (res.size() > 1 && res.back() == 0) res.pop_back();
    return res;
}

std::vector<uint32_t> from_decimal(const std::string& s) {
    std::vector<uint32_t> res(1, 0);
    for (char c : s) {
        if (c < '0' || c > '9') continue;
        uint32_t digit = c - '0';
        uint64_t carry = digit;
        for (size_t i = 0; i < res.size(); ++i) {
            uint64_t cur = (uint64_t)res[i] * 10 + carry;
            res[i] = (uint32_t)(cur & 0xFFFFFFFF);
            carry = cur >> 32;
        }
        while (carry) {
            res.push_back((uint32_t)(carry & 0xFFFFFFFF));
            carry >>= 32;
        }
    }
    return res;
}

std::string to_decimal(const std::vector<uint32_t>& num) {
    if (num.size() == 1 && num[0] == 0) return "0";
    std::vector<uint32_t> tmp = num;
    std::string digits;
    while (!(tmp.size() == 1 && tmp[0] == 0)) {
        uint32_t rem;
        tmp = big_div10(tmp, rem);
        digits.push_back('0' + rem);
    }
    std::reverse(digits.begin(), digits.end());
    return digits;
}

std::string sqrt_precision(int n, int digits) {
    if (n < 0) return "NaN";
    if (n == 0) return "0." + std::string(digits, '0');

    int int_part = 0;
    while ((int_part + 1) * (int_part + 1) <= n) {
        int_part++;
    }

    std::string result = std::to_string(int_part);
    if (digits == 0) return result;
    result += ".";

    int remainder_int = n - int_part * int_part;
    std::vector<uint32_t> remainder = from_decimal(std::to_string(remainder_int));
    std::vector<uint32_t> hundred = from_decimal("100");
    remainder = big_mul(remainder, hundred);

    std::vector<uint32_t> divisor = from_decimal(std::to_string(int_part * 20));

    for (int i = 0; i < digits; i++) {
        int b = 0;
        for (int test = 9; test >= 0; test--) {
            std::vector<uint32_t> test_vec = from_decimal(std::to_string(test));
            std::vector<uint32_t> divisor_plus_test = big_add(divisor, test_vec);
            std::vector<uint32_t> product = big_mul(divisor_plus_test, test_vec);

            if (big_ge(remainder, product)) {
                b = test;
                break;
            }
        }

        result.push_back('0' + b);

        std::vector<uint32_t> b_vec = from_decimal(std::to_string(b));
        std::vector<uint32_t> divisor_plus_b = big_add(divisor, b_vec);
        std::vector<uint32_t> product = big_mul(divisor_plus_b, b_vec);
        big_sub(remainder, product);
        remainder = big_mul(remainder, hundred);

        std::vector<uint32_t> two_b = from_decimal(std::to_string(2 * b));
        divisor = big_add(divisor, two_b);
        divisor = big_mul(divisor, from_decimal("10"));
    }

    return result;
}

std::string cbrt_precision(int n, int digits) {
    if (n == 0) return "0." + std::string(digits, '0');

    bool negative = (n < 0);
    long long abs_n = negative ? -(long long)n : (long long)n;

    long long int_part = 0;
    while ((int_part + 1) * (int_part + 1) * (int_part + 1) <= abs_n) {
        int_part++;
    }

    std::string result = std::to_string(int_part);
    if (digits == 0) {
        return negative ? "-" + result : result;
    }
    result += ".";

    long long remainder_int = abs_n - int_part * int_part * int_part;
    std::vector<uint32_t> remainder = from_decimal(std::to_string(remainder_int));
    std::vector<uint32_t> thousand = from_decimal("1000");

    std::vector<uint32_t> a_val = from_decimal(std::to_string(int_part));

    for (int i = 0; i < digits; i++) {
        remainder = big_mul(remainder, thousand);

        int b = 0;
        for (int test = 9; test >= 0; test--) {
            std::vector<uint32_t> b_val = from_decimal(std::to_string(test));

            std::vector<uint32_t> a_square = big_mul(a_val, a_val);
            std::vector<uint32_t> a_square_300 = big_mul(a_square, from_decimal("300"));

            std::vector<uint32_t> a_b = big_mul(a_val, b_val);
            std::vector<uint32_t> a_b_30 = big_mul(a_b, from_decimal("30"));

            std::vector<uint32_t> b_square = big_mul(b_val, b_val);

            std::vector<uint32_t> sum = big_add(a_square_300, a_b_30);
            sum = big_add(sum, b_square);

            std::vector<uint32_t> product = big_mul(sum, b_val);

            if (big_ge(remainder, product)) {
                b = test;
                break;
            }
        }

        result.push_back('0' + b);

        std::vector<uint32_t> b_val = from_decimal(std::to_string(b));
        std::vector<uint32_t> a_square = big_mul(a_val, a_val);
        std::vector<uint32_t> a_square_300 = big_mul(a_square, from_decimal("300"));
        std::vector<uint32_t> a_b = big_mul(a_val, b_val);
        std::vector<uint32_t> a_b_30 = big_mul(a_b, from_decimal("30"));
        std::vector<uint32_t> b_square = big_mul(b_val, b_val);

        std::vector<uint32_t> sum = big_add(a_square_300, a_b_30);
        sum = big_add(sum, b_square);
        std::vector<uint32_t> product = big_mul(sum, b_val);

        big_sub(remainder, product);

        a_val = big_add(big_mul(a_val, from_decimal("10")), b_val);
    }

    return negative ? "-" + result : result;
}

struct STTerm {
    long long coeff;
    std::map<char, int> exps;

    STTerm(long long c = 0) : coeff(c) {}

    bool SameVarPart(const STTerm& o) const {
        return exps == o.exps;
    }

    std::string ToString() const {
        if (coeff == 0) return "0";
        std::string s;
        if (coeff < 0) s += "-";
        long long ac = std::abs(coeff);
        if (ac != 1 || exps.empty()) s += std::to_string(ac);
        for (auto& p : exps) {
            if (p.second == 0) continue;
            s += p.first;
            if (p.second != 1) s += "^" + std::to_string(p.second);
        }
        return s;
    }
};

struct STPoly {
    std::vector<STTerm> terms;

    void Combine() {
        std::map<std::map<char, int>, long long> merged;
        for (size_t i = 0; i < terms.size(); ++i) {
            merged[terms[i].exps] += terms[i].coeff;
        }
        terms.clear();
        for (auto it = merged.begin(); it != merged.end(); ++it) {
            if (it->second != 0) {
                STTerm t(it->second);
                t.exps = it->first;
                terms.push_back(t);
            }
        }
        std::sort(terms.begin(), terms.end(), [](const STTerm& a, const STTerm& b) {
            int sa = 0, sb = 0;
            for (auto& p : a.exps) sa += p.second;
            for (auto& p : b.exps) sb += p.second;
            if (sa != sb) return sa > sb;
            return a.exps > b.exps;
            });
    }

    std::string ToString() const {
        if (terms.empty()) return "0";
        std::string s;
        for (size_t i = 0; i < terms.size(); ++i) {
            std::string ts = terms[i].ToString();
            if (i == 0) {
                s += ts;
            }
            else {
                if (!ts.empty() && ts[0] == '-') {
                    s += " - " + ts.substr(1);
                }
                else {
                    s += " + " + ts;
                }
            }
        }
        return s;
    }
};

enum STTokenType { ST_TOK_NUM, ST_TOK_VAR, ST_TOK_OP, ST_TOK_LPAREN, ST_TOK_RPAREN, ST_TOK_END };

struct STToken {
    STTokenType type;
    long long num;
    char ch;
};

inline std::vector<STToken> STTokenize(const std::string& s) {
    std::vector<STToken> tokens;
    size_t i = 0;
    while (i < s.size()) {
        char c = s[i];
        if (std::isspace((unsigned char)c)) { i++; continue; }

        if (std::isdigit((unsigned char)c)) {
            long long n = 0;
            while (i < s.size() && std::isdigit((unsigned char)s[i])) {
                n = n * 10 + (s[i] - '0');
                i++;
            }
            STToken t;
            t.type = ST_TOK_NUM;
            t.num = n;
            t.ch = 0;
            tokens.push_back(t);
            continue;
        }

        if (std::isalpha((unsigned char)c)) {
            STToken t;
            t.type = ST_TOK_VAR;
            t.num = 0;
            t.ch = c;
            tokens.push_back(t);
            i++;
            continue;
        }

        if (c == '+' || c == '-' || c == '*' || c == '/' || c == '^') {
            STToken t;
            t.type = ST_TOK_OP;
            t.num = 0;
            t.ch = c;
            tokens.push_back(t);
            i++;
            continue;
        }

        if (c == '(') {
            STToken t;
            t.type = ST_TOK_LPAREN;
            t.num = 0;
            t.ch = 0;
            tokens.push_back(t);
            i++;
            continue;
        }
        if (c == ')') {
            STToken t;
            t.type = ST_TOK_RPAREN;
            t.num = 0;
            t.ch = 0;
            tokens.push_back(t);
            i++;
            continue;
        }

        throw std::runtime_error(std::string("illegal char: ") + c);
    }
    STToken endTok;
    endTok.type = ST_TOK_END;
    endTok.num = 0;
    endTok.ch = 0;
    tokens.push_back(endTok);
    return tokens;
}

inline std::string STInsertImplicitMultiply(const std::string& s) {
    std::string result;
    for (size_t i = 0; i < s.size(); ++i) {
        char cur = s[i];
        if (i > 0) {
            char prev = s[i - 1];
            bool prevIsOperand = std::isalnum((unsigned char)prev) || prev == ')';
            bool curIsOperand = std::isalpha((unsigned char)cur) || cur == '(';
            if (prevIsOperand && curIsOperand) {
                result += '*';
            }
        }
        result += cur;
    }
    return result;
}

class STParser {
    std::vector<STToken> tokens;
    size_t pos;

    STToken Peek() { return tokens[pos]; }
    STToken Next() { return tokens[pos++]; }

public:
    STParser(const std::vector<STToken>& t) : tokens(t), pos(0) {}

    STPoly ParseExpr() {
        STPoly result = ParseTerm();
        while (Peek().type == ST_TOK_OP && (Peek().ch == '+' || Peek().ch == '-')) {
            char op = Next().ch;
            STPoly rhs = ParseTerm();
            if (op == '+') {
                for (size_t i = 0; i < rhs.terms.size(); ++i) {
                    result.terms.push_back(rhs.terms[i]);
                }
            }
            else {
                for (size_t i = 0; i < rhs.terms.size(); ++i) {
                    STTerm t = rhs.terms[i];
                    t.coeff = -t.coeff;
                    result.terms.push_back(t);
                }
            }
        }
        result.Combine();
        return result;
    }

    STPoly ParseTerm() {
        STPoly result = ParseFactor();
        while (Peek().type == ST_TOK_OP && (Peek().ch == '*' || Peek().ch == '/')) {
            char op = Next().ch;
            STPoly rhs = ParseFactor();
            if (op == '*') {
                result = Multiply(result, rhs);
            }
            else {
                if (rhs.terms.size() != 1) {
                    throw std::runtime_error("division only supports monomial divisor");
                }
                result = Divide(result, rhs.terms[0]);
            }
        }
        return result;
    }

    STPoly ParseFactor() {
        STPoly base = ParseBase();
        if (Peek().type == ST_TOK_OP && Peek().ch == '^') {
            Next();
            if (Peek().type != ST_TOK_NUM) {
                throw std::runtime_error("exponent must be integer");
            }
            int exp = (int)Next().num;
            return Power(base, exp);
        }
        return base;
    }

    STPoly ParseBase() {
        STToken t = Peek();
        if (t.type == ST_TOK_NUM) {
            Next();
            STPoly p;
            p.terms.push_back(STTerm(t.num));
            return p;
        }
        if (t.type == ST_TOK_VAR) {
            Next();
            STPoly p;
            STTerm term(1);
            term.exps[t.ch] = 1;
            p.terms.push_back(term);
            return p;
        }
        if (t.type == ST_TOK_LPAREN) {
            Next();
            STPoly p = ParseExpr();
            if (Peek().type != ST_TOK_RPAREN) {
                throw std::runtime_error("unmatched paren");
            }
            Next();
            return p;
        }
        throw std::runtime_error("syntax error");
    }

    static STPoly Multiply(const STPoly& a, const STPoly& b) {
        STPoly result;
        for (size_t i = 0; i < a.terms.size(); ++i) {
            for (size_t j = 0; j < b.terms.size(); ++j) {
                STTerm t;
                t.coeff = a.terms[i].coeff * b.terms[j].coeff;
                t.exps = a.terms[i].exps;
                for (auto it = b.terms[j].exps.begin(); it != b.terms[j].exps.end(); ++it) {
                    t.exps[it->first] += it->second;
                }
                result.terms.push_back(t);
            }
        }
        result.Combine();
        return result;
    }

    static STPoly Divide(const STPoly& a, const STTerm& divisor) {
        if (divisor.coeff == 0) throw std::runtime_error("divide by zero");
        STPoly result;
        for (size_t i = 0; i < a.terms.size(); ++i) {
            STTerm t;
            t.coeff = a.terms[i].coeff / divisor.coeff;
            t.exps = a.terms[i].exps;
            for (auto it = divisor.exps.begin(); it != divisor.exps.end(); ++it) {
                t.exps[it->first] -= it->second;
                if (t.exps[it->first] == 0) t.exps.erase(it->first);
            }
            result.terms.push_back(t);
        }
        result.Combine();
        return result;
    }

    static STPoly Power(const STPoly& base, int exp) {
        if (exp < 0) throw std::runtime_error("negative exponent not supported");
        STPoly result;
        result.terms.push_back(STTerm(1));
        for (int i = 0; i < exp; ++i) {
            result = Multiply(result, base);
        }
        return result;
    }
};

class STFactorizer {
public:
    static std::string Factorize(const STPoly& poly) {
        if (poly.terms.empty()) return "0";

        std::set<char> vars;
        for (size_t i = 0; i < poly.terms.size(); ++i) {
            for (auto it = poly.terms[i].exps.begin(); it != poly.terms[i].exps.end(); ++it) {
                vars.insert(it->first);
            }
        }

        if (vars.size() > 1) {
            return "[multivariate factorization not supported]\n" + poly.ToString();
        }

        if (vars.empty()) {
            return std::to_string(poly.terms[0].coeff);
        }

        char var = *vars.begin();

        STPoly gcdPoly = ExtractGCD(poly);
        if (gcdPoly.terms.size() == 1) {
            bool isTrivial = (gcdPoly.terms[0].coeff == 1 && gcdPoly.terms[0].exps.empty());
            if (!isTrivial) {
                STPoly inner = STParser::Divide(poly, gcdPoly.terms[0]);
                if (inner.terms.size() == 1) {
                    return gcdPoly.ToString() + " * " + inner.ToString();
                }
                std::string innerStr = Factorize(inner);
                return gcdPoly.ToString() + "(" + innerStr + ")";
            }
        }

        int maxDeg = 0;
        for (size_t i = 0; i < poly.terms.size(); ++i) {
            auto it = poly.terms[i].exps.find(var);
            if (it != poly.terms[i].exps.end()) {
                if (it->second > maxDeg) maxDeg = it->second;
            }
        }

        if (maxDeg <= 1) {
            return poly.ToString();
        }
        if (maxDeg == 2) {
            return FactorizeQuadratic(poly, var);
        }
        if (maxDeg == 3) {
            return FactorizeCubic(poly, var);
        }

        return "[degree too high]\n" + poly.ToString();
    }

private:
    static STPoly ExtractGCD(const STPoly& poly) {
        if (poly.terms.empty()) return STPoly();

        long long g = std::abs(poly.terms[0].coeff);
        for (size_t i = 1; i < poly.terms.size(); ++i) {
            g = std::gcd(g, std::abs(poly.terms[i].coeff));
        }
        if (g == 0) g = 1;

        std::map<char, int> minExp;
        for (auto it = poly.terms[0].exps.begin(); it != poly.terms[0].exps.end(); ++it) {
            minExp[it->first] = it->second;
        }
        for (size_t i = 1; i < poly.terms.size(); ++i) {
            for (auto it = minExp.begin(); it != minExp.end(); ++it) {
                auto f = poly.terms[i].exps.find(it->first);
                if (f == poly.terms[i].exps.end()) {
                    it->second = 0;
                }
                else {
                    if (f->second < it->second) it->second = f->second;
                }
            }
        }

        STPoly gcdPoly;
        STTerm gTerm(g);
        for (auto it = minExp.begin(); it != minExp.end(); ++it) {
            if (it->second > 0) gTerm.exps[it->first] = it->second;
        }
        gcdPoly.terms.push_back(gTerm);
        return gcdPoly;
    }

    static std::string FormatLinearFactor(long long d, char var, long long n) {
        std::string s;
        if (d == 1) s = std::string(1, var);
        else if (d == -1) s = "-" + std::string(1, var);
        else s = std::to_string(d) + var;
        if (n > 0) s += " + " + std::to_string(n);
        else if (n < 0) s += " - " + std::to_string(-n);
        return s;
    }

    static std::string FactorizeQuadratic(const STPoly& poly, char var) {
        long long a = 0, b = 0, c = 0;
        for (size_t i = 0; i < poly.terms.size(); ++i) {
            int deg = 0;
            auto it = poly.terms[i].exps.find(var);
            if (it != poly.terms[i].exps.end()) deg = it->second;
            if (deg == 2) a = poly.terms[i].coeff;
            else if (deg == 1) b = poly.terms[i].coeff;
            else if (deg == 0) c = poly.terms[i].coeff;
        }

        if (a > 0 && c > 0) {
            long long pa = (long long)std::round(std::sqrt((double)a));
            long long pc = (long long)std::round(std::sqrt((double)c));
            if (pa * pa == a && pc * pc == c) {
                if (2 * pa * pc == b) {
                    return "(" + FormatLinearFactor(pa, var, pc) + ")^2";
                }
                if (2 * pa * pc == -b) {
                    return "(" + FormatLinearFactor(pa, var, -pc) + ")^2";
                }
            }
        }

        if (a > 0 && b == 0 && c < 0) {
            long long pa = (long long)std::round(std::sqrt((double)a));
            long long pc = (long long)std::round(std::sqrt((double)-c));
            if (pa * pa == a && pc * pc == -c) {
                return "(" + FormatLinearFactor(pa, var, pc) + ")("
                    + FormatLinearFactor(pa, var, -pc) + ")";
            }
        }

        long long disc = b * b - 4 * a * c;
        if (disc >= 0) {
            long long sd = (long long)std::round(std::sqrt((double)disc));
            if (sd * sd == disc) {
                long long n1 = -b + sd;
                long long n2 = -b - sd;
                long long d1 = 2 * a;
                long long d2 = 2 * a;
                long long gg1 = std::gcd(std::abs(n1), std::abs(d1));
                long long gg2 = std::gcd(std::abs(n2), std::abs(d2));
                if (gg1 == 0) gg1 = 1;
                if (gg2 == 0) gg2 = 1;
                n1 /= gg1; d1 /= gg1;
                n2 /= gg2; d2 /= gg2;
                if (d1 < 0) { n1 = -n1; d1 = -d1; }
                if (d2 < 0) { n2 = -n2; d2 = -d2; }

                std::string f1 = FormatLinearFactor(d1, var, n1);
                std::string f2 = FormatLinearFactor(d2, var, n2);
                if (f1 == f2) {
                    return "(" + f1 + ")^2";
                }
                return "(" + f1 + ")(" + f2 + ")";
            }
        }

        return poly.ToString();
    }

    static std::string FactorizeCubic(const STPoly& poly, char var) {
        long long a = 0, b = 0, c = 0, d = 0;
        for (size_t i = 0; i < poly.terms.size(); ++i) {
            int deg = 0;
            auto it = poly.terms[i].exps.find(var);
            if (it != poly.terms[i].exps.end()) deg = it->second;
            if (deg == 3) a = poly.terms[i].coeff;
            else if (deg == 2) b = poly.terms[i].coeff;
            else if (deg == 1) c = poly.terms[i].coeff;
            else if (deg == 0) d = poly.terms[i].coeff;
        }

        std::vector<long long> candidates;
        if (d == 0) {
            candidates.push_back(0);
        }
        else {
            for (long long i = 1; i <= std::abs(d); ++i) {
                if (d % i == 0) {
                    candidates.push_back(i);
                    candidates.push_back(-i);
                }
            }
        }

        for (size_t k = 0; k < candidates.size(); ++k) {
            long long r = candidates[k];
            long long val = a * r * r * r + b * r * r + c * r + d;
            if (val == 0) {
                long long A = a;
                long long B = b + A * r;
                long long C = c + B * r;

                STPoly quad;
                if (A != 0) {
                    STTerm t2; t2.coeff = A; t2.exps[var] = 2; quad.terms.push_back(t2);
                }
                if (B != 0) {
                    STTerm t1; t1.coeff = B; t1.exps[var] = 1; quad.terms.push_back(t1);
                }
                if (C != 0) {
                    STTerm t0; t0.coeff = C; quad.terms.push_back(t0);
                }
                quad.Combine();

                std::string factor1;
                if (r > 0) factor1 = "(x - " + std::to_string(r) + ")";
                else if (r < 0) factor1 = "(x + " + std::to_string(-r) + ")";
                else factor1 = "(x)";

                if (quad.terms.empty()) return factor1;
                std::string factor2 = Factorize(quad);
                return factor1 + factor2;
            }
        }

        return poly.ToString();
    }
};

inline STPoly STParseExpression(const std::string& expr) {
    std::string clean;
    for (size_t i = 0; i < expr.size(); ++i) {
        if (!std::isspace((unsigned char)expr[i])) clean += expr[i];
    }
    clean = STInsertImplicitMultiply(clean);
    std::vector<STToken> tokens = STTokenize(clean);
    STParser parser(tokens);
    return parser.ParseExpr();
}

inline std::string STExpand(const std::string& expr) {
    try {
        STPoly poly = STParseExpression(expr);
        return poly.ToString();
    }
    catch (const std::exception& e) {
        return std::string("error: ") + e.what();
    }
}

inline std::string STFactor(const std::string& expr) {
    try {
        STPoly poly = STParseExpression(expr);
        return STFactorizer::Factorize(poly);
    }
    catch (const std::exception& e) {
        return std::string("error: ") + e.what();
    }
}

inline std::string STNumCommand(const std::string& mode, const std::string& expr) {
    try {
        STPoly poly = STParseExpression(expr);
        if (mode == "/1") {
            return poly.ToString();
        }
        else if (mode == "/2") {
            return STFactorizer::Factorize(poly);
        }
        return "error: mode must be /1 or /2";
    }
    catch (const std::exception& e) {
        return std::string("error: ") + e.what();
    }
}