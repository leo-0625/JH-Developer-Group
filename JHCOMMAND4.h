#pragma once

#include <Windows.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <vector>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <stack>
#include <cctype>
#pragma comment(lib, "psapi.lib")

class BigNum {
public:
    std::vector<int> digits;

    int scale = 0;
    bool negative = false;

    BigNum() { digits.push_back(0); }

    explicit BigNum(const std::string& s) {
        int i = 0;
        if (i < (int)s.size() && (s[i] == '+' || s[i] == '-')) {
            negative = (s[i] == '-');
            i++;
        }
        int dotPos = -1;
        for (int j = i; j < (int)s.size(); j++) {
            if (s[j] == '.') { dotPos = j; break; }
        }
        std::string intPart, fracPart;
        if (dotPos == -1) {
            intPart = s.substr(i);
        }
        else {
            intPart = s.substr(i, dotPos - i);
            fracPart = s.substr(dotPos + 1);
            scale = (int)fracPart.size();
        }

        while (intPart.size() > 1 && intPart[0] == '0') intPart.erase(0, 1);
        if (intPart.empty()) intPart = "0";

        for (int j = (int)fracPart.size() - 1; j >= 0; j--) digits.push_back(fracPart[j] - '0');
        for (int j = (int)intPart.size() - 1; j >= 0; j--) digits.push_back(intPart[j] - '0');
        trim();
    }

    void trim() {
        while (scale > 0 && !digits.empty() && digits[0] == 0) {
            digits.erase(digits.begin());
            scale--;
        }
        while (digits.size() > 1 && digits.back() == 0) digits.pop_back();
        if (digits.size() == 1 && digits[0] == 0) negative = false;
    }

    static int cmpAbs(const BigNum& a, const BigNum& b) {
        int maxScale = (std::max)(a.scale, b.scale);
        std::vector<int> A = a.digits, B = b.digits;
        while ((int)A.size() < (int)B.size() + (maxScale - a.scale)) A.push_back(0);
        while ((int)B.size() < (int)A.size() + (maxScale - b.scale)) B.push_back(0);
        int len = (std::max)(A.size(), B.size());
        A.resize(len, 0); B.resize(len, 0);
        for (int i = len - 1; i >= 0; i--) {
            if (A[i] != B[i]) return A[i] > B[i] ? 1 : -1;
        }
        return 0;
    }

    static BigNum addAbs(const BigNum& a, const BigNum& b) {
        BigNum r;
        r.scale = (std::max)(a.scale, b.scale);
        int len = (std::max)(a.digits.size(), b.digits.size()) + 1;
        r.digits.assign(len, 0);
        for (int i = 0; i < (int)a.digits.size(); i++) {
            int pos = i + (r.scale - a.scale);
            if (pos < len) r.digits[pos] += a.digits[i];
        }
        for (int i = 0; i < (int)b.digits.size(); i++) {
            int pos = i + (r.scale - b.scale);
            if (pos < len) r.digits[pos] += b.digits[i];
        }
        int carry = 0;
        for (int i = 0; i < len; i++) {
            int sum = r.digits[i] + carry;
            r.digits[i] = sum % 10;
            carry = sum / 10;
        }
        while (carry) { r.digits.push_back(carry % 10); carry /= 10; }
        r.trim();
        return r;
    }

    static BigNum subAbs(const BigNum& a, const BigNum& b) {
        BigNum r;
        r.scale = (std::max)(a.scale, b.scale);
        int len = (std::max)(a.digits.size(), b.digits.size()) + 1;
        r.digits.assign(len, 0);
        for (int i = 0; i < (int)a.digits.size(); i++) {
            int pos = i + (r.scale - a.scale);
            r.digits[pos] += a.digits[i];
        }
        for (int i = 0; i < (int)b.digits.size(); i++) {
            int pos = i + (r.scale - b.scale);
            r.digits[pos] -= b.digits[i];
        }
        for (int i = 0; i < len - 1; i++) {
            if (r.digits[i] < 0) { r.digits[i] += 10; r.digits[i + 1] -= 1; }
        }
        r.trim();
        return r;
    }

    static BigNum mulAbs(const BigNum& a, const BigNum& b) {
        BigNum r;
        r.scale = a.scale + b.scale;
        r.digits.assign(a.digits.size() + b.digits.size(), 0);
        for (int i = 0; i < (int)a.digits.size(); i++) {
            int carry = 0;
            for (int j = 0; j < (int)b.digits.size() || carry; j++) {
                int sum = r.digits[i + j] + a.digits[i] * (j < (int)b.digits.size() ? b.digits[j] : 0) + carry;
                r.digits[i + j] = sum % 10;
                carry = sum / 10;
            }
        }
        r.trim();
        return r;
    }

    BigNum operator+(const BigNum& o) const {
        if (!negative && !o.negative) return addAbs(*this, o);
        if (negative && o.negative) { BigNum r = addAbs(*this, o); r.negative = true; return r; }
        int c = cmpAbs(*this, o);
        if (c == 0) return BigNum("0");
        if (c > 0) { BigNum r = subAbs(*this, o); r.negative = negative; return r; }
        else { BigNum r = subAbs(o, *this); r.negative = o.negative; return r; }
    }

    BigNum operator-(const BigNum& o) const {
        BigNum t = o; t.negative = !o.negative; return *this + t;
    }

    BigNum operator*(const BigNum& o) const {
        BigNum r = mulAbs(*this, o);
        r.negative = (negative != o.negative);
        return r;
    }
};

BigNum powBig(const BigNum& base, long long exp) {
    BigNum r("1");
    BigNum b = base;
    while (exp > 0) {
        if (exp & 1) r = r * b;
        b = b * b;
        exp >>= 1;
    }
    return r;
}

BigNum divBig(const BigNum& a, const BigNum& b, int scaleDigits = 10) {
    if (b.digits.size() == 1 && b.digits[0] == 0) throw std::runtime_error("除数为 0");
    BigNum A = a, B = b;
    int aScale = A.scale, bScale = B.scale;
    int shift = scaleDigits + bScale - aScale;
    if (shift > 0) {
        for (int i = 0; i < shift; i++) A.digits.insert(A.digits.begin(), 0);
    }
    else {
        for (int i = 0; i < -shift; i++) B.digits.insert(B.digits.begin(), 0);
    }
    A.scale = B.scale = 0;
    std::vector<int> q;
    BigNum rem("0");
    BigNum ten("10");
    for (int i = (int)A.digits.size() - 1; i >= 0; i--) {
        rem.digits.insert(rem.digits.begin(), A.digits[i]);
        rem.trim();
        int d = 0;
        while (BigNum::cmpAbs(rem, B) >= 0) { rem = BigNum::subAbs(rem, B); d++; }
        q.push_back(d);
    }
    BigNum res;
    res.digits.clear();
    for (int i = (int)q.size() - 1; i >= 0; i--) res.digits.push_back(q[i]);
    if (res.digits.empty()) res.digits.push_back(0);
    res.scale = scaleDigits;
    res.negative = (a.negative != b.negative);
    res.trim();
    return res;
}

std::string toString(const BigNum& n) {
    if (n.digits.size() == 1 && n.digits[0] == 0) return "0";
    std::string s;
    if (n.negative) s += '-';
    int intLen = (int)n.digits.size() - n.scale;
    if (intLen <= 0) {
        s += "0.";
        for (int i = 0; i < -intLen; i++) s += '0';
        for (int i = (int)n.digits.size() - 1; i >= 0; i--) s += char('0' + n.digits[i]);
    }
    else {
        for (int i = (int)n.digits.size() - 1; i >= n.scale; i--) s += char('0' + n.digits[i]);
        if (n.scale > 0) {
            s += '.';
            for (int i = n.scale - 1; i >= 0; i--) s += char('0' + n.digits[i]);
        }
    }
    if (s.find('.') != std::string::npos) {
        while (!s.empty() && s.back() == '0') s.pop_back();
        if (!s.empty() && s.back() == '.') s.pop_back();
    }
    return s;
}

int priority(char op) {
    if (op == '^') return 3;
    if (op == '*' || op == '/') return 2;
    if (op == '+' || op == '-') return 1;
    return 0;
}

bool isRightAssoc(char op) { return op == '^'; }

std::vector<std::string> toRPN(const std::string& expr) {
    std::vector<std::string> output;
    std::stack<char> ops;
    int i = 0, n = expr.size();
    while (i < n) {
        char c = expr[i];
        if (isspace(c)) { i++; continue; }
        if (isdigit(c) || c == '.') {
            std::string num;
            while (i < n && (isdigit(expr[i]) || expr[i] == '.')) num += expr[i++];
            output.push_back(num);
            continue;
        }
        if (c == '(') { ops.push(c); i++; continue; }
        if (c == ')') {
            while (!ops.empty() && ops.top() != '(') {
                output.push_back(std::string(1, ops.top())); ops.pop();
            }
            if (ops.empty()) throw std::runtime_error("括号不匹配");
            ops.pop(); i++; continue;
        }
        if (c == '+' || c == '-' || c == '*' || c == '/' || c == '^') {

            if (c == '-' && (i == 0 || expr[i - 1] == '(' || priority(expr[i - 1]) > 0)) {
                output.push_back("0");
                ops.push('-');
                i++; continue;
            }
            while (!ops.empty() && ops.top() != '(' &&
                (priority(ops.top()) > priority(c) ||
                    (priority(ops.top()) == priority(c) && !isRightAssoc(c)))) {
                output.push_back(std::string(1, ops.top())); ops.pop();
            }
            ops.push(c); i++; continue;
        }
        throw std::runtime_error(std::string("非法字符: ") + c);
    }
    while (!ops.empty()) {
        if (ops.top() == '(') throw std::runtime_error("括号不匹配");
        output.push_back(std::string(1, ops.top())); ops.pop();
    }
    return output;
}

BigNum evalRPN(const std::vector<std::string>& rpn) {
    std::stack<BigNum> st;
    for (const auto& tok : rpn) {
        if (tok == "+" || tok == "-" || tok == "*" || tok == "/" || tok == "^") {
            if (st.size() < 2) throw std::runtime_error("表达式错误");
            BigNum b = st.top(); st.pop();
            BigNum a = st.top(); st.pop();
            if (tok == "+") st.push(a + b);
            else if (tok == "-") st.push(a - b);
            else if (tok == "*") st.push(a * b);
            else if (tok == "/") st.push(divBig(a, b));
            else if (tok == "^") {
                if (b.scale != 0 || b.negative) throw std::runtime_error("乘方指数必须是非负整数");
                long long e = 0;
                for (int i = (int)b.digits.size() - 1; i >= 0; i--) e = e * 10 + b.digits[i];
                st.push(powBig(a, e));
            }
        }
        else {
            st.push(BigNum(tok));
        }
    }
    if (st.size() != 1) throw std::runtime_error("表达式错误");
    return st.top();
}

BOOL WINAPI OptPerformFullOptimization(VOID);
BOOL WINAPI OptStopAutoService(VOID);

#define OPTIMIZER_WORKING_SET_THRESHOLD   (100 * 1024 * 1024)
#define OPTIMIZER_OPTIMIZATION_INTERVAL   30000

typedef struct _OPTIMIZER_STATISTICS {
    DWORD dwMemoryTrims;
    DWORD dwZombiesCleaned;
    DWORD dwTotalOptimizations;
    DWORD dwLastError;
} OPTIMIZER_STATISTICS, * POPTIMIZER_STATISTICS;

static HANDLE g_hServiceThread = NULL;
static HANDLE g_hStopEvent = NULL;
static OPTIMIZER_STATISTICS g_Statistics = { 0 };
static CRITICAL_SECTION g_csStats;
static BOOL g_bInitialized = FALSE;

static BOOL EnableDebugPrivilege(VOID) {
    HANDLE hToken;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
        return FALSE;

    TOKEN_PRIVILEGES tp;
    LUID luid;

    if (!LookupPrivilegeValueW(NULL, SE_DEBUG_NAME, &luid)) {
        CloseHandle(hToken);
        return FALSE;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    BOOL result = AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);
    CloseHandle(hToken);
    return result && GetLastError() == ERROR_SUCCESS;
}

static DWORD WINAPI OptimizationServiceThread(LPVOID lpParam) {
    while (WaitForSingleObject(g_hStopEvent, OPTIMIZER_OPTIMIZATION_INTERVAL) == WAIT_TIMEOUT) {
        OptPerformFullOptimization();
    }
    return 0;
}

BOOL WINAPI OptInitialize(VOID) {
    if (g_bInitialized) return TRUE;

    EnableDebugPrivilege();

    InitializeCriticalSection(&g_csStats);
    ZeroMemory(&g_Statistics, sizeof(OPTIMIZER_STATISTICS));
    g_hStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    g_bInitialized = TRUE;

    return TRUE;
}

VOID WINAPI OptUninitialize(VOID) {
    if (!g_bInitialized) return;

    OptStopAutoService();

    if (g_hStopEvent) {
        CloseHandle(g_hStopEvent);
        g_hStopEvent = NULL;
    }

    DeleteCriticalSection(&g_csStats);
    g_bInitialized = FALSE;
}

BOOL WINAPI OptTrimWorkingSets(VOID) {
    if (!g_bInitialized) return FALSE;

    SetProcessWorkingSetSize(GetCurrentProcess(), -1, -1);

    EnterCriticalSection(&g_csStats);
    g_Statistics.dwMemoryTrims++;
    g_Statistics.dwTotalOptimizations++;
    LeaveCriticalSection(&g_csStats);

    return TRUE;
}

DWORD WINAPI OptCleanupZombieProcesses(VOID) {
    if (!g_bInitialized) return 0;

    DWORD cleaned = 0;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return 0;

    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);

    if (Process32FirstW(hSnapshot, &pe32)) {
        do {
            if (pe32.th32ProcessID == 0 || pe32.th32ProcessID == 4 || pe32.th32ProcessID == GetCurrentProcessId())
                continue;

            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_TERMINATE,
                FALSE, pe32.th32ProcessID);
            if (hProcess) {
                DWORD exitCode;
                if (GetExitCodeProcess(hProcess, &exitCode) && exitCode != STILL_ACTIVE) {
                    TerminateProcess(hProcess, 0);
                    cleaned++;
                }
                CloseHandle(hProcess);
            }
        } while (Process32NextW(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);

    EnterCriticalSection(&g_csStats);
    g_Statistics.dwZombiesCleaned += cleaned;
    g_Statistics.dwTotalOptimizations++;
    LeaveCriticalSection(&g_csStats);

    return cleaned;
}

BOOL WINAPI OptPerformFullOptimization(VOID) {
    if (!g_bInitialized) { std::cout << "ERROR!"; return FALSE; }

    OptTrimWorkingSets();
    std::cout << "\n已完成50%";
    OptCleanupZombieProcesses();
    std::cout << "\r已完成99%";
    Sleep(130);
    std::cout << "\r已完成100%";

    return TRUE;
}

BOOL WINAPI OptGetStatistics(POPTIMIZER_STATISTICS lpStatistics) {
    if (!g_bInitialized || !lpStatistics) return FALSE;

    EnterCriticalSection(&g_csStats);
    CopyMemory(lpStatistics, &g_Statistics, sizeof(OPTIMIZER_STATISTICS));
    LeaveCriticalSection(&g_csStats);

    return TRUE;
}

BOOL WINAPI OptStartAutoService(VOID) {
    if (!g_bInitialized) return FALSE;
    if (g_hServiceThread) return TRUE;

    if (g_hStopEvent)
        ResetEvent(g_hStopEvent);

    g_hServiceThread = CreateThread(NULL, 0, OptimizationServiceThread, NULL, 0, NULL);
    return (g_hServiceThread != NULL);
}

BOOL WINAPI OptStopAutoService(VOID) {
    if (!g_hServiceThread) return FALSE;

    if (g_hStopEvent)
        SetEvent(g_hStopEvent);

    WaitForSingleObject(g_hServiceThread, 5000);
    CloseHandle(g_hServiceThread);
    g_hServiceThread = NULL;

    return TRUE;
}

DWORD WINAPI OptGetMemoryPressure(VOID) {
    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(MEMORYSTATUSEX);

    if (GlobalMemoryStatusEx(&memStatus))
        return memStatus.dwMemoryLoad;

    return 0;
}