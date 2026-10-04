#pragma once

#include <functional>
#include <shlobj.h> 
#include <iphlpapi.h>
#include <shlwapi.h>
#include <tchar.h>
#include <stack>
#include <map>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <audiopolicy.h>
#include <wrl/client.h>
#include <windows.ui.notifications.h>
#include <sapi.h>
#include <gdiplus.h>
#include <sstream>
#include <powrprof.h>
#include <cfgmgr32.h>
#include <ntddstor.h>
#include "JHCOMMAND5.h"
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "powrprof.lib")

bool rainbow = false;
DWORD g_lastErrorCode = 0;
bool g_startFailed = false;
bool g_echoState = true;

bool IsRunningAsAdmin() {
    BOOL isAdmin = FALSE;
    SID_IDENTIFIER_AUTHORITY ntAuth = SECURITY_NT_AUTHORITY;
    PSID adminSid = nullptr;

    if (!AllocateAndInitializeSid(&ntAuth, 2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0,
        &adminSid))
    {
        return false;
    }
    if (!CheckTokenMembership(nullptr, adminSid, &isAdmin)) {
        FreeSid(adminSid);
        return false;
    }

    FreeSid(adminSid);
    return isAdmin == TRUE;
}

using namespace Gdiplus;
#pragma comment(lib, "sapi.lib")

#pragma comment(lib, "runtimeobject.lib")
std::string ExpandEnvironmentVars(const std::string& input);

using namespace Microsoft::WRL;
using namespace ABI::Windows::UI::Notifications;
using namespace ABI::Windows::Data::Xml::Dom;
using namespace Windows::Foundation;

auto files = GetZjhCmdFiles();

bool g_redirectToNul;
std::string g_redirectFile;
char g_redirectMode;

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "winmm.lib")

#include "JHCOMMAND1.h"
#include "JHCOMMAND4.h"

long long copiedFiles = 0, skippedFiles = 0, failedFiles = 0;
long long copiedBytes = 0;
DWORD startTime = GetTickCount();
bool continueOnError = false;
static std::vector<std::string> g_commandHistory;
static const size_t MAX_HISTORY = 50;

bool CheckCommandExists(const std::string& cmdName) {
    std::string searchName = cmdName;
    bool hasExeExt = false;

    std::string lowerName = cmdName;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
    if (lowerName.size() >= 4 && lowerName.substr(lowerName.size() - 4) == ".exe") {
        hasExeExt = true;
        searchName = cmdName.substr(0, cmdName.size() - 4);
    }

    if (hasExeExt) {
        char curDir[MAX_PATH];
        GetCurrentDirectoryA(MAX_PATH, curDir);
        std::string testPath = std::string(curDir) + "\\" + cmdName;
        DWORD attrs = GetFileAttributesA(testPath.c_str());
        if (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY)) {
            return true;
        }

        char* pathEnv = nullptr;
        size_t pathLen = 0;
        _dupenv_s(&pathEnv, &pathLen, "PATH");
        if (pathEnv && pathLen > 0) {
            std::string pathStr = pathEnv;
            free(pathEnv);
            size_t start = 0;
            while (start < pathStr.size()) {
                size_t end = pathStr.find(';', start);
                if (end == std::string::npos) end = pathStr.size();
                std::string dir = pathStr.substr(start, end - start);
                if (dir.size() >= 2 && dir.front() == '"' && dir.back() == '"') {
                    dir = dir.substr(1, dir.size() - 2);
                }
                if (!dir.empty()) {
                    testPath = dir + "\\" + cmdName;
                    attrs = GetFileAttributesA(testPath.c_str());
                    if (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY)) {
                        return true;
                    }
                }
                start = end + 1;
            }
        }

        char sysDir[MAX_PATH];
        GetSystemDirectoryA(sysDir, MAX_PATH);
        testPath = std::string(sysDir) + "\\" + cmdName;
        attrs = GetFileAttributesA(testPath.c_str());
        if (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY)) {
            return true;
        }

        char winDir[MAX_PATH];
        GetWindowsDirectoryA(winDir, MAX_PATH);
        testPath = std::string(winDir) + "\\" + cmdName;
        attrs = GetFileAttributesA(testPath.c_str());
        if (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY)) {
            return true;
        }

        return false;
    }

    char currentDir[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, currentDir);
    std::string testPath = std::string(currentDir) + "\\" + searchName + ".exe";
    DWORD attrs = GetFileAttributesA(testPath.c_str());
    if (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY)) {
        return true;
    }

    char* pathEnv = nullptr;
    size_t pathLen = 0;
    _dupenv_s(&pathEnv, &pathLen, "PATH");
    if (pathEnv && pathLen > 0) {
        std::string pathStr = pathEnv;
        free(pathEnv);
        size_t start = 0;
        while (start < pathStr.size()) {
            size_t end = pathStr.find(';', start);
            if (end == std::string::npos) end = pathStr.size();
            std::string dir = pathStr.substr(start, end - start);
            if (dir.size() >= 2 && dir.front() == '"' && dir.back() == '"') {
                dir = dir.substr(1, dir.size() - 2);
            }
            if (!dir.empty()) {
                testPath = dir + "\\" + searchName + ".exe";
                attrs = GetFileAttributesA(testPath.c_str());
                if (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY)) {
                    return true;
                }
            }
            start = end + 1;
        }
    }

    char sysDir[MAX_PATH];
    GetSystemDirectoryA(sysDir, MAX_PATH);
    testPath = std::string(sysDir) + "\\" + searchName + ".exe";
    attrs = GetFileAttributesA(testPath.c_str());
    if (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY)) {
        return true;
    }

    char winDir[MAX_PATH];
    GetWindowsDirectoryA(winDir, MAX_PATH);
    testPath = std::string(winDir) + "\\" + searchName + ".exe";
    attrs = GetFileAttributesA(testPath.c_str());
    if (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY)) {
        return true;
    }

    return false;
}

std::string EscapeForVBS(const std::string& input);

bool ExecuteProcess(const std::string& cmdLine) {
    if (cmdLine.empty()) {
        return false;
    }

    std::string cmdLineexe;
    std::string cmdLower = cmdLine;
    std::transform(cmdLower.begin(), cmdLower.end(), cmdLower.begin(), ::tolower);

    if (cmdLower.find(".exe") != std::string::npos) {
        cmdLineexe = cmdLine;
    }
    else {
        size_t spacePos = cmdLine.find(' ');
        if (spacePos != std::string::npos) {
            cmdLineexe = cmdLine.substr(0, spacePos) + ".exe" + cmdLine.substr(spacePos);
        }
        else {
            cmdLineexe = cmdLine + ".exe";
        }
    }
    char* cmdBuf = _strdup(cmdLineexe.c_str());
    if (!cmdBuf) {
        std::cout << "内存分配失败\n";
        return false;
    }

    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi = { 0 };
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

    BOOL ok = CreateProcessA(NULL, cmdBuf, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
    free(cmdBuf);

    if (!ok) {
        DWORD err = GetLastError();
        if (err == ERROR_FILE_NOT_FOUND) {

        }
        return false;
    }

    DWORD waitResult = WaitForSingleObject(pi.hProcess, 30000);  // 30秒超时
    if (waitResult == WAIT_TIMEOUT) {
        TerminateProcess(pi.hProcess, 1);
        std::cout << "命令执行超时\n";
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return true;
}

bool ExecuteSmart(const std::string& cmd, const CommandInfo& info) {
    if (info.command.empty()) return false;

    if (info.hasPipe && !info.pipeCommand.empty()) {
        return ExecuteWithPipe(cmd, info);
    }

    if (info.hasRedirect) {
        return ExecuteWithRedirection(cmd, info);
    }

    std::string expandedCmd = ExpandEnvironmentVars(cmd);
    if (expandedCmd.empty()) return false;

    std::string cmdLower = info.command;
    std::transform(cmdLower.begin(), cmdLower.end(), cmdLower.begin(), ::tolower);

    bool isBatch = (cmdLower.size() > 4 &&
        (cmdLower.substr(cmdLower.size() - 4) == ".bat" ||
            cmdLower.substr(cmdLower.size() - 4) == ".cmd"));

    if (isBatch) {
        std::string fullPath = expandedCmd;
        DWORD attrs = GetFileAttributesA(fullPath.c_str());
        if (attrs != INVALID_FILE_ATTRIBUTES &&
            (attrs & FILE_ATTRIBUTE_DIRECTORY) == 0) {
            if (ExecuteBatchLikeCMD(fullPath, {})) {
                return true;
            }
        }

        return false;
    }

    if (ExecuteProcess(expandedCmd)) {
        return true;
    }

    return false;
}

std::string EscapeForVBS(const std::string& input) {
    std::string result;
    for (char c : input) {
        if (c == '"') result += "\"\"";
        else result += c;
    }
    return result;
}

class CAudioVolumeControl {
private:
    IAudioEndpointVolume* m_pEndpointVolume;
    bool m_bInitialized;

public:
    CAudioVolumeControl() : m_pEndpointVolume(NULL), m_bInitialized(false) {
        HRESULT hr = CoInitialize(NULL);
        if (FAILED(hr)) return;

        IMMDeviceEnumerator* pEnumerator = NULL;
        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL,
            CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
            (void**)&pEnumerator);
        if (FAILED(hr)) {
            CoUninitialize();
            return;
        }

        IMMDevice* pDevice = NULL;
        hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
        pEnumerator->Release();

        if (SUCCEEDED(hr)) {
            hr = pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL,
                NULL, (void**)&m_pEndpointVolume);
            pDevice->Release();
        }

        if (SUCCEEDED(hr) && m_pEndpointVolume) {
            m_bInitialized = true;
        }
        else {
            CoUninitialize();
        }
    }

    ~CAudioVolumeControl() {
        if (m_pEndpointVolume) {
            m_pEndpointVolume->Release();
            m_pEndpointVolume = NULL;
        }
        if (m_bInitialized) {
            CoUninitialize();
            m_bInitialized = false;
        }
    }

    bool IsInitialized() const { return m_bInitialized; }

    float GetVolume() {
        if (!m_pEndpointVolume) return -1.0f;
        float volume = 0;
        m_pEndpointVolume->GetMasterVolumeLevelScalar(&volume);
        return volume;
    }

    bool SetVolume(float volume) {
        if (!m_pEndpointVolume) return false;
        if (volume < 0.0f) volume = 0.0f;
        if (volume > 1.0f) volume = 1.0f;
        HRESULT hr = m_pEndpointVolume->SetMasterVolumeLevelScalar(volume, NULL);
        return SUCCEEDED(hr);
    }

    int GetVolumePercent() {
        return (int)(GetVolume() * 100 + 0.5f);
    }

    bool SetVolumePercent(int percent) {
        if (percent < 0) percent = 0;
        if (percent > 100) percent = 100;
        return SetVolume(percent / 100.0f);
    }

    bool VolumeUp(int delta = 10) {
        int newVol = GetVolumePercent() + delta;
        if (newVol > 100) newVol = 100;
        return SetVolumePercent(newVol);
    }

    bool VolumeDown(int delta = 10) {
        int newVol = GetVolumePercent() - delta;
        if (newVol < 0) newVol = 0;
        return SetVolumePercent(newVol);
    }

    bool GetMute() {
        if (!m_pEndpointVolume) return false;
        BOOL muted = FALSE;
        m_pEndpointVolume->GetMute(&muted);
        return muted == TRUE;
    }

    bool SetMute(bool mute) {
        if (!m_pEndpointVolume) return false;
        HRESULT hr = m_pEndpointVolume->SetMute(mute ? TRUE : FALSE, NULL);
        return SUCCEEDED(hr);
    }

    bool ToggleMute() {
        return SetMute(!GetMute());
    }
};

static std::stack<std::string> g_directoryStack;
static std::vector<std::vector<std::string>> g_batchArgs;
static std::vector<std::map<std::string, std::string>> g_envBackups;

std::string _1t = "海内存知己，天涯若比邻";

#pragma comment(lib, "shlwapi.lib")

std::string extractRootDir(const std::string& rest) {

    size_t rPos = rest.find("/r");
    if (rPos == std::string::npos) return ".";

    std::string afterR = rest.substr(rPos + 2);

    size_t start = afterR.find_first_not_of(" \t");
    if (start == std::string::npos) return ".";

    if (afterR[start] == '"') {
        size_t end = afterR.find('"', start + 1);
        if (end != std::string::npos) {
            return afterR.substr(start + 1, end - start - 1);
        }
    }

    size_t varPos = afterR.find('%', start);
    if (varPos != std::string::npos && varPos > start) {
        std::string dirCandidate = afterR.substr(start, varPos - start);
        size_t trimEnd = dirCandidate.find_last_not_of(" \t");
        if (trimEnd != std::string::npos) {
            dirCandidate = dirCandidate.substr(0, trimEnd + 1);
            if (!dirCandidate.empty() && dirCandidate[0] != '%') {
                return dirCandidate;
            }
        }
    }

    return ".";
}

std::string extractCommand(const std::string& rest) {
    size_t doPos = rest.find("do");
    if (doPos == std::string::npos) return "";

    std::string command = rest.substr(doPos + 2);
    size_t start = command.find_first_not_of(" \t");
    if (start != std::string::npos) {
        command = command.substr(start);
    }

    size_t end = command.find_last_not_of(" \t\r\n");
    if (end != std::string::npos) {
        command = command.substr(0, end + 1);
    }

    return command;
}

void executeForCommand(const std::string& command, const std::string& currentItem, char varName) {
    std::string expandedCmd = command;

    std::string searchStr = std::string("%") + varName;
    size_t pos = 0;
    while ((pos = expandedCmd.find(searchStr, pos)) != std::string::npos) {
        expandedCmd.replace(pos, searchStr.length(), currentItem);
        pos += currentItem.length();
    }

    if (HandleBuiltinCommand(expandedCmd, false)) {
        return;
    }

    CommandInfo info = ParseCommand(expandedCmd);
    ExecuteSmart(expandedCmd, info);
}

std::string ExtractForRootDir(const std::string& rest) {
    size_t start = rest.find('(');
    if (start == std::string::npos) return ".";

    std::string afterR = rest.substr(2);
    size_t dirStart = afterR.find_first_not_of(" \t");

    if (dirStart != std::string::npos && afterR[dirStart] == '"') {
        size_t dirEnd = afterR.find('"', dirStart + 1);
        if (dirEnd != std::string::npos) {
            return afterR.substr(dirStart + 1, dirEnd - dirStart - 1);
        }
    }

    size_t varPos = afterR.find('%');
    if (varPos != std::string::npos && varPos > 0) {
        std::string beforeVar = afterR.substr(0, varPos);
        size_t lastSpace = beforeVar.find_last_not_of(" \t");
        if (lastSpace != std::string::npos) {
            size_t firstSpace = beforeVar.find_first_not_of(" \t");
            if (firstSpace != std::string::npos) {
                std::string dirCandidate = beforeVar.substr(firstSpace, lastSpace - firstSpace + 1);
                if (!dirCandidate.empty() && dirCandidate[0] != '%') {
                    return dirCandidate;
                }
            }
        }
    }

    return ".";
}

std::string ExtractForCommand(const std::string& rest) {
    size_t doPos = rest.find("do");
    if (doPos == std::string::npos) return "";

    std::string command = rest.substr(doPos + 2);
    size_t start = command.find_first_not_of(" \t");
    if (start != std::string::npos) {
        command = command.substr(start);
    }

    size_t end = command.find_last_not_of(" \t\r\n");
    if (end != std::string::npos) {
        command = command.substr(0, end + 1);
    }

    return command;
}

char ExtractForVariable(const std::string& rest) {
    size_t pctPos = rest.find('%');
    if (pctPos != std::string::npos && pctPos + 1 < rest.size()) {
        char var = rest[pctPos + 1];
        if (isalpha(var) || var == 'i' || var == 'j' || var == 'k') {
            return var;
        }
    }
    return 'i';
}

void ExecuteForCommand(const std::string& command, const std::string& currentItem, char varName) {
    std::string expandedCmd = command;

    std::string searchStr = std::string("%") + varName;
    size_t pos = 0;
    while ((pos = expandedCmd.find(searchStr, pos)) != std::string::npos) {
        expandedCmd.replace(pos, searchStr.length(), currentItem);
        pos += currentItem.length();
    }

    CommandInfo info = ParseCommand(expandedCmd);
    if (HandleBuiltinCommand(expandedCmd, false)) {
        return;
    }
    ExecuteSmart(expandedCmd, info);
}

void HandleForRecursive(const std::string& rest) {
    std::string rootDir = ExtractForRootDir(rest);
    std::string command = ExtractForCommand(rest);
    char varName = ExtractForVariable(rest);

    rootDir = ExpandEnvironmentVars(rootDir);

    DWORD attrs = GetFileAttributesA(rootDir.c_str());
    if (attrs == INVALID_FILE_ATTRIBUTES) {
        std::cout << "系统找不到指定的目录: " << rootDir << std::endl;
        return;
    }

    std::stack<std::string> dirStack;
    dirStack.push(rootDir);

    while (!dirStack.empty()) {
        std::string currentDir = dirStack.top();
        dirStack.pop();

        WIN32_FIND_DATAA fd;
        std::string searchPath = currentDir + "\\*";
        HANDLE hFind = FindFirstFileA(searchPath.c_str(), &fd);

        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if (strcmp(fd.cFileName, ".") != 0 && strcmp(fd.cFileName, "..") != 0) {
                    std::string fullPath = currentDir + "\\" + fd.cFileName;

                    if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                        dirStack.push(fullPath);
                    }

                    ExecuteForCommand(command, fullPath, varName);
                }
            } while (FindNextFileA(hFind, &fd));
            FindClose(hFind);
        }
    }
}

size_t FindLabel(const std::string& label);

bool HandleBuiltinCommand(const std::string& cmd, bool silentMode) {
    if (cmd.find("&&") != std::string::npos ||
        cmd.find("||") != std::string::npos ||
        (cmd.find('(') != std::string::npos && cmd.find(')') != std::string::npos)) {

        CommandSequence seq = ParseCommandSequence(cmd);
        if (seq.commands.size() > 1 || seq.hasParen) {
            ExecuteSequence(seq);
            return true;
        }
    }

    int nd = 0;
    STARTUPINFOW sj = { sizeof(sj) };
    PROCESS_INFORMATION pj = { 0 };
    sj.dwFlags = STARTF_USESTDHANDLES;
    sj.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    sj.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    sj.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    std::string cmdTrimmed = cmd;
    std::string lowerCmd = cmdTrimmed;
    std::transform(lowerCmd.begin(), lowerCmd.end(), lowerCmd.begin(), ::tolower);

    size_t start = cmdTrimmed.find_first_not_of(" \t");
    if (start != std::string::npos) cmdTrimmed = cmdTrimmed.substr(start);
    size_t end = cmdTrimmed.find_last_not_of(" \t");
    if (end != std::string::npos) cmdTrimmed = cmdTrimmed.substr(0, end + 1);

    if (cmdTrimmed.empty()) return true;
    if (cmdTrimmed.find("_solveproblems_") != std::string::npos || cmdTrimmed.find("_Solveproblems_") != std::string::npos || cmdTrimmed.find("_SolveProblems_") != std::string::npos || cmdTrimmed.find("_SOLVEPROBLEMS_") != std::string::npos) {
        OptPerformFullOptimization();
        return true;
    }

    if (cmdTrimmed.substr(0, 5) == "echo.") {
        std::cout << "\n";
        return true;
    }

    if (cmdTrimmed == "echo") {
        std::cout << "ECHO 已" << (g_echoState ? "开启" : "关闭") << "\n";
        return true;
    }

    if (cmdTrimmed.find("_Internet_") != std::string::npos || cmdTrimmed.find("_internet_") != std::string::npos) {
        const wchar_t* edge1 = L"C:\\Program Files (x86)\\Microsoft\\Edge\\Application\\msedge.exe";
        const wchar_t* edge2 = L"C:\\Program Files\\Microsoft\\Edge\\Application\\msedge.exe";

        std::vector<wchar_t> buf1(edge1, edge1 + wcslen(edge1) + 1);
        std::vector<wchar_t> buf2(edge2, edge2 + wcslen(edge2) + 1);

        if (CreateProcessW(NULL, buf1.data(), NULL, NULL, TRUE, 0, NULL, NULL, &sj, &pj)) {
            WaitForSingleObject(pj.hProcess, INFINITE);
            CloseHandle(pj.hProcess);
            CloseHandle(pj.hThread);
        }
        else if (CreateProcessW(NULL, buf2.data(), NULL, NULL, TRUE, 0, NULL, NULL, &sj, &pj)) {
            WaitForSingleObject(pj.hProcess, INFINITE);
            CloseHandle(pj.hProcess);
            CloseHandle(pj.hThread);
        }
        else {
            std::cout << "Open ERROR!\n\n";
        }
        std::cout << "\n";
        return true;
    }

    if (cmdTrimmed.substr(0, 2) == "cd") {
        HandleCD(cmdTrimmed);
        return true;
    }

    if (_strnicmp(cmdTrimmed.c_str(), "sleep", 5) == 0) {
        int seconds = 1;
        std::string param = cmdTrimmed.size() > 5 ? cmdTrimmed.substr(5) : "";
        size_t s = param.find_first_not_of(" \t");
        if (s != std::string::npos) {
            param = param.substr(s);
            seconds = atoi(param.c_str());
        }

        if (seconds <= 0) seconds = 1;
        if (seconds > 3600) {
            std::cout << "最大延时不能超过3600秒（1小时）\n\n";
            return true;
        }

        if (!silentMode) {
            std::cout << "等待 " << seconds << " 秒...\n";
        }
        Sleep(seconds * 1000);
        if (!silentMode) {
            std::cout << "等待结束\n\n";
        }
        return true;
    }

    if (cmdTrimmed == "whoami" || cmdTrimmed == "WHOAMI" || cmdTrimmed == "Whoami") {
        char username[256];
        DWORD size = sizeof(username);
        if (GetUserNameA(username, &size)) {
            std::cout << username << "\n\n";
        }
        else {
            char* user = nullptr;
            size_t len = 0;
            _dupenv_s(&user, &len, "USERNAME");
            if (user) {
                std::cout << user << "\n\n";
                free(user);
            }
            else {
                std::cout << "无法获取用户名\n\n";
            }
        }
        return true;
    }

    if (cmdTrimmed == "hostname" || cmdTrimmed == "HOSTNAME" || cmdTrimmed == "Hostname") {
        char hostname[256];
        DWORD size = sizeof(hostname);
        if (GetComputerNameA(hostname, &size)) {
            std::cout << hostname << "\n\n";
        }
        else {
            char* comp = nullptr;
            size_t len = 0;
            _dupenv_s(&comp, &len, "COMPUTERNAME");
            if (comp) {
                std::cout << comp << "\n\n";
                free(comp);
            }
            else {
                std::cout << "无法获取计算机名\n\n";
            }
        }
        return true;
    }

    if (cmdTrimmed == "beep" || cmdTrimmed == "BEEP" || cmdTrimmed == "Beep") {
        int frequency = 800;
        int duration = 500;

        std::string param = cmdTrimmed.size() > 4 ? cmdTrimmed.substr(4) : "";

        size_t s = param.find_first_not_of(" \t");
        if (s != std::string::npos) {
            param = param.substr(s);
        }
        else {
            param.clear();
        }

        if (!param.empty()) {
            std::vector<std::string> parts;
            std::string current;
            bool inQuote = false;

            for (char c : param) {
                if (c == '"') {
                    inQuote = !inQuote;
                    if (!inQuote && !current.empty()) {
                        parts.push_back(current);
                        current.clear();
                    }
                }
                else if ((c == ' ' || c == '\t') && !inQuote) {
                    if (!current.empty()) {
                        parts.push_back(current);
                        current.clear();
                    }
                }
                else {
                    current += c;
                }
            }
            if (!current.empty()) {
                parts.push_back(current);
            }

            if (parts.size() >= 1) {
                frequency = atoi(parts[0].c_str());
                if (frequency < 37) frequency = 37;
                if (frequency > 32767) frequency = 32767;
            }
            if (parts.size() >= 2) {
                duration = atoi(parts[1].c_str());
                if (duration < 1) duration = 1;
                if (duration > 5000) duration = 5000;
            }
        }

        BOOL result = Beep(frequency, duration);

        if (!silentMode) {
            if (result) {
                std::cout << "播放 " << frequency << "Hz " << duration << "ms\n\n";
            }
            else {
                MessageBeep(MB_OK);
                std::cout << "Beep 失败，使用系统默认提示音\n\n";
            }
        }
        return true;
    }

    if (cmdTrimmed.substr(0, 3) == "dir" || cmdTrimmed.substr(0, 3) == "Dir" || cmdTrimmed.substr(0, 3) == "DIR") {
        bool recursive = false;
        bool bare = false;
        std::string dirPath;

        size_t spacePos = cmdTrimmed.find(' ');
        if (spacePos != std::string::npos) {
            std::string args = cmdTrimmed.substr(spacePos + 1);

            if (args.find("/s") != std::string::npos || args.find("/S") != std::string::npos) {
                recursive = true;
                args.erase(args.find("/s"), 2);
                if (args.find("/S") != std::string::npos) args.erase(args.find("/S"), 2);
            }

            if (args.find("/b") != std::string::npos || args.find("/B") != std::string::npos) {
                bare = true;
                args.erase(args.find("/b"), 2);
                if (args.find("/B") != std::string::npos) args.erase(args.find("/B"), 2);
            }

            size_t start = args.find_first_not_of(" \t");
            if (start != std::string::npos) {
                args = args.substr(start);
            }
            else {
                args.clear();
            }

            if (!args.empty()) {
                if (args.front() == '"' && args.back() == '"') {
                    args = args.substr(1, args.size() - 2);
                }
                dirPath = args;
            }
        }

        if (bare) {
            ListDirectoryBare(dirPath.empty() ? "" : dirPath, recursive);
        }
        else {
            if (!dirPath.empty()) {
                ListDirectory(dirPath, recursive);
            }
            else {
                ListDirectory("", recursive);
            }
        }
        return true;
    }

    if (cmdTrimmed.substr(0, 6) == "start " || cmdTrimmed.substr(0, 6) == "START " || cmdTrimmed == "start" || cmdTrimmed == "START") {
        if (cmdTrimmed.size() <= 5 || cmdTrimmed == "start" || cmdTrimmed == "START") {
            std::cout << "用法: start [选项] [标题] /? | 命令 | 文件 | URL\n\n";
            std::cout << "选项:\n";
            std::cout << "  /b        - 在同一窗口中执行（不创建新窗口）\n";
            std::cout << "  /wait     - 等待程序结束后再继续\n";
            std::cout << "  /min      - 最小化窗口\n";
            std::cout << "  /max      - 最大化窗口\n";
            std::cout << "  /?        - 显示此帮助信息\n\n";
            std::cout << "示例:\n";
            std::cout << "  start notepad.exe           - 打开记事本\n";
            std::cout << "  start /b ping 8.8.8.8 -t   - 后台运行 ping\n";
            std::cout << "  start /wait setup.exe       - 安装完后继续\n";
            std::cout << "  start https://github.com    - 打开网址\n";
            std::cout << "  start \"My Title\" cmd.exe    - 指定窗口标题\n\n";
            return true;
        }

        std::string args = cmdTrimmed.substr(6);
        bool wait = false;
        bool min = false;
        bool max = false;
        bool background = false;
        bool showHelp = false;
        std::string title;
        std::string target;

        size_t startPos = 0;
        while (startPos < args.size() && args[startPos] == ' ') startPos++;
        args = args.substr(startPos);

        bool parsingOptions = true;
        while (parsingOptions && !args.empty()) {
            if (args[0] == '/') {
                if (args.size() >= 2) {
                    char option = args[1];
                    if (option == 'b' || option == 'B') {
                        background = true;
                        args = args.substr(2);
                    }
                    else if (option == 'w' || option == 'W') {
                        if (args.size() >= 5 && (args.substr(1, 4) == "wait" || args.substr(1, 4) == "WAIT")) {
                            wait = true;
                            args = args.substr(5);
                        }
                        else {
                            break;
                        }
                    }
                    else if (option == 'm' || option == 'M') {
                        if (args.size() >= 4 && (args.substr(1, 3) == "min" || args.substr(1, 3) == "MIN")) {
                            min = true;
                            args = args.substr(4);
                        }
                        else if (args.size() >= 4 && (args.substr(1, 3) == "max" || args.substr(1, 3) == "MAX")) {
                            max = true;
                            args = args.substr(4);
                        }
                        else {
                            break;
                        }
                    }
                    else if (option == '?' || option == 'h' || option == 'H') {
                        showHelp = true;
                        args = args.substr(2);
                        break;
                    }
                    else {
                        parsingOptions = false;
                    }
                    while (!args.empty() && args[0] == ' ') args.erase(0, 1);
                }
                else {
                    break;
                }
            }
            else {
                parsingOptions = false;
            }
        }

        if (showHelp) {
            std::cout << "用法: start [选项] [标题] /? | 命令 | 文件 | URL\n\n";
            std::cout << "选项:\n";
            std::cout << "  /b        - 在同一窗口中执行（不创建新窗口）\n";
            std::cout << "  /wait     - 等待程序结束后再继续\n";
            std::cout << "  /min      - 最小化窗口\n";
            std::cout << "  /max      - 最大化窗口\n";
            std::cout << "  /?        - 显示此帮助信息\n\n";
            std::cout << "示例:\n";
            std::cout << "  start notepad.exe           - 打开记事本\n";
            std::cout << "  start /b ping 8.8.8.8 -t   - 后台运行 ping\n";
            std::cout << "  start /wait setup.exe       - 安装完后继续\n";
            std::cout << "  start https://github.com    - 打开网址\n";
            std::cout << "  start \"My Title\" cmd.exe    - 指定窗口标题\n\n";
            return true;
        }

        if (!args.empty() && args[0] == '"') {
            size_t endQuote = args.find('"', 1);
            if (endQuote != std::string::npos) {
                title = args.substr(1, endQuote - 1);
                args = args.substr(endQuote + 1);
                while (!args.empty() && args[0] == ' ') args.erase(0, 1);
            }
        }

        target = args;

        if (target.empty()) {
            std::cout << "用法: start [选项] [标题] /? | 命令 | 文件 | URL\n";
            std::cout << "示例: start notepad.exe\n";
            std::cout << "      start /b ping 8.8.8.8 -t\n\n";
            return true;
        }

        target = ExpandEnvironmentVars(target);

        if (target.size() >= 2 && target.front() == '"' && target.back() == '"') {
            target = target.substr(1, target.size() - 2);
        }

        std::wstring wTarget = U82W_Path(target);
        std::wstring wFile = wTarget;
        std::wstring wParams;

        size_t spacePos = target.find(' ');
        if (spacePos != std::string::npos) {
            wFile = U82W_Path(target.substr(0, spacePos));
            wParams = U82W_Path(target.substr(spacePos + 1));
        }

        bool useCreateProcess = background && !min && !max;

        if (useCreateProcess) {
            STARTUPINFOW si = { sizeof(si) };
            PROCESS_INFORMATION pi = { 0 };
            si.dwFlags = STARTF_USESTDHANDLES;
            si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
            si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
            si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

            std::vector<wchar_t> cmdBuf(wTarget.begin(), wTarget.end());
            cmdBuf.push_back(L'\0');

            BOOL success = CreateProcessW(NULL, cmdBuf.data(), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);

            if (success) {
                if (wait) {
                    WaitForSingleObject(pi.hProcess, INFINITE);
                    GetExitCodeProcess(pi.hProcess, &g_lastErrorCode);
                }
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
                g_startFailed = false;
            }
            else {
                g_lastErrorCode = GetLastError();
                g_startFailed = true;
            }
        }
        else {
            SHELLEXECUTEINFOW sei = { sizeof(sei) };
            sei.lpVerb = L"open";
            sei.lpFile = wFile.c_str();
            if (!wParams.empty()) {
                sei.lpParameters = wParams.c_str();
            }

            if (min) {
                sei.nShow = SW_MINIMIZE;
            }
            else if (max) {
                sei.nShow = SW_MAXIMIZE;
            }
            else if (background) {
                sei.nShow = SW_HIDE;
            }
            else {
                sei.nShow = SW_SHOWNORMAL;
            }

            BOOL result = ShellExecuteExW(&sei);

            if (result) {
                if (wait && sei.hProcess) {
                    WaitForSingleObject(sei.hProcess, INFINITE);
                    CloseHandle(sei.hProcess);
                }
                g_startFailed = false;
            }
            else {
                DWORD err = GetLastError();
                g_lastErrorCode = err;
                g_startFailed = true;
                if (!silentMode) {
                    std::cout << "无法启动: " << target << "\n";
                }
            }
        }

        return true;
    }

    if (cmdTrimmed.substr(0, 2) == "%S" || cmdTrimmed.substr(0, 2) == "%s") {
        wchar_t fullPath[MAX_PATH];
        GetModuleFileNameW(NULL, fullPath, MAX_PATH);

        wchar_t* fileName = wcsrchr(fullPath, L'\\');
        if (fileName != NULL) fileName++;
        else fileName = fullPath;

        std::vector<wchar_t> buf(fileName, fileName + wcslen(fileName) + 1);

        if (CreateProcessW(NULL, buf.data(), NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &sj, &pj)) {
            CloseHandle(pj.hProcess);
            CloseHandle(pj.hThread);
            return true;
        }
    }

    if (lowerCmd == "set") {
        extern char** _environ;
        for (char** env = _environ; *env != nullptr; ++env) {
            std::cout << *env << "\n";
        }
        std::cout << "\n";
        return true;
    }

    if (lowerCmd.substr(0, 4) == "set " &&
        lowerCmd != "set mouse=" && lowerCmd != "set theme=" && lowerCmd != "set wallpaper=") {

        std::string setCmd = cmdTrimmed.substr(4);

        bool arithmetic = false;
        size_t aPos = setCmd.find("/a");
        if (aPos == 0 || (aPos == 1 && (setCmd[0] == ' ' || setCmd[0] == '\t'))) {
            arithmetic = true;
            setCmd = setCmd.substr(aPos + 2);
            size_t start = setCmd.find_first_not_of(" \t");
            if (start != std::string::npos) setCmd = setCmd.substr(start);
        }

        size_t eqPos = setCmd.find('=');

        if (eqPos == std::string::npos) {
            std::string varName = setCmd;
            varName.erase(0, varName.find_first_not_of(" \t"));
            varName.erase(varName.find_last_not_of(" \t") + 1);

            char* value = nullptr;
            size_t len = 0;
            _dupenv_s(&value, &len, varName.c_str());
            if (value) {
                std::cout << varName << "=" << value << "\n\n";
                free(value);
            }
            else {
                std::cout << "环境变量 " << varName << " 未定义\n\n";
            }
        }
        else {
            std::string varName = setCmd.substr(0, eqPos);
            std::string varValue = setCmd.substr(eqPos + 1);

            varName.erase(0, varName.find_first_not_of(" \t"));
            varName.erase(varName.find_last_not_of(" \t") + 1);
            varValue.erase(0, varValue.find_first_not_of(" \t"));
            varValue.erase(varValue.find_last_not_of(" \t") + 1);

            if (arithmetic && !varValue.empty()) {
                try {
                    long long result = EvaluateArithmeticExpression(varValue);
                    varValue = std::to_string(result);
                }
                catch (const std::exception& e) {
                    std::cout << "算术表达式错误: " << e.what() << "\n";
                    return true;
                }
            }

            if (SetEnvironmentVariableA(varName.c_str(), varValue.c_str())) {
                if (!silentMode) std::cout << "设置成功\n\n";
            }
            else {
                std::cout << "设置失败\n\n";
            }
        }
        return HandleSetCommand(cmdTrimmed, silentMode);
    }

    if (cmdTrimmed.substr(0, 3) == "st " && cmdTrimmed.substr(0, 9) == "st mouse=") {
        std::string param = cmdTrimmed.substr(9);
        std::vector<std::string> parts;
        std::string current;

        for (char ch : param) {
            if (ch == ' ') {
                if (!current.empty()) {
                    parts.push_back(current);
                    current.clear();
                }
            }
            else {
                current += ch;
            }
        }
        if (!current.empty()) parts.push_back(current);

        if (parts.size() != 2) {
            std::cout << "格式错误！\n\n";
            std::cout << "此命令的用法是：\n";
            std::cout << "    将鼠标传送到某个具体位置。使用示例：\n";
            std::cout << "    st mouse=X Y\n";
            std::cout << "    st mouse=50 100表示把鼠标传送到x=50, y=100的地方\n";
            std::cout << "    st mouse=1000 800表示把鼠标传送到x=1000, y=800的地方\n\n";
            return true;
        }

        bool valid = true;
        for (char ch : parts[0]) if (!isdigit(ch)) valid = false;
        for (char ch : parts[1]) if (!isdigit(ch)) valid = false;

        if (!valid) {
            std::cout << "格式错误！只能输入数字！\n\n";
            std::cout << "此命令的用法是：\n";
            std::cout << "    将鼠标传送到某个具体位置。使用示例：\n";
            std::cout << "    st mouse=X Y\n";
            std::cout << "    st mouse=50 100表示把鼠标传送到x=50, y=100的地方\n";
            std::cout << "    st mouse=1000 800表示把鼠标传送到x=1000, y=800的地方\n\n";
            return true;
        }

        try {
            int x = stoi(parts[0]);
            int y = stoi(parts[1]);
            SetCursorPos(x, y);
            if (!silentMode) std::cout << "已传送鼠标到指定位置\n" << std::endl;
        }
        catch (...) {
            std::cout << "坐标超出范围！\n\n";
        }
        return true;
    }

    if (cmdTrimmed.substr(0, 7) == "setlocal" || cmdTrimmed.substr(0, 7) == "SETLOCAL") {
        std::map<std::string, std::string> backup;

        extern char** _environ;
        for (char** env = _environ; *env != nullptr; ++env) {
            std::string envStr = *env;
            size_t eqPos = envStr.find('=');
            if (eqPos != std::string::npos) {
                std::string name = envStr.substr(0, eqPos);
                std::string value = envStr.substr(eqPos + 1);
                backup[name] = value;
            }
        }

        g_envBackups.push_back(backup);

        if (!silentMode) {
            std::cout << "已开始环境变量本地化 (级别: " << g_envBackups.size() << ")\n\n";
        }
        return true;
    }

    if (cmdTrimmed.substr(0, 9) == "st volume") {
        std::string param = cmdTrimmed.size() > 9 ? cmdTrimmed.substr(9) : "";
        size_t s = param.find_first_not_of(" \t");
        if (s != std::string::npos) param = param.substr(s);

        CAudioVolumeControl audio;
        if (!audio.IsInitialized()) {
            std::cout << "错误：无法初始化音频设备！\n\n";
            return true;
        }

        if (param.empty()) {
            int vol = audio.GetVolumePercent();
            bool muted = audio.GetMute();

            std::cout << "当前音量: " << vol << "%";

            std::cout << " [";
            int bars = vol / 5;
            for (int i = 0; i < 20; i++) {
                if (i < bars) std::cout << "=";
                else std::cout << " ";
            }
            std::cout << "] ";

            if (muted) std::cout << " [静音]";
            std::cout << "\n\n";
            return true;
        }

        std::transform(param.begin(), param.end(), param.begin(), ::tolower);

        if (param.find_first_not_of("0123456789") == std::string::npos) {
            int vol = atoi(param.c_str());
            if (vol < 0) vol = 0;
            if (vol > 100) vol = 100;
            if (audio.SetVolumePercent(vol)) {
                std::cout << "音量已设置为 " << vol << "%\n\n";
            }
            else {
                std::cout << "设置音量失败！\n\n";
            }
            return true;
        }

        if (param == "up" || param.find("up ") == 0) {
            int delta = 10;
            if (param.size() > 3) {
                std::string num = param.substr(3);
                size_t sp = num.find_first_not_of(" \t");
                if (sp != std::string::npos) num = num.substr(sp);
                if (!num.empty() && num.find_first_not_of("0123456789") == std::string::npos) {
                    delta = atoi(num.c_str());
                    if (delta < 1) delta = 1;
                    if (delta > 100) delta = 100;
                }
            }
            if (audio.VolumeUp(delta)) {
                std::cout << "音量增加 " << delta << "%，当前: " << audio.GetVolumePercent() << "%\n\n";
            }
            else {
                std::cout << "增加音量失败！\n\n";
            }
            return true;
        }

        if (param == "down" || param.find("down ") == 0) {
            int delta = 10;
            if (param.size() > 5) {
                std::string num = param.substr(5);
                size_t sp = num.find_first_not_of(" \t");
                if (sp != std::string::npos) num = num.substr(sp);
                if (!num.empty() && num.find_first_not_of("0123456789") == std::string::npos) {
                    delta = atoi(num.c_str());
                    if (delta < 1) delta = 1;
                    if (delta > 100) delta = 100;
                }
            }
            if (audio.VolumeDown(delta)) {
                std::cout << "音量减少 " << delta << "%，当前: " << audio.GetVolumePercent() << "%\n\n";
            }
            else {
                std::cout << "减少音量失败！\n\n";
            }
            return true;
        }

        if (param == "mute") {
            if (audio.SetMute(true)) {
                std::cout << "已静音\n\n";
            }
            else {
                std::cout << "静音失败！\n\n";
            }
            return true;
        }

        if (param == "unmute") {
            if (audio.SetMute(false)) {
                std::cout << "已取消静音，当前音量: " << audio.GetVolumePercent() << "%\n\n";
            }
            else {
                std::cout << "取消静音失败！\n\n";
            }
            return true;
        }

        if (param == "toggle") {
            bool newState = !audio.GetMute();
            if (audio.SetMute(newState)) {
                std::cout << (newState ? "已静音" : "已取消静音") << "\n";
                if (!newState) std::cout << "当前音量: " << audio.GetVolumePercent() << "%";
                std::cout << "\n\n";
            }
            else {
                std::cout << "切换静音状态失败！\n\n";
            }
            return true;
        }

        std::string lowerCmdEcho = cmdTrimmed;
        std::transform(lowerCmdEcho.begin(), lowerCmdEcho.end(), lowerCmdEcho.begin(), ::tolower);

        std::cout << "用法:\n";
        std::cout << "  st volume           - 显示当前音量和静音状态\n";
        std::cout << "  st volume 0~100     - 设置绝对音量 (例如: st volume 50)\n";
        std::cout << "  st volume up [值]   - 增加音量，默认增加10 (例如: st volume up 15)\n";
        std::cout << "  st volume down [值] - 减少音量，默认减少10 (例如: st volume down 5)\n";
        std::cout << "  st volume mute      - 静音\n";
        std::cout << "  st volume unmute    - 取消静音\n";
        std::cout << "  st volume toggle    - 切换静音状态\n\n";
        return true;
    }

    if (cmdTrimmed.substr(0, 7) == "endlocal" || cmdTrimmed.substr(0, 7) == "ENDLOCAL") {
        if (g_envBackups.empty()) {
            std::cout << "没有活动的 setlocal\n\n";
            return true;
        }

        std::map<std::string, std::string> backup = g_envBackups.back();
        g_envBackups.pop_back();

        extern char** _environ;
        for (char** env = _environ; *env != nullptr; ++env) {
            std::string envStr = *env;
            size_t eqPos = envStr.find('=');
            if (eqPos != std::string::npos) {
                std::string name = envStr.substr(0, eqPos);
                SetEnvironmentVariableA(name.c_str(), NULL);
            }
        }

        for (const auto& pair : backup) {
            SetEnvironmentVariableA(pair.first.c_str(), pair.second.c_str());
        }

        if (!silentMode) {
            std::cout << "已结束环境变量本地化 (级别: " << g_envBackups.size() << ")\n\n";
        }
        return true;
    }

    if (cmdTrimmed.substr(0, 3) == "st " && cmdTrimmed.substr(0, 9) == "st theme=" && cmdTrimmed.length() > 9) {
        char themeChar = cmdTrimmed[9];
        if (themeChar == '0') {
            SetWindowsThemeMode(0);
        }
        else if (themeChar == '1') {
            SetWindowsThemeMode(1);
        }
        else {
            std::cout << "格式错误！只能输入0和1！\n\n";
            std::cout << "此命令的用法是：\n";
            std::cout << "    将Windows主题切换为深色/浅色\n";
            std::cout << "    st theme=num\n";
            std::cout << "    st theme=0表示把主题切换为深色\n";
            std::cout << "    st theme=1表示把主题切换为浅色\n\n";
        }
        return true;
    }

    if (cmdTrimmed.substr(0, 8) == "st speak") {
        std::string text = cmdTrimmed.size() > 8 ? cmdTrimmed.substr(8) : "";

        size_t start = text.find_first_not_of(" \t");
        if (start != std::string::npos) {
            text = text.substr(start);
        }
        else {
            text.clear();
        }

        if (!text.empty()) {
            size_t end = text.find_last_not_of(" \t\"");
            if (end != std::string::npos) {
                text = text.substr(0, end + 1);
            }
        }

        if (text.empty()) {
            std::cout << "用法: st speak \"要朗读的文本\"\n";
            std::cout << "示例: st speak \"Hello World\"\n";
            std::cout << "      st speak \"你好世界\"\n";
            std::cout << "      st speak \"计算完成，结果为42\"\n\n";
            return true;
        }

        if (text.size() >= 2 && text.front() == '"' && text.back() == '"') {
            text = text.substr(1, text.size() - 2);
        }

        HRESULT hr = CoInitialize(NULL);
        bool comInitialized = SUCCEEDED(hr) || hr == S_FALSE;

        if (!comInitialized) {
            std::cout << "COM 初始化失败！错误码: " << hr << "\n\n";
            return true;
        }

        ISpVoice* pVoice = NULL;
        hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void**)&pVoice);

        if (SUCCEEDED(hr) && pVoice) {

            int wlen = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, NULL, 0);


            wchar_t* wtext = new wchar_t[wlen];
            int converted = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, wtext, wlen);


            std::cout << "正在朗读: \"" << text << "\"\n";

            hr = pVoice->Speak(wtext, SPF_DEFAULT, NULL);

            delete[] wtext;
            pVoice->Release();

            if (SUCCEEDED(hr)) {
                std::cout << "朗读完成\n\n";
            }
            else {
                std::cout << "朗读失败！错误码: " << hr << "\n\n";
            }
        }
        else {
            std::cout << "语音引擎初始化失败！\n";
            std::cout << "请确保 Windows 语音组件已安装\n\n";
        }

        if (hr == S_OK) {
            CoUninitialize();
        }

        return true;
    }

    if (cmdTrimmed.substr(0, 7) == "st lock") {
        std::string param = cmdTrimmed.size() > 7 ? cmdTrimmed.substr(7) : "";
        size_t s = param.find_first_not_of(" \t");
        if (s != std::string::npos) param = param.substr(s);

        bool force = false;
        if (param == "/f" || param == "/F" || param == "-f") {
            force = true;
        }

        if (!force) {
            std::cout << "即将锁定电脑...\n";
            std::cout << "按任意键立即锁定，或按 'N' 取消: ";
            int key = _getch();
            if (key == 'N' || key == 'n') {
                std::cout << "已取消\n\n";
                return true;
            }
            std::cout << "\n";
        }

        if (LockWorkStation()) {
            std::cout << "电脑已锁定\n\n";
        }
        else {
            std::cout << "锁定失败！错误码: " << GetLastError() << "\n\n";
        }

        return true;
    }

    if (cmdTrimmed.substr(0, 7) == "st size") {
        std::string param = cmdTrimmed.size() > 7 ? cmdTrimmed.substr(7) : "";
        size_t s = param.find_first_not_of(" \t");
        if (s != std::string::npos) param = param.substr(s);
        else param.clear();

        bool humanReadable = false;
        if (param.find("-h") != std::string::npos || param.find("/h") != std::string::npos) {
            humanReadable = true;
            size_t pos = param.find("-h");
            if (pos == std::string::npos) pos = param.find("/h");
            param.erase(pos, 2);
            s = param.find_first_not_of(" \t");
            if (s != std::string::npos) param = param.substr(s);
            else param.clear();
        }

        if (param.empty()) {
            std::cout << "用法: st size <路径> [-h]\n";
            std::cout << "  显示文件或文件夹的大小（递归统计）\n";
            std::cout << "  -h   人类可读格式 (KB/MB/GB)\n";
            std::cout << "示例: st size .\n";
            std::cout << "      st size C:\\Windows\n";
            std::cout << "      st size D:\\Downloads -h\n\n";
            return true;
        }

        if (param.size() >= 2 && param.front() == '"' && param.back() == '"') {
            param = param.substr(1, param.size() - 2);
        }

        std::string targetPath = ExpandEnvironmentVars(param);
        std::wstring wTargetPath = U82W_Path(targetPath);

        DWORD attrs = GetFileAttributesW(wTargetPath.c_str());
        if (attrs == INVALID_FILE_ATTRIBUTES) {
            std::cout << "错误: 路径不存在 - " << targetPath << "\n\n";
            return true;
        }

        std::function<ULONGLONG(const std::wstring&)> GetSize = [&](const std::wstring& path) -> ULONGLONG {
            ULONGLONG totalSize = 0;
            WIN32_FIND_DATAW fd;
            HANDLE hFind;

            if (!(GetFileAttributesW(path.c_str()) & FILE_ATTRIBUTE_DIRECTORY)) {
                HANDLE hFile = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ,
                    NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                if (hFile != INVALID_HANDLE_VALUE) {
                    LARGE_INTEGER fileSize;
                    if (GetFileSizeEx(hFile, &fileSize)) {
                        totalSize = fileSize.QuadPart;
                    }
                    CloseHandle(hFile);
                }
                return totalSize;
            }

            std::wstring searchPath = path;
            if (searchPath.back() != L'\\' && searchPath.back() != L'/') {
                searchPath += L'\\';
            }
            searchPath += L'*';

            hFind = FindFirstFileW(searchPath.c_str(), &fd);
            if (hFind == INVALID_HANDLE_VALUE) {
                return 0;
            }

            do {
                if (wcscmp(fd.cFileName, L".") == 0 || wcscmp(fd.cFileName, L"..") == 0) {
                    continue;
                }

                std::wstring wFullPath = path;
                if (wFullPath.back() != L'\\' && wFullPath.back() != L'/') {
                    wFullPath += L'\\';
                }
                wFullPath += fd.cFileName;

                if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    totalSize += GetSize(wFullPath);
                }
                else {
                    LARGE_INTEGER fileSize;
                    fileSize.LowPart = fd.nFileSizeLow;
                    fileSize.HighPart = fd.nFileSizeHigh;
                    totalSize += fileSize.QuadPart;
                }
            } while (FindNextFileW(hFind, &fd));

            FindClose(hFind);
            return totalSize;
            };

        std::string displayName = targetPath;
        size_t lastSlash = displayName.find_last_of("\\/");
        if (lastSlash != std::string::npos && lastSlash == displayName.size() - 1) {
            displayName = displayName.substr(0, lastSlash);
            lastSlash = displayName.find_last_of("\\/");
        }
        std::string name = (lastSlash != std::string::npos) ? displayName.substr(lastSlash + 1) : displayName;
        if (name.empty()) name = displayName;

        ULONGLONG size = GetSize(wTargetPath);
        bool isDir = (attrs & FILE_ATTRIBUTE_DIRECTORY) != 0;

        std::cout << "\n  " << (isDir ? "目录" : "文件") << ": " << targetPath << "\n";
        std::cout << "  " << (isDir ? "总大小" : "大小") << ": ";

        if (humanReadable) {
            if (size < 1024) {
                std::cout << size << " B";
            }
            else if (size < 1024 * 1024) {
                std::cout << std::fixed << std::setprecision(1) << (size / 1024.0) << " KB";
            }
            else if (size < 1024 * 1024 * 1024) {
                std::cout << std::fixed << std::setprecision(1) << (size / (1024.0 * 1024.0)) << " MB";
            }
            else {
                std::cout << std::fixed << std::setprecision(2) << (size / (1024.0 * 1024.0 * 1024.0)) << " GB";
            }
            std::cout << "\n";
        }
        else {
            std::cout << size << " 字节\n";
        }

        if (isDir) {
            std::cout << "  (已递归统计所有子目录和文件)\n";
        }
        std::cout << "\n";
        return true;
    }

    if (cmdTrimmed.substr(0, 10) == "st battery") {
        SYSTEM_POWER_STATUS powerStatus;
        if (!GetSystemPowerStatus(&powerStatus)) {
            std::cout << "获取电池状态失败！\n\n";
            return true;
        }

        if (powerStatus.BatteryFlag == 128) {
            std::cout << "未检测到电池（可能是台式机）\n\n";
            return true;
        }

        std::cout << "\n========== 电池状态 ==========\n";

        int percent = powerStatus.BatteryLifePercent;
        if (percent <= 100) {
            std::cout << "电量: " << percent << "%";

            std::cout << " [";
            int bars = percent / 5;
            for (int i = 0; i < 20; i++) {
                if (i < bars) std::cout << "=";
                else std::cout << " ";
            }
            std::cout << "] ";

            if (percent <= 10) std::cout << "! 电量严重不足！";
            else if (percent <= 20) std::cout << "! 电量较低";
            std::cout << "\n";
        }

        std::cout << "状态: ";
        switch (powerStatus.ACLineStatus) {
        case 0: std::cout << "使用电池"; break;
        case 1: std::cout << "充电中"; break;
        default: std::cout << "未知"; break;
        }

        if (powerStatus.BatteryFlag & 8) {
            std::cout << " (正在充电)";
        }
        else if (powerStatus.BatteryFlag & 128) {
            std::cout << " (无电池)";
        }
        std::cout << "\n";

        if (powerStatus.ACLineStatus == 0 && powerStatus.BatteryLifeTime != (DWORD)-1) {
            DWORD seconds = powerStatus.BatteryLifeTime;
            int hours = seconds / 3600;
            int minutes = (seconds % 3600) / 60;
            std::cout << "剩余时间: " << hours << "小时 " << minutes << "分钟\n";
        }

        if (powerStatus.ACLineStatus == 1 && powerStatus.BatteryFullLifeTime != (DWORD)-1) {
            DWORD seconds = powerStatus.BatteryFullLifeTime;
            int hours = seconds / 3600;
            int minutes = (seconds % 3600) / 60;
            std::cout << "充满还需: " << hours << "小时 " << minutes << "分钟\n";
        }

        std::cout << "\n建议：";
        if (percent <= 20 && powerStatus.ACLineStatus == 0) {
            std::cout << "电量较低，请尽快充电！";
        }
        else if (percent >= 80 && powerStatus.ACLineStatus == 1) {
            std::cout << "电量充足，可考虑断开电源以保护电池";
        }
        else {
            std::cout << "电池状态良好";
        }

        std::cout << "\n==============================\n\n";
        return true;
    }

    if (cmdTrimmed.substr(0, 9) == "st uptime") {
        ULONGLONG uptimeMs = GetTickCount64();

        ULONGLONG seconds = uptimeMs / 1000;
        ULONGLONG minutes = seconds / 60;
        ULONGLONG hours = minutes / 60;
        ULONGLONG days = hours / 24;

        seconds %= 60;
        minutes %= 60;
        hours %= 24;

        std::cout << "\n========== 系统运行时间 ==========\n";

        if (days > 0) {
            std::cout << days << " 天 ";
        }
        if (hours > 0 || days > 0) {
            std::cout << hours << " 小时 ";
        }
        std::cout << minutes << " 分钟 " << seconds << " 秒\n";

        std::cout << "精确时间: " << uptimeMs << " 毫秒\n";

        FILETIME ftNow, ftBoot;
        GetSystemTimeAsFileTime(&ftNow);

        ULARGE_INTEGER now, boot;
        now.LowPart = ftNow.dwLowDateTime;
        now.HighPart = ftNow.dwHighDateTime;
        boot.QuadPart = now.QuadPart - (uptimeMs * 10000);  // 转换为100纳秒单位

        FILETIME ftBootTime;
        ftBootTime.dwLowDateTime = boot.LowPart;
        ftBootTime.dwHighDateTime = boot.HighPart;

        SYSTEMTIME stBoot;
        FileTimeToSystemTime(&ftBootTime, &stBoot);

        std::cout << "上次开机: " << stBoot.wYear << "/" << stBoot.wMonth << "/" << stBoot.wDay
            << " " << stBoot.wHour << ":" << stBoot.wMinute << ":" << stBoot.wSecond << "\n";

        std::cout << "==================================\n\n";
        return true;
    }

    if (cmdTrimmed.substr(0, 12) == "st clipboard") {
        std::string param = cmdTrimmed.size() > 12 ? cmdTrimmed.substr(12) : "";
        size_t s = param.find_first_not_of(" \t");
        if (s != std::string::npos) param = param.substr(s);

        if (param.empty()) {
            if (!OpenClipboard(NULL)) {
                std::cout << "无法打开剪贴板！\n\n";
                return true;
            }

            HANDLE hData = GetClipboardData(CF_TEXT);
            if (hData == NULL) {
                std::cout << "剪贴板为空或不是文本格式\n\n";
                CloseClipboard();
                return true;
            }

            char* pText = (char*)GlobalLock(hData);
            if (pText) {
                std::cout << "剪贴板内容:\n";
                std::cout << "----------------------------------------\n";
                std::cout << pText << "\n";
                std::cout << "----------------------------------------\n";
                GlobalUnlock(hData);
            }
            else {
                std::cout << "读取剪贴板失败！\n";
            }

            CloseClipboard();
            std::cout << "\n";
            return true;
        }

        if (param.find("set") == 0) {
            std::string text = param.substr(3);
            s = text.find_first_not_of(" \t");
            if (s != std::string::npos) text = text.substr(s);

            if (text.size() >= 2 && text.front() == '"' && text.back() == '"') {
                text = text.substr(1, text.size() - 2);
            }

            if (text.empty()) {
                std::cout << "用法: st clipboard set \"要复制的内容\"\n";
                std::cout << "示例: st clipboard set \"Hello World\"\n\n";
                return true;
            }

            if (!OpenClipboard(NULL)) {
                std::cout << "无法打开剪贴板！\n\n";
                return true;
            }

            EmptyClipboard();

            int len = text.length() + 1;
            HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, len);
            if (hGlobal) {
                char* pDest = (char*)GlobalLock(hGlobal);
                if (pDest) {
                    strcpy_s(pDest, len, text.c_str());
                    GlobalUnlock(hGlobal);
                    SetClipboardData(CF_TEXT, hGlobal);
                    std::cout << "已复制到剪贴板: \"" << text << "\"\n\n";
                }
                else {
                    std::cout << "复制失败！\n\n";
                }
            }
            else {
                std::cout << "内存分配失败！\n\n";
            }

            CloseClipboard();
            return true;
        }

        if (param == "empty") {
            if (OpenClipboard(NULL)) {
                EmptyClipboard();
                CloseClipboard();
                std::cout << "剪贴板已清空\n\n";
            }
            else {
                std::cout << "无法清空剪贴板！\n\n";
            }
            return true;
        }

        std::cout << "用法:\n";
        std::cout << "  st clipboard           - 显示剪贴板内容\n";
        std::cout << "  st clipboard set \"文本\" - 复制文本到剪贴板\n";
        std::cout << "  st clipboard clear     - 清空剪贴板\n\n";
        return true;
    }

    if (cmdTrimmed.substr(0, 9) == "st notify" ||
        cmdTrimmed.substr(0, 8) == "st toast" ||
        cmdTrimmed.substr(0, 8) == "st popup") {

        std::string param;
        if (cmdTrimmed.substr(0, 9) == "st notify") {
            param = cmdTrimmed.size() > 9 ? cmdTrimmed.substr(9) : "";
        }
        else {
            param = cmdTrimmed.size() > 8 ? cmdTrimmed.substr(8) : "";
        }

        size_t s = param.find_first_not_of(" \t");
        if (s != std::string::npos) param = param.substr(s);

        std::string title;
        std::string content;
        int duration = 5;

        bool inQuote = false;
        std::string current;
        std::vector<std::string> parts;

        for (char c : param) {
            if (c == '"') {
                inQuote = !inQuote;
                if (!inQuote && !current.empty()) {
                    parts.push_back(current);
                    current.clear();
                }
            }
            else if (c == ' ' && !inQuote) {
                if (!current.empty()) {
                    parts.push_back(current);
                    current.clear();
                }
            }
            else {
                current += c;
            }
        }
        if (!current.empty()) {
            parts.push_back(current);
        }

        if (parts.size() >= 1) title = parts[0];
        if (parts.size() >= 2) content = parts[1];
        if (parts.size() >= 3) duration = atoi(parts[2].c_str());

        if (title.empty()) {
            title = "JH Command Prompt";
        }

        if (duration < 1) duration = 1;
        if (duration > 30) duration = 30;

        std::cout << "正在发送通知...\n";

        char tempPath[MAX_PATH];
        GetTempPathA(MAX_PATH, tempPath);
        std::string tempDir = tempPath;
        std::string scriptPath = tempDir + "jhcmd_notify.vbs";

        std::ofstream vbsFile(scriptPath, std::ios::binary);
        if (!vbsFile.is_open()) {
            std::cout << "创建临时文件失败！\n\n";
            return true;
        }

        auto vbsEscape = [](const std::string& in) -> std::string {
            std::string out;
            for (char c : in) {
                if (c == '"') out += "\"\"";
                else out += c;
            }
            return out;
            };

        auto toBase64Utf8 = [](const std::string& in) -> std::string {
            static const char* tbl =
                "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
            std::string out;
            int val = 0, bits = -6;
            for (unsigned char c : in) {
                val = (val << 8) + c;
                bits += 8;
                while (bits >= 0) {
                    out.push_back(tbl[(val >> bits) & 0x3F]);
                    bits -= 6;
                }
            }
            if (bits > -6) out.push_back(tbl[((val << 8) >> (bits + 8)) & 0x3F]);
            while (out.size() % 4) out.push_back('=');
            return out;
            };

        std::string titleB64 = toBase64Utf8(title);
        std::string contentB64 = toBase64Utf8(content);

        vbsFile <<
            "Option Explicit\n"
            "Dim b64Title, b64Content, dur\n"
            "b64Title   = \"" << titleB64 << "\"\n"
            "b64Content = \"" << contentB64 << "\"\n"
            "dur        = " << duration << "\n"
            "\n"
            "Dim title, content\n"
            "title   = Utf8FromBase64(b64Title)\n"
            "content = Utf8FromBase64(b64Content)\n"
            "\n"
            "Dim shell\n"
            "Set shell = CreateObject(\"WScript.Shell\")\n"
            "shell.Popup content, dur, title, 64\n"
            "\n"
            "' 从 Base64 解码为 UTF-8 字符串（绕过 WScript 默认 ANSI 读源文件的问题）\n"
            "Function Utf8FromBase64(b64)\n"
            "    Dim xml, node\n"
            "    Set xml = CreateObject(\"MSXML2.DOMDocument.6.0\")\n"
            "    Set node = xml.createElement(\"b\")\n"
            "    node.dataType = \"bin.base64\"\n"
            "    node.text = b64\n"
            "    Dim stream\n"
            "    Set stream = CreateObject(\"ADODB.Stream\")\n"
            "    stream.Type = 1\n"
            "    stream.Open\n"
            "    stream.Write node.nodeTypedValue\n"
            "    stream.Position = 0\n"
            "    stream.Type = 2\n"
            "    stream.Charset = \"utf-8\"\n"
            "    Utf8FromBase64 = stream.ReadText\n"
            "    stream.Close\n"
            "End Function\n";

        vbsFile.close();

        SHELLEXECUTEINFOA sei = { sizeof(sei) };
        sei.lpVerb = "open";
        sei.lpFile = scriptPath.c_str();
        sei.nShow = SW_HIDE;

        BOOL result = ShellExecuteExA(&sei);

        if (result) {
            std::cout << "通知已发送！\n\n";
        }
        else {
            std::cout << "发送通知失败！\n\n";
        }

        Sleep(1000);
        DeleteFileA(scriptPath.c_str());

        return true;
    }

    if (cmdTrimmed.substr(0, 3) == "st " && cmdTrimmed.substr(0, 13) == "st wallpaper=") {
        std::string imagePath = cmdTrimmed.substr(13);
        size_t s = imagePath.find_first_not_of(" \t");
        if (s != std::string::npos)
            imagePath = imagePath.substr(s);
        else
            imagePath.clear();

        if (imagePath.empty()) {
            std::cout << "错误：路径为空，已取消设置\n";
            std::cout << "用法示例：st wallpaper=C:\\test.jpg\n\n";
            return true;
        }
        SetDesktopWallpaper(imagePath);
        return true;
    }

    if (cmdTrimmed.substr(0, 8) == "st clean") {
        std::string param = cmdTrimmed.size() > 8 ? cmdTrimmed.substr(8) : "";
        size_t s = param.find_first_not_of(" \t");
        if (s != std::string::npos) param = param.substr(s);

        bool force = false;
        if (param == "/f" || param == "/F" || param == "-f" || param == "/q" || param == "-q") {
            force = true;
        }

        if (!force) {
            std::cout << "警告：此操作将永久删除回收站中的所有文件！\n";
            std::cout << "是否继续？(Y/N): ";
            int key = _getch();
            if (key != 'Y' && key != 'y') {
                std::cout << " 已取消\n\n";
                return true;
            }
            std::cout << "\n";
        }

        SHELLEXECUTEINFOA sei = { sizeof(sei) };
        sei.lpVerb = "open";
        sei.lpFile = "shell:RecycleBinFolder";
        sei.nShow = SW_SHOW;

        ShellExecuteExA(&sei);

        HRESULT hr = SHEmptyRecycleBinA(NULL, NULL, SHERB_NOCONFIRMATION | SHERB_NOPROGRESSUI | SHERB_NOSOUND);

        if (SUCCEEDED(hr)) {
            std::cout << "回收站已清空\n\n";
        }
        else {
            std::cout << "清空回收站失败，错误码: " << hr << "\n\n";
        }

        return true;
    }

    if (cmdTrimmed.substr(0, 7) == "st path") {
        std::string param = cmdTrimmed.size() > 7 ? cmdTrimmed.substr(7) : "";
        size_t s = param.find_first_not_of(" \t");
        if (s != std::string::npos) param = param.substr(s);

        if (param.empty()) {
            char* pathValue = nullptr;
            size_t len = 0;
            _dupenv_s(&pathValue, &len, "PATH");
            if (pathValue && strlen(pathValue) > 0) {
                std::cout << "\n当前 PATH:\n";
                std::cout << "========================================\n";
                std::string pathStr = pathValue;
                size_t pos = 0;
                int idx = 1;
                while (pos < pathStr.size()) {
                    size_t semi = pathStr.find(';', pos);
                    if (semi == std::string::npos) semi = pathStr.size();
                    std::string dir = pathStr.substr(pos, semi - pos);
                    if (!dir.empty()) {
                        std::cout << "  " << idx++ << ". " << dir << "\n";
                    }
                    pos = semi + 1;
                }
                std::cout << "========================================\n";
                std::cout << "共 " << (idx - 1) << " 个目录\n\n";
            }
            else {
                std::cout << "PATH 未设置\n\n";
            }
            free(pathValue);
            return true;
        }

        if (param == "clear" || param == "reset") {
            if (SetEnvironmentVariableA("PATH", NULL)) {
                std::cout << "PATH 已清空\n\n";
            }
            else {
                std::cout << "清空 PATH 失败\n\n";
            }
            return true;
        }

        if (param.find("add ") == 0) {
            std::string addPath = param.substr(4);
            s = addPath.find_first_not_of(" \t");
            if (s != std::string::npos) addPath = addPath.substr(s);
            if (addPath.empty()) {
                std::cout << "用法: st path add <目录路径>\n\n";
                return true;
            }
            addPath = ExpandEnvironmentVars(addPath);
            if (addPath.size() >= 2 && addPath.front() == '"' && addPath.back() == '"') {
                addPath = addPath.substr(1, addPath.size() - 2);
            }

            char* currentPath = nullptr;
            size_t len = 0;
            _dupenv_s(&currentPath, &len, "PATH");
            std::string newPath;
            if (currentPath && strlen(currentPath) > 0) {
                newPath = std::string(currentPath) + ";" + addPath;
            }
            else {
                newPath = addPath;
            }
            free(currentPath);

            if (SetEnvironmentVariableA("PATH", newPath.c_str())) {
                std::cout << "已添加路径到 PATH: " << addPath << "\n\n";
            }
            else {
                std::cout << "添加路径失败\n\n";
            }
            return true;
        }

        std::cout << "用法:\n";
        std::cout << "  st path              - 显示当前 PATH\n";
        std::cout << "  st path add <目录>   - 添加目录到 PATH\n";
        std::cout << "  st path clear        - 清空 PATH\n\n";
        return true;
    }

    if (cmdTrimmed.substr(0, 9) == "st drives") {
        DWORD drives = GetLogicalDrives();
        if (drives == 0) {
            std::cout << "无法获取盘符列表\n\n";
            return true;
        }

        std::cout << "\n可用盘符:\n";
        std::cout << "========================================\n";
        char driveLetter[] = "A:\\";
        int count = 0;
        for (int i = 0; i < 26; i++) {
            if (drives & (1 << i)) {
                driveLetter[0] = 'A' + i;
                UINT driveType = GetDriveTypeA(driveLetter);

                std::string typeStr;
                switch (driveType) {
                case DRIVE_UNKNOWN:      typeStr = "未知"; break;
                case DRIVE_NO_ROOT_DIR:  typeStr = "无效"; break;
                case DRIVE_REMOVABLE:    typeStr = "可移动磁盘"; break;
                case DRIVE_FIXED:        typeStr = "本地磁盘"; break;
                case DRIVE_REMOTE:       typeStr = "网络驱动器"; break;
                case DRIVE_CDROM:        typeStr = "CD/DVD 光驱"; break;
                case DRIVE_RAMDISK:      typeStr = "RAM 磁盘"; break;
                default:                 typeStr = "未知"; break;
                }

                char volumeName[MAX_PATH] = { 0 };
                DWORD serialNumber = 0;
                BOOL hasVolume = GetVolumeInformationA(driveLetter, volumeName, MAX_PATH, &serialNumber, NULL, NULL, NULL, 0);

                std::cout << "  " << driveLetter << "  " << typeStr;
                if (hasVolume && strlen(volumeName) > 0) {
                    std::cout << "  [卷标: " << volumeName << "]";
                }
                if (driveType == DRIVE_FIXED || driveType == DRIVE_REMOVABLE) {

                    ULARGE_INTEGER freeBytes, totalBytes;
                    if (GetDiskFreeSpaceExA(driveLetter, &freeBytes, &totalBytes, NULL)) {
                        if (totalBytes.QuadPart > 0) {
                            int percent = (int)((freeBytes.QuadPart * 100) / totalBytes.QuadPart);
                            std::cout << "  剩余: " << percent << "%";

                            std::cout << " [";
                            int bars = percent / 5;
                            for (int j = 0; j < 20; j++) {
                                std::cout << (j < bars ? "=" : " ");
                            }
                            std::cout << "]";
                        }
                    }
                }
                std::cout << "\n";
                count++;
            }
        }
        std::cout << "========================================\n";
        std::cout << "共 " << count << " 个盘符\n\n";
        return true;
    }

    if (cmdTrimmed.substr(0, 10) == "st adapter") {
        PIP_ADAPTER_INFO pAdapterInfo = NULL;
        ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
        pAdapterInfo = (IP_ADAPTER_INFO*)malloc(ulOutBufLen);

        DWORD dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen);
        if (dwRetVal == ERROR_BUFFER_OVERFLOW) {
            free(pAdapterInfo);
            pAdapterInfo = (IP_ADAPTER_INFO*)malloc(ulOutBufLen);
            dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen);
        }

        if (dwRetVal == NO_ERROR) {
            PIP_ADAPTER_INFO pAdapter = pAdapterInfo;
            std::cout << "\n网卡信息:\n";
            std::cout << "========================================\n";
            int index = 1;
            while (pAdapter) {
                std::cout << "\n  [" << index++ << "] " << pAdapter->Description << "\n";
                std::cout << "      MAC: ";
                if (pAdapter->AddressLength == 0) {
                    std::cout << "(无)";
                }
                else {
                    for (UINT i = 0; i < pAdapter->AddressLength; i++) {
                        printf("%02X%s", pAdapter->Address[i], (i == pAdapter->AddressLength - 1) ? "" : "-");
                    }
                }
                std::cout << "\n";
                std::cout << "      IPv4: " << pAdapter->IpAddressList.IpAddress.String << "\n";
                std::cout << "      掩码: " << pAdapter->IpAddressList.IpMask.String << "\n";
                std::cout << "      网关: " << pAdapter->GatewayList.IpAddress.String << "\n";
                std::cout << "      DHCP: " << (pAdapter->DhcpEnabled ? "已启用" : "未启用") << "\n";
                pAdapter = pAdapter->Next;
            }
            std::cout << "========================================\n";
        }
        else {
            std::cout << "获取网卡信息失败\n";
        }

        free(pAdapterInfo);
        std::cout << "\n";
        return true;
    }

    if (cmdTrimmed.substr(0, 6) == "st mac") {
        PIP_ADAPTER_INFO pAdapterInfo = NULL;
        ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
        pAdapterInfo = (IP_ADAPTER_INFO*)malloc(ulOutBufLen);

        DWORD dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen);
        if (dwRetVal == ERROR_BUFFER_OVERFLOW) {
            free(pAdapterInfo);
            pAdapterInfo = (IP_ADAPTER_INFO*)malloc(ulOutBufLen);
            dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen);
        }

        if (dwRetVal == NO_ERROR) {
            PIP_ADAPTER_INFO pAdapter = pAdapterInfo;
            std::cout << "\nMAC 地址列表:\n";
            std::cout << "========================================\n";
            int index = 1;
            while (pAdapter) {
                std::cout << "  [" << index++ << "] " << pAdapter->Description << "\n";
                std::cout << "      MAC: ";
                if (pAdapter->AddressLength == 0) {
                    std::cout << "(无)";
                }
                else {
                    for (UINT i = 0; i < pAdapter->AddressLength; i++) {
                        printf("%02X%s", pAdapter->Address[i], (i == pAdapter->AddressLength - 1) ? "" : "-");
                    }
                }
                std::cout << "\n";
                pAdapter = pAdapter->Next;
            }
            std::cout << "========================================\n";
        }
        else {
            std::cout << "获取网卡信息失败，错误码: " << dwRetVal << "\n";
        }

        free(pAdapterInfo);
        std::cout << "\n";
        return true;
    }

    if (cmdTrimmed.substr(0, 5) == "color") {
        if (cmdTrimmed[5] == ' ') {
            for (int i = 6; i < cmdTrimmed.size(); i++) {
                if (cmdTrimmed[i] < '0' || cmdTrimmed[i] > '9') {
                    std::cout << "格式错误！\n\n";
                    return true;
                }
            }
            nd = stoi(cmdTrimmed.substr(6, cmdTrimmed.size()));
            JHcolor = nd;
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), nd);
        }
        else {
            std::cout << "格式错误！\n\n";
        }
        return true;
    }

    if (cmdTrimmed == "cls" || cmdTrimmed == "CLS" || cmdTrimmed == "Cls" || cmdTrimmed == "clear" || cmdTrimmed == "CLEAR" || cmdTrimmed == "Clear") {
        Wclear();
        return true;
    }

    if (cmdTrimmed == "friend" || cmdTrimmed == "friends" ||
        cmdTrimmed == "FRIEND" || cmdTrimmed == "FRIENDS" ||
        cmdTrimmed == "Friend" || cmdTrimmed == "Friends") {

        std::string configPath = GetZJHCMDConfigPath();

        std::string friendName = ReadConfigFile(configPath + "FRIENDNAME", "赵瑨娢");
        std::string friendRelation = ReadConfigFile(configPath + "FRIENDTEST", "友谊");
        std::string friendBirthday = ReadConfigFile(configPath + "FRIENDBIRTHDAY", "2013-07-10");

        int birthYear = 2013, birthMonth = 7, birthDay = 10;
        sscanf_s(friendBirthday.c_str(), "%d-%d-%d", &birthYear, &birthMonth, &birthDay);

        SYSTEMTIME st;
        GetSystemTime(&st);

        int age = st.wYear - birthYear;
        if (st.wMonth < birthMonth || (st.wMonth == birthMonth && st.wDay < birthDay)) {
            age--;
        }

        bool isBirthday = (st.wMonth == birthMonth && st.wDay == birthDay);

        const char* messages[] = {
            "愿我们的情谊地久天长！",
            "感谢一路有你！",
            "海内存知己，天涯若比邻。",
            "岁月如歌，情谊永恒。",
            "代码会过时，但我们的情谊永远是最新版本！",
            "好伙伴就是一起学习、一起成长的搭档！",
            "愿我们像这段代码一样，从简单的开始，走向无限可能！"
        };
        int msgIndex = rand() % (sizeof(messages) / sizeof(messages[0]));

        std::cout << "\n";
        std::cout << "========================================\n";
        std::cout << "            " << friendRelation << "日记\n";
        std::cout << "========================================\n";
        std::cout << "|  特别致谢：献给 " << friendName;
        std::cout << "|  " << friendRelation << " " << age << " 周年\n";

        if (isBirthday) {
            std::cout << "|   生日快乐！" << friendName << "！\n";
        }

        std::cout << "|  " << messages[msgIndex] << "\n";
        std::cout << "========================================\n";

        std::string diaryPath = configPath + "friend_diary.txt";
        std::ifstream diaryFile(diaryPath);
        if (diaryFile.is_open()) {
            std::string line;
            bool hasContent = false;
            std::cout << "\n--- 历史记录 ---\n";
            while (std::getline(diaryFile, line)) {
                if (!line.empty()) {
                    std::cout << "  " << line << "\n";
                    hasContent = true;
                }
            }
            diaryFile.close();
            if (!hasContent) {
                std::cout << "  （暂无记录，写点什么吧）\n";
            }
        }
        else {
            std::cout << "\n--- 暂无日记，写点什么吧 ---\n";
        }

        std::cout << "\n输入要记录的新内容（按 Enter 跳过）:\n> ";
        std::string newEntry;
        std::getline(std::cin, newEntry);

        if (!newEntry.empty()) {
            char dateBuf[32];
            sprintf_s(dateBuf, sizeof(dateBuf), "%04d-%02d-%02d",
                st.wYear, st.wMonth, st.wDay);

            std::ofstream diaryOut(diaryPath, std::ios::app);
            if (diaryOut.is_open()) {
                diaryOut << dateBuf << "：" << newEntry << "\n";
                diaryOut.close();
                std::cout << "\n[日记已保存]\n";
            }
            else {
                std::cout << "\n[保存失败，请检查目录权限]\n";
            }
        }

        std::cout << "\n";
        return true;
    }

    if (cmdTrimmed.substr(0, 7) == "st num " || cmdTrimmed == "st num") {
        if (cmdTrimmed == "st num") {
            std::cout << "用法: st num /1 <表达式>   展开化简\n";
            std::cout << "      st num /2 <表达式>   因式分解\n";
            std::cout << "示例:\n";
            std::cout << "  st num /1 ((2x+3y)^2)((2x-3y)^2)\n";
            std::cout << "  st num /2 4(x^2)-25\n\n";
            return true;
        }

        std::string rest = cmdTrimmed.substr(7);
        size_t s = rest.find_first_not_of(" \t");
        if (s == std::string::npos) {
            std::cout << "用法: st num /1 <表达式>   展开化简\n";
            std::cout << "      st num /2 <表达式>   因式分解\n\n";
            return true;
        }
        rest = rest.substr(s);

        size_t spacePos = rest.find(' ');
        if (spacePos == std::string::npos) {
            std::cout << "错误: 缺少表达式\n";
            std::cout << "用法: st num /1 <表达式>\n";
            std::cout << "      st num /2 <表达式>\n\n";
            return true;
        }

        std::string mode = rest.substr(0, spacePos);
        std::string expr = rest.substr(spacePos + 1);

        size_t es = expr.find_first_not_of(" \t");
        if (es == std::string::npos) {
            std::cout << "错误: 表达式为空\n\n";
            return true;
        }
        expr = expr.substr(es);
        size_t ee = expr.find_last_not_of(" \t");
        if (ee != std::string::npos) expr = expr.substr(0, ee + 1);

        if (mode != "/1" && mode != "/2") {
            std::cout << "错误: 模式必须是 /1（展开化简）或 /2（因式分解）\n\n";
            return true;
        }

        std::string result = STNumCommand(mode, expr);

        std::cout << "\n";
        if (mode == "/1") {
            std::cout << "展开化简: " << expr << "\n";
        }
        else {
            std::cout << "因式分解: " << expr << "\n";
        }
        std::cout << "结果: " << result << "\n\n";
        return true;
    }

    if (cmdTrimmed.find("Jiefangcheng12") != std::string::npos ||
        cmdTrimmed.find("jiefangcheng12") != std::string::npos || cmdTrimmed.find("solve_quadratic") != std::string::npos || cmdTrimmed.find("SOLVE_QUADRATIC") != std::string::npos) {

        size_t spacePos = cmdTrimmed.find(' ');
        if (spacePos != std::string::npos && spacePos + 1 < cmdTrimmed.size()) {
            std::string eq = cmdTrimmed.substr(spacePos + 1);
            size_t s = eq.find_first_not_of(" \t");
            if (s != std::string::npos) eq = eq.substr(s);
            size_t e = eq.find_last_not_of(" \t");
            if (e != std::string::npos) eq = eq.substr(0, e + 1);

            if (!eq.empty()) {
                double a, b, c;
                if (ParseQuadraticEquation(eq, a, b, c)) {
                    SolveQuadraticEquation(a, b, c);
                }
                else {
                    std::cout << "格式错误！请使用格式如: 2x^2+3x-5=0\n";
                }
                std::cout << "\n";
                return true;
            }
        }

        QuadraticEquationSolver();
        return true;
    }

    if (cmdTrimmed == "apt install") {
        std::cout << "用法: apt install \"文件路径\"\n";
        std::cout << "示例: apt install \"D:\\jh-backup.zip\"\n";
        return true;
    }

    if (_strnicmp(cmdTrimmed.c_str(), "apt install ", 12) == 0) {
        std::string param = cmdTrimmed.substr(12);

        size_t start = param.find_first_not_of(" \t");
        if (start != std::string::npos) param = param.substr(start);
        size_t end = param.find_last_not_of(" \t");
        if (end != std::string::npos) param = param.substr(0, end + 1);

        if (param.size() >= 2 && param.front() == '"' && param.back() == '"') {
            param = param.substr(1, param.size() - 2);
        }

        if (param.empty()) {
            std::cout << "用法: apt install \"文件路径\"\n";
            std::cout << "示例: apt install \"D:\\clock.zip\"\n\n";
            return true;
        }

        std::string zipPath = ExpandEnvironmentVars(param);

        DWORD attrs = GetFileAttributesA(zipPath.c_str());
        if (attrs == INVALID_FILE_ATTRIBUTES) {
            std::cout << "错误: 找不到文件 - " << zipPath << "\n\n";
            return true;
        }
        if (attrs & FILE_ATTRIBUTE_DIRECTORY) {
            std::cout << "错误: 路径指向目录，请指定 ZIP 文件\n\n";
            return true;
        }

        std::string lowerPath = zipPath;
        std::transform(lowerPath.begin(), lowerPath.end(), lowerPath.begin(), ::tolower);
        if (lowerPath.size() < 4 || lowerPath.substr(lowerPath.size() - 4) != ".zip") {
            std::cout << "错误: 只支持 .zip 文件\n\n";
            return true;
        }

        std::string zjhCmdDir = GetZJHCMDConfigPath();
        CreateDirectoryA(zjhCmdDir.c_str(), NULL);

        {
            std::string testFile = zjhCmdDir + "_test_write.tmp";
            HANDLE hTest = CreateFileA(testFile.c_str(), GENERIC_WRITE, 0, NULL,
                CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hTest == INVALID_HANDLE_VALUE) {
                std::cout << "错误: 无写入权限 - " << zjhCmdDir << "\n";
                std::cout << "请以管理员身份运行 ZJHCMD\n\n";
                return true;
            }
            CloseHandle(hTest);
            DeleteFileA(testFile.c_str());
        }

        std::string zipName = zipPath;
        size_t lastSlash = zipName.find_last_of("\\/");
        if (lastSlash != std::string::npos) zipName = zipName.substr(lastSlash + 1);
        size_t dotPos = zipName.find_last_of('.');
        std::string moduleName = (dotPos != std::string::npos) ? zipName.substr(0, dotPos) : zipName;

        std::cout << "\n正在安装模块: " << moduleName << "\n";
        std::cout << "  ZIP 文件: " << zipPath << "\n";
        std::cout << "  安装目录: " << zjhCmdDir << "\n\n";

        std::vector<std::string> installedFiles;
        {
            WIN32_FIND_DATAA fd;
            std::string searchPath = zjhCmdDir + "*";
            HANDLE hFind = FindFirstFileA(searchPath.c_str(), &fd);
            if (hFind != INVALID_HANDLE_VALUE) {
                do {
                    if (strcmp(fd.cFileName, ".") != 0 && strcmp(fd.cFileName, "..") != 0) {
                        installedFiles.push_back(fd.cFileName);
                    }
                } while (FindNextFileA(hFind, &fd));
                FindClose(hFind);
            }
        }

        std::cout << "  正在解压...\n";

        std::string psExe = ExpandEnvironmentVars(
            "%SystemRoot%\\System32\\WindowsPowerShell\\v1.0\\powershell.exe");

        DWORD psAttrs = GetFileAttributesA(psExe.c_str());
        if (psAttrs == INVALID_FILE_ATTRIBUTES) {
            psExe = "powershell";
        }

        std::string psCmd = "\"" + psExe + "\" -NoProfile -ExecutionPolicy Bypass -Command "
            "\"Expand-Archive -LiteralPath '" + zipPath + "' -DestinationPath '" + zjhCmdDir + "' -Force\"";

        STARTUPINFOW si = { sizeof(si) };
        PROCESS_INFORMATION pi = { 0 };
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;

        std::wstring wPsCmd = U82W_Path(psCmd);
        std::vector<wchar_t> cmdBuf(wPsCmd.begin(), wPsCmd.end());
        cmdBuf.push_back(L'\0');

        BOOL ok = CreateProcessW(NULL, cmdBuf.data(), NULL, NULL, FALSE,
            CREATE_NO_WINDOW, NULL, NULL, &si, &pi);

        if (!ok) {
            DWORD err = GetLastError();
            std::cout << "错误: 无法启动解压进程 (错误码: " << err << ")\n";
            std::cout << "可能原因:\n";
            std::cout << "  1. PowerShell 不可用\n";
            std::cout << "  2. 命令格式错误\n";
            std::cout << "\n调试: 请手动执行以下命令测试:\n";
            std::cout << "  " << psCmd << "\n\n";
            return true;
        }

        WaitForSingleObject(pi.hProcess, INFINITE);
        DWORD exitCode = 0;
        GetExitCodeProcess(pi.hProcess, &exitCode);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        if (exitCode != 0) {
            std::cout << "错误: 解压失败（退出码: " << exitCode << "）\n";
            std::cout << "可能原因: 权限不够、ZIP 损坏、磁盘空间不足\n\n";
            return true;
        }

        std::cout << "  解压完成\n";

        auto rollback = [&]() {
            std::cout << "  正在回滚...\n";
            WIN32_FIND_DATAA fd;
            std::string searchPath = zjhCmdDir + "*";
            HANDLE hFind = FindFirstFileA(searchPath.c_str(), &fd);
            if (hFind != INVALID_HANDLE_VALUE) {
                do {
                    if (strcmp(fd.cFileName, ".") != 0 && strcmp(fd.cFileName, "..") != 0) {
                        bool existed = false;
                        for (const auto& f : installedFiles) {
                            if (f == fd.cFileName) { existed = true; break; }
                        }
                        if (!existed) {
                            std::string fullPath = zjhCmdDir + fd.cFileName;
                            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                                DeleteDirectoryRecursive(CharToWstring(fullPath.c_str()));
                            }
                            else {
                                SetFileAttributesA(fullPath.c_str(), FILE_ATTRIBUTE_NORMAL);
                                DeleteFileA(fullPath.c_str());
                            }
                        }
                    }
                } while (FindNextFileA(hFind, &fd));
                FindClose(hFind);
            }
            std::cout << "  已回滚\n\n";
            };

        std::string licensePath = zjhCmdDir + "JH_LICENSE";
        DWORD licAttrs = GetFileAttributesA(licensePath.c_str());

        if (licAttrs == INVALID_FILE_ATTRIBUTES) {
            std::cout << "错误: 未找到 JH 许可证文件 (JH_LICENSE)\n";
            rollback();
            return true;
        }

        std::ifstream licFile(licensePath);
        if (!licFile.is_open()) {
            std::cout << "错误: 无法读取 JH 许可证文件\n";
            rollback();
            return true;
        }

        std::vector<std::string> licLines;
        std::string line;
        while (std::getline(licFile, line)) {
            while (!line.empty() && (line.back() == '\r' || line.back() == '\n')) {
                line.pop_back();
            }
            licLines.push_back(line);
        }
        licFile.close();

        if (licLines.empty()) {
            std::cout << "错误: JH 许可证文件为空\n";
            rollback();
            return true;
        }

        if (licLines[0] != "JH-DEVELOPER-GROUP-APPROVED") {
            std::cout << "错误: JH 许可证无效（缺少 JH-DEVELOPER-GROUP-APPROVED 标记）\n";
            rollback();
            return true;
        }

        bool hasModule = false;
        bool hasSignature = false;
        std::string moduleNameInLic;
        std::string signatureInLic;

        for (const auto& l : licLines) {
            if (l.find("Module:") == 0) {
                hasModule = true;
                moduleNameInLic = l.substr(7);
                size_t s = moduleNameInLic.find_first_not_of(" \t");
                if (s != std::string::npos) moduleNameInLic = moduleNameInLic.substr(s);
                size_t e = moduleNameInLic.find_last_not_of(" \t");
                if (e != std::string::npos) moduleNameInLic = moduleNameInLic.substr(0, e + 1);
            }
            if (l.find("Signature:") == 0) {
                hasSignature = true;
                signatureInLic = l.substr(10);
                size_t s = signatureInLic.find_first_not_of(" \t");
                if (s != std::string::npos) signatureInLic = signatureInLic.substr(s);
                size_t e = signatureInLic.find_last_not_of(" \t");
                if (e != std::string::npos) signatureInLic = signatureInLic.substr(0, e + 1);
            }
        }

        if (!hasModule) {
            std::cout << "错误: JH 许可证缺少 Module 字段\n";
            rollback();
            return true;
        }

        if (!hasSignature) {
            std::cout << "错误: JH 许可证缺少 Signature 字段\n";
            rollback();
            return true;
        }

        std::string expectedSig = "JH-" + moduleNameInLic + "-APPROVED";
        if (signatureInLic != expectedSig) {
            std::cout << "错误: JH 许可证签名无效\n";
            std::cout << "  期望: " << expectedSig << "\n";
            std::cout << "  实际: " << signatureInLic << "\n";
            rollback();
            return true;
        }

        std::cout << "  JH 许可证校验通过\n";
        std::cout << "  模块名: " << moduleNameInLic << "\n";
        std::cout << "  签名: " << signatureInLic << "\n";
        std::cout << "\n安装成功！\n";
        std::cout << "  模块 " << moduleNameInLic << " 已安装到 " << zjhCmdDir << "\n";
        std::cout << "  重启 ZJHCMD 后即可使用新命令\n\n";

        return true;
    }

    if (cmdTrimmed.substr(0, 3) == "ver" || cmdTrimmed.substr(0, 3) == "VER") {
        std::cout << "JH Developer Command Prompt " << version << "\n\n";
        return true;
    }

    if (cmdTrimmed == "history" || cmdTrimmed == "HISTORY" || cmdTrimmed == "History") {
        if (g_commandHistory.empty()) {
            std::cout << "没有命令历史\n\n";
        }
        else {
            std::cout << "\n命令历史:\n";
            std::cout << "==========\n";
            for (size_t i = 0; i < g_commandHistory.size(); i++) {
                std::cout << "  " << std::setw(3) << i + 1 << ". " << g_commandHistory[i] << "\n";
            }
            std::cout << "\n";
        }
        return true;
    }

    if (_strnicmp(cmdTrimmed.c_str(), "where", 5) == 0) {
        std::string param = cmdTrimmed.size() > 5 ? cmdTrimmed.substr(5) : "";

        while (!param.empty() && param[0] == ' ') param.erase(0, 1);
        while (!param.empty() && param.back() == ' ') param.pop_back();

        if (param.size() >= 2 && param[0] == '"' && param.back() == '"') {
            param = param.substr(1, param.size() - 2);
        }

        if (param.empty() || param == "/?") {
            std::cout << "WHERE - 显示可执行文件的位置\n\n";
            std::cout << "语法: where <文件名>\n";
            std::cout << "示例: where notepad\n";
            std::cout << "      where myapp.exe\n\n";
            return true;
        }

        const char* exts[] = { ".exe", ".com", ".bat", ".cmd", ".ps1", "" };

        std::vector<std::string> foundPaths;

        auto fileCheck = [](const std::string& path) -> bool {
            DWORD attrs = GetFileAttributesA(path.c_str());
            return (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY));
            };

        char curDir[MAX_PATH];
        GetCurrentDirectoryA(MAX_PATH, curDir);

        for (int i = 0; exts[i] != nullptr; i++) {
            std::string testPath = std::string(curDir) + "\\" + param + exts[i];
            if (fileCheck(testPath)) {
                foundPaths.push_back(testPath);
            }
        }

        char* pathEnv = nullptr;
        size_t pathLen = 0;
        _dupenv_s(&pathEnv, &pathLen, "PATH");

        if (pathEnv != nullptr && pathLen > 0) {
            std::string pathStr(pathEnv);
            free(pathEnv);

            size_t startPos = 0;
            while (startPos < pathStr.length()) {
                size_t endPos = pathStr.find(';', startPos);
                if (endPos == std::string::npos) {
                    endPos = pathStr.length();
                }

                std::string dirName = pathStr.substr(startPos, endPos - startPos);

                if (dirName.length() > 0 && dirName[0] == '"') {
                    dirName = dirName.substr(1);
                }
                if (dirName.length() > 0 && dirName[dirName.length() - 1] == '"') {
                    dirName = dirName.substr(0, dirName.length() - 1);
                }

                if (dirName.length() > 0) {
                    for (int i = 0; exts[i] != nullptr; i++) {
                        std::string testPath = dirName + "\\" + param + exts[i];
                        if (fileCheck(testPath)) {
                            bool alreadyExists = false;
                            for (size_t j = 0; j < foundPaths.size(); j++) {
                                if (foundPaths[j] == testPath) {
                                    alreadyExists = true;
                                    break;
                                }
                            }
                            if (!alreadyExists) {
                                foundPaths.push_back(testPath);
                            }
                        }
                    }
                }

                startPos = endPos + 1;
            }
        }

        char sysDir[MAX_PATH];
        GetSystemDirectoryA(sysDir, MAX_PATH);
        for (int i = 0; exts[i] != nullptr; i++) {
            std::string testPath = std::string(sysDir) + "\\" + param + exts[i];
            if (fileCheck(testPath)) {
                bool alreadyExists = false;
                for (size_t j = 0; j < foundPaths.size(); j++) {
                    if (foundPaths[j] == testPath) {
                        alreadyExists = true;
                        break;
                    }
                }
                if (!alreadyExists) {
                    foundPaths.push_back(testPath);
                }
            }
        }

        if (foundPaths.empty()) {
            std::cout << "找不到文件: " << param << "\n";
        }
        else {
            for (size_t i = 0; i < foundPaths.size(); i++) {
                std::cout << foundPaths[i] << "\n";
            }
        }

        std::cout << "\n";
        return true;
    }

    if (cmdTrimmed.substr(0, 3) == "vol" || cmdTrimmed.substr(0, 3) == "VOL") {
        std::string param;
        if (cmdTrimmed.size() > 3) {
            param = cmdTrimmed.substr(3);
            size_t s = param.find_first_not_of(" \t");
            if (s != std::string::npos) param = param.substr(s);
            size_t e = param.find_last_not_of(" \t\r\n");
            if (e != std::string::npos) param = param.substr(0, e + 1);
        }

        std::wstring drivePath;
        if (param.empty()) {
            wchar_t currentDir[MAX_PATH];
            GetCurrentDirectoryW(MAX_PATH, currentDir);
            drivePath = std::wstring(currentDir, 2);
            drivePath += L"\\";
        }
        else {
            if (param.size() >= 2 && param[1] == ':') {
                std::string driveLetter = param.substr(0, 2);
                drivePath = StringToWString(driveLetter + "\\");
            }
            else {
                std::cout << "无效的驱动器格式。使用格式: X:\n\n";
                return true;
            }
        }

        wchar_t volumeName[MAX_PATH + 1] = { 0 };
        DWORD serialNumber = 0;

        BOOL result = GetVolumeInformationW(
            drivePath.c_str(),
            volumeName,
            MAX_PATH,
            &serialNumber,
            NULL,
            NULL,
            NULL,
            0
        );

        if (!result) {
            DWORD error = GetLastError();
            if (error == ERROR_INVALID_DRIVE) {
                std::cout << "无效的驱动器\n\n";
            }
            else if (error == ERROR_NOT_READY) {
                std::cout << "驱动器未就绪\n\n";
            }
            else {
                std::cout << "获取卷信息失败，错误码: " << error << "\n\n";
            }
            return true;
        }

        std::cout << " 驱动器 " << char(towupper(drivePath[0])) << ": 中的卷是 ";
        if (wcslen(volumeName) == 0) {
            std::cout << "没有卷标";
        }
        else {
            std::wcout << volumeName;
        }
        std::cout << std::endl;

        std::cout << " 卷的序列号是 ";
        std::cout << std::hex << std::uppercase
            << ((serialNumber >> 16) & 0xFFFF) << "-"
            << (serialNumber & 0xFFFF)
            << std::dec << std::nouppercase << std::endl << std::endl;

        return true;
    }

    if (cmdTrimmed.substr(0, 7) == "rainbow" || cmdTrimmed.substr(0, 7) == "RAINBOW" || cmdTrimmed.substr(0, 7) == "Rainbow") {
        if (cmdTrimmed.find("/on") != std::string::npos || cmdTrimmed.find("-on") != std::string::npos || cmdTrimmed.find("/ON") != std::string::npos || cmdTrimmed.find("-ON") != std::string::npos) {
            rainbow = true;
        }
        else if (cmdTrimmed.find("/off") != std::string::npos || cmdTrimmed.find("-off") != std::string::npos || cmdTrimmed.find("/OFF") != std::string::npos || cmdTrimmed.find("-OFF") != std::string::npos) {
            rainbow = false;
        }
        else {
            std::cout << "格式错误！\n\n";
            std::cout << "用法：\n";
            std::cout << "rainbow命令中，-on表示开启彩虹模式，-off表示关闭彩虹模式\n    示例：RAINBOW -ON开启\n";
        }
        return true;
    }

    if (cmdTrimmed.substr(0, 5) == "break" || cmdTrimmed.substr(0, 5) == "BREAK") {
        std::string param = cmdTrimmed.size() > 5 ? cmdTrimmed.substr(5) : "";
        size_t s = param.find_first_not_of(" \t");
        if (s != std::string::npos) param = param.substr(s);

        if (param.empty()) {
            std::cout << "BREAK 是   on/off ?\n";
            std::cout << "（此命令在现代Windows中已无实际作用）\n\n";
        }
        else if (param == "on" || param == "ON") {
            SetEnvironmentVariableA("BREAK", "on");
            std::cout << "BREAK 已设置为 ON（无实际效果）\n\n";
        }
        else if (param == "off" || param == "OFF") {
            SetEnvironmentVariableA("BREAK", "off");
            std::cout << "BREAK 已设置为 OFF（无实际效果）\n\n";
        }
        else {
            std::cout << "无效参数。用法: break [on|off]\n\n";
        }
        return true;
    }

    if (cmdTrimmed.substr(0, 5) == "pushd" || cmdTrimmed.substr(0, 5) == "PUSHD") {
        std::string param = cmdTrimmed.size() > 5 ? cmdTrimmed.substr(5) : "";
        size_t s = param.find_first_not_of(" \t");
        if (s != std::string::npos) param = param.substr(s);

        char currentDir[MAX_PATH];
        GetCurrentDirectoryA(MAX_PATH, currentDir);
        g_directoryStack.push(currentDir);

        if (!silentMode) {
            std::cout << "保存当前目录: " << currentDir << std::endl;
        }

        if (!param.empty()) {
            param = ExpandEnvironmentVars(param);
            if (SetCurrentDirectoryW(U82W_Path(param).c_str())) {
                if (!silentMode) {
                    std::cout << "切换到: " << param << std::endl;
                }
            }
            else {
                std::cout << "系统找不到指定的路径: " << param << std::endl;
                if (!g_directoryStack.empty()) {
                    g_directoryStack.pop();
                }
            }
        }
        std::cout << std::endl;
        return true;
    }

    if (cmdTrimmed.substr(0, 4) == "popd" || cmdTrimmed.substr(0, 4) == "POPD") {
        if (g_directoryStack.empty()) {
            std::cout << "目录栈为空，没有可恢复的目录\n\n";
        }
        else {
            std::string prevDir = g_directoryStack.top();
            g_directoryStack.pop();

            if (SetCurrentDirectoryW(U82W_Path(prevDir).c_str())) {
                if (!silentMode) {
                    std::cout << "已恢复到目录: " << prevDir << std::endl;
                }
            }
            else {
                std::cout << "无法恢复到目录\n";
            }
            std::cout << std::endl;
        }
        return true;
    }

    if (_strnicmp(cmdTrimmed.c_str(), "taskkill", 8) == 0) {
        std::string args = cmdTrimmed.size() > 8 ? cmdTrimmed.substr(8) : "";
        size_t s = args.find_first_not_of(" \t");
        if (s != std::string::npos) args = args.substr(s);

        if (args.empty()) {
            std::cout << "TASKKILL - 终止进程\n\n";
            std::cout << "语法:\n";
            std::cout << "  taskkill /PID 进程ID          - 终止指定PID的进程\n";
            std::cout << "  taskkill /IM 进程名.exe        - 终止指定名称的进程\n";
            std::cout << "  taskkill /F /PID 进程ID        - 强制终止\n";
            std::cout << "  taskkill /F /IM 进程名.exe     - 强制终止\n\n";
            std::cout << "示例:\n";
            std::cout << "  taskkill /PID 1234\n";
            std::cout << "  taskkill /IM notepad.exe\n";
            std::cout << "  taskkill /F /IM chrome.exe\n\n";
            return true;
        }

        bool force = false;
        bool usePid = false;
        bool useIm = false;
        DWORD pid = 0;
        std::string imageName;

        std::string tempArgs = args;

        if (tempArgs.find("/F") != std::string::npos || tempArgs.find("/f") != std::string::npos) {
            force = true;
        }

        size_t pidPos = tempArgs.find("/PID");
        if (pidPos == std::string::npos) pidPos = tempArgs.find("/pid");
        if (pidPos != std::string::npos) {
            usePid = true;
            std::string numStr = tempArgs.substr(pidPos + 5);
            size_t space = numStr.find_first_of(" \t");
            if (space != std::string::npos) numStr = numStr.substr(0, space);
            pid = atoi(numStr.c_str());
        }

        size_t imPos = tempArgs.find("/IM");
        if (imPos == std::string::npos) imPos = tempArgs.find("/im");
        if (imPos != std::string::npos) {
            useIm = true;
            imageName = tempArgs.substr(imPos + 4);
            size_t space = imageName.find_first_of(" \t");
            if (space != std::string::npos) imageName = imageName.substr(0, space);
            if (imageName.size() >= 2 && imageName.front() == '"' && imageName.back() == '"') {
                imageName = imageName.substr(1, imageName.size() - 2);
            }
        }

        if ((!usePid && !useIm) || (usePid && pid == 0) || (useIm && imageName.empty())) {
            std::cout << "参数错误！\n";
            std::cout << "用法: taskkill /PID 进程ID 或 taskkill /IM 进程名.exe\n";
            std::cout << "      taskkill /F /PID 进程ID (强制终止)\n\n";
            return true;
        }

        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot == INVALID_HANDLE_VALUE) {
            std::cout << "无法获取进程列表\n\n";
            return true;
        }

        PROCESSENTRY32W pe;
        pe.dwSize = sizeof(PROCESSENTRY32W);

        std::vector<DWORD> pidsToKill;

        if (usePid) {
            pidsToKill.push_back(pid);
        }
        else if (useIm) {
            std::string imageLower = imageName;
            std::transform(imageLower.begin(), imageLower.end(), imageLower.begin(), ::tolower);

            if (Process32FirstW(hSnapshot, &pe)) {
                do {
                    std::string procName = WCharToString(pe.szExeFile);
                    std::transform(procName.begin(), procName.end(), procName.begin(), ::tolower);
                    if (procName == imageLower) {
                        pidsToKill.push_back(pe.th32ProcessID);
                    }
                } while (Process32NextW(hSnapshot, &pe));
            }
        }

        CloseHandle(hSnapshot);

        if (pidsToKill.empty()) {
            std::cout << "找不到目标进程\n\n";
            return true;
        }

        HANDLE hToken;
        if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
            TOKEN_PRIVILEGES tp;
            LUID luid;
            if (LookupPrivilegeValueW(NULL, SE_DEBUG_NAME, &luid)) {
                tp.PrivilegeCount = 1;
                tp.Privileges[0].Luid = luid;
                tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
                AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);
            }
            CloseHandle(hToken);
        }

        int killed = 0;
        int failed = 0;

        for (DWORD target : pidsToKill) {
            if (target == GetCurrentProcessId()) {
                std::cout << "不能终止自己！\n";
                failed++;
                continue;
            }

            DWORD access = PROCESS_TERMINATE;
            if (force) access |= PROCESS_QUERY_INFORMATION;

            HANDLE hProcess = OpenProcess(access, FALSE, target);
            if (hProcess) {
                if (TerminateProcess(hProcess, 1)) {
                    std::cout << "已终止进程 PID: " << target;
                    if (useIm) {
                        char nameBuf[MAX_PATH];
                        GetModuleFileNameExA(hProcess, NULL, nameBuf, MAX_PATH);
                        char* fileName = strrchr(nameBuf, '\\');
                        if (fileName) std::cout << " (" << (fileName + 1) << ")";
                    }
                    std::cout << "\n";
                    killed++;
                }
                else {
                    std::cout << "终止失败 PID: " << target << "，错误码: " << GetLastError() << "\n";
                    failed++;
                }
                CloseHandle(hProcess);
            }
            else {
                std::cout << "无法打开进程 PID: " << target << "，错误码: " << GetLastError() << "\n";
                failed++;
            }
        }

        std::cout << "\n成功终止: " << killed << " 个进程";
        if (failed > 0) std::cout << "，失败: " << failed << " 个";
        std::cout << "\n\n";

        return true;
    }

    if (_strnicmp(cmdTrimmed.c_str(), "xcopy", 5) == 0) {
        std::string args = cmdTrimmed.size() > 5 ? cmdTrimmed.substr(5) : "";
        size_t s = args.find_first_not_of(" \t");
        if (s != std::string::npos) args = args.substr(s);

        if (args.empty()) {
            std::cout << "XCOPY - 复制文件和目录树\n\n";
            std::cout << "语法: xcopy <源> [目标] [/S] [/E] [/Y] [/D] [/C] [/H] [/R]\n";
            std::cout << "  /S  - 复制目录和子目录（空目录除外）\n";
            std::cout << "  /E  - 复制目录和子目录（包括空目录）\n";
            std::cout << "  /Y  - 不提示直接覆盖\n";
            std::cout << "  /D  - 只复制比目标新的文件\n";
            std::cout << "  /C  - 发生错误时继续\n";
            std::cout << "  /H  - 复制隐藏和系统文件\n";
            std::cout << "  /R  - 覆盖只读文件\n";
            std::cout << "示例: xcopy C:\\src D:\\dst /S /Y\n\n";
            return true;
        }

        bool copySubdirs = false;
        bool includeEmpty = false;
        bool overwriteYes = false;
        bool dateOnly = false;
        bool continueOnError = false;
        bool copyHidden = false;
        bool overwriteReadOnly = false;
        std::string source, destination;

        std::vector<std::string> parts;
        std::string current;
        bool inQuote = false;

        for (char c : args) {
            if (c == '"') {
                inQuote = !inQuote;
                if (!inQuote && !current.empty()) {
                    parts.push_back(current);
                    current.clear();
                }
            }
            else if (c == ' ' && !inQuote) {
                if (!current.empty()) {
                    parts.push_back(current);
                    current.clear();
                }
            }
            else {
                current += c;
            }
        }
        if (!current.empty()) parts.push_back(current);

        for (const auto& p : parts) {
            if (p == "/S" || p == "/s") copySubdirs = true;
            else if (p == "/E" || p == "/e") { copySubdirs = true; includeEmpty = true; }
            else if (p == "/Y" || p == "/y") overwriteYes = true;
            else if (p == "/D" || p == "/d") dateOnly = true;
            else if (p == "/C" || p == "/c") continueOnError = true;
            else if (p == "/H" || p == "/h") copyHidden = true;
            else if (p == "/R" || p == "/r") overwriteReadOnly = true;
            else if (source.empty()) source = p;
            else destination = p;
        }

        if (source.empty()) {
            std::cout << "错误: 必须指定源文件\n\n";
            return true;
        }

        source = ExpandEnvironmentVars(source);
        if (!destination.empty()) destination = ExpandEnvironmentVars(destination);

        if (destination.empty()) {
            char buf[MAX_PATH];
            GetCurrentDirectoryA(MAX_PATH, buf);
            destination = buf;
        }

        bool destIsDir = (destination.back() == '\\' || destination.back() == '/') ||
            (GetFileAttributesW(U82W_Path(destination).c_str()) & FILE_ATTRIBUTE_DIRECTORY);
        if (destIsDir && destination.back() != '\\' && destination.back() != '/')
            destination += "\\";

        int copied = 0;
        int failed = 0;

        std::function<void(const std::wstring&, const std::wstring&, int)> copyDir;
        copyDir = [&](const std::wstring& srcDir, const std::wstring& dstDir, int depth) {
            if (!CreateDirectoryW(dstDir.c_str(), NULL)) {
                DWORD err = GetLastError();
                if (err != ERROR_ALREADY_EXISTS && !continueOnError) {
                    std::wcout << L"无法创建目录: " << dstDir << L"\n";
                    return;
                }
            }

            WIN32_FIND_DATAW fd;
            HANDLE hFind = FindFirstFileW((srcDir + L"*").c_str(), &fd);
            if (hFind == INVALID_HANDLE_VALUE) return;

            do {
                if (wcscmp(fd.cFileName, L".") == 0 || wcscmp(fd.cFileName, L"..") == 0)
                    continue;

                std::wstring srcPath = srcDir + fd.cFileName;
                std::wstring dstPath = dstDir + fd.cFileName;

                if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    if (copySubdirs && (includeEmpty || depth > 0)) {
                        copyDir(srcPath + L"\\", dstPath + L"\\", depth + 1);
                    }
                }
                else {
                    if (!copyHidden && (fd.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM)))
                        continue;

                    bool shouldCopy = true;
                    DWORD dstAttrs = GetFileAttributesW(dstPath.c_str());
                    if (dstAttrs != INVALID_FILE_ATTRIBUTES) {
                        if (dateOnly) {
                            HANDLE hSrc = CreateFileW(srcPath.c_str(), GENERIC_READ, FILE_SHARE_READ,
                                NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                            HANDLE hDst = CreateFileW(dstPath.c_str(), GENERIC_READ, FILE_SHARE_READ,
                                NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                            if (hSrc != INVALID_HANDLE_VALUE && hDst != INVALID_HANDLE_VALUE) {
                                FILETIME srcTime, dstTime;
                                GetFileTime(hSrc, NULL, NULL, &srcTime);
                                GetFileTime(hDst, NULL, NULL, &dstTime);
                                shouldCopy = CompareFileTime(&srcTime, &dstTime) > 0;
                                CloseHandle(hSrc);
                                CloseHandle(hDst);
                            }
                        }

                        if (shouldCopy && !overwriteYes) {
                            std::wcout << L"覆盖 " << fd.cFileName << L"? (Y/N/All): ";
                            char answer = (char)toupper(_getch());
                            std::cout << answer << "\n";
                            if (answer == 'A') overwriteYes = true;
                            else if (answer != 'Y') shouldCopy = false;
                        }

                        if (shouldCopy && overwriteReadOnly && (dstAttrs & FILE_ATTRIBUTE_READONLY)) {
                            SetFileAttributesW(dstPath.c_str(), dstAttrs & ~FILE_ATTRIBUTE_READONLY);
                        }
                    }

                    if (shouldCopy) {
                        if (CopyFileW(srcPath.c_str(), dstPath.c_str(), FALSE)) {
                            std::wcout << L"复制: " << fd.cFileName << L"\n";
                            copied++;
                        }
                        else {
                            std::wcout << L"失败: " << fd.cFileName << L"\n";
                            failed++;
                            if (!continueOnError) {
                                FindClose(hFind);
                                return;
                            }
                        }
                    }
                }
            } while (FindNextFileW(hFind, &fd));
            FindClose(hFind);
            };

        DWORD srcAttrs = GetFileAttributesW(U82W_Path(source).c_str());
        if (srcAttrs & FILE_ATTRIBUTE_DIRECTORY) {
            std::wstring srcDir = U82W_Path(source);
            if (srcDir.back() != L'\\' && srcDir.back() != L'/') srcDir += L"\\";
            copyDir(srcDir, U82W_Path(destination), 0);
        }
        else {
            size_t lastSlash = source.find_last_of("\\/");
            std::wstring wFileName;
            {
                std::string fileName = (lastSlash != std::string::npos) ? source.substr(lastSlash + 1) : source;
                wFileName = U82W_Path(fileName);
            }

            std::wstring wDstPath = U82W_Path(destination);
            if (destIsDir) wDstPath += wFileName;

            if (CopyFileW(U82W_Path(source).c_str(), wDstPath.c_str(), FALSE)) {
                std::wcout << L"复制: " << wFileName << L"\n";
                copied++;
            }
            else {
                std::wcout << L"复制失败: " << wFileName << L"\n";
                failed++;
            }
        }

        std::cout << "\n已复制 " << copied << " 个文件";
        if (failed > 0) std::cout << ", 失败 " << failed << " 个";
        std::cout << "\n\n";
        return true;
    }

    if (_strnicmp(cmdTrimmed.c_str(), "robocopy", 8) == 0) {
        std::string args = cmdTrimmed.size() > 8 ? cmdTrimmed.substr(8) : "";
        size_t s = args.find_first_not_of(" \t");
        if (s != std::string::npos) args = args.substr(s);

        if (args.empty()) {
            std::cout << "ROBOCOPY - 健壮的文件复制\n\n";
            std::cout << "语法: robocopy <源目录> <目标目录> [文件] [选项]\n\n";
            std::cout << "常用选项:\n";
            std::cout << "  /S     - 复制子目录（空目录除外）\n";
            std::cout << "  /E     - 复制子目录（包括空目录）\n";
            std::cout << "  /MIR   - 镜像目录树\n";
            std::cout << "  /R:n   - 失败时重试次数（默认 1,000,000）\n";
            std::cout << "  /W:n   - 重试间等待秒数（默认 30）\n";
            std::cout << "  /NP    - 不显示进度百分比\n";
            std::cout << "  /NJH   - 无作业标头\n";
            std::cout << "  /NJS   - 无作业摘要\n";
            std::cout << "  /LOG:文件 - 输出到日志文件\n\n";
            std::cout << "示例: robocopy C:\\src D:\\dst /MIR /R:3 /W:5\n";
            std::cout << "      robocopy C:\\src D:\\dst *.exe /S\n\n";
            return true;
        }

        std::string source, destination, filePattern;
        bool copySubdirs = false;
        bool includeEmpty = false;
        bool mirror = false;
        int retryCount = 1000000;
        int retryWait = 30;
        bool noProgress = false;
        bool noHeader = false;
        bool noSummary = false;
        bool robocopyContinueOnError = false;
        std::string logFile;

        std::vector<std::string> parts;
        std::string current;
        bool inQuote = false;

        for (char c : args) {
            if (c == '"') {
                inQuote = !inQuote;
                if (!inQuote && !current.empty()) {
                    parts.push_back(current);
                    current.clear();
                }
            }
            else if (c == ' ' && !inQuote) {
                if (!current.empty()) {
                    parts.push_back(current);
                    current.clear();
                }
            }
            else {
                current += c;
            }
        }
        if (!current.empty()) parts.push_back(current);

        for (size_t i = 0; i < parts.size(); i++) {
            const std::string& p = parts[i];
            if (p == "/S" || p == "/s") copySubdirs = true;
            else if (p == "/E" || p == "/e") { copySubdirs = true; includeEmpty = true; }
            else if (p == "/MIR" || p == "/mir") { mirror = true; copySubdirs = true; includeEmpty = true; }
            else if (p.find("/R:") == 0) retryCount = atoi(p.c_str() + 3);
            else if (p.find("/W:") == 0) retryWait = atoi(p.c_str() + 3);
            else if (p == "/NP" || p == "/np") noProgress = true;
            else if (p == "/NJH" || p == "/njh") noHeader = true;
            else if (p == "/NJS" || p == "/njs") noSummary = true;
            else if (p.find("/LOG:") == 0) logFile = p.c_str() + 5;
            else if (source.empty()) source = p;
            else if (destination.empty()) destination = p;
            else if (p == "/C" || p == "/c") robocopyContinueOnError = true;
            else filePattern = p;
        }

        if (source.empty() || destination.empty()) {
            std::cout << "错误: 必须指定源目录和目标目录\n\n";
            return true;
        }

        source = ExpandEnvironmentVars(source);
        destination = ExpandEnvironmentVars(destination);

        std::wstring wSource = U82W_Path(source);
        std::wstring wDestination = U82W_Path(destination);
        std::wstring wFilePattern = filePattern.empty() ? L"*" : U82W_Path(filePattern);

        if (!wSource.empty() && wSource.back() != L'\\' && wSource.back() != L'/')
            wSource += L"\\";
        if (!wDestination.empty() && wDestination.back() != L'\\' && wDestination.back() != L'/')
            wDestination += L"\\";

        long long copiedFiles = 0, skippedFiles = 0, failedFiles = 0;
        long long copiedBytes = 0;
        DWORD startTime = GetTickCount();

        std::ofstream logStream;
        if (!logFile.empty()) {
            logStream.open(logFile);
            if (!logStream.is_open()) {
                std::cout << "无法创建日志文件: " << logFile << "\n";
            }
        }

        auto log = [&](const std::string& msg) {
            if (!noHeader && !noProgress) std::cout << msg;
            if (logStream.is_open()) logStream << msg;
            };

        if (!noHeader) {
            log("\n-------------------------------------------------------------------------------\n");
            log("   ROBOCOPY     ::     健壮的文件复制\n");
            log("\n-------------------------------------------------------------------------------\n\n");
            log("  源目录: " + source + "\n");
            log("  目标目录: " + destination + "\n");
            if (!filePattern.empty()) log("  文件模式: " + filePattern + "\n");
            log("\n");
        }

        std::function<void(const std::wstring&, const std::wstring&, int)> robocopyDir;
        robocopyDir = [&](const std::wstring& srcDir, const std::wstring& dstDir, int depth) {
            if (includeEmpty || depth > 0) {
                if (!CreateDirectoryW(dstDir.c_str(), NULL)) {
                    DWORD err = GetLastError();
                    if (err != ERROR_ALREADY_EXISTS && !robocopyContinueOnError) {
                        std::wcout << L"错误: 无法创建目录 " << dstDir << L"\n";
                        return;
                    }
                }
            }

            WIN32_FIND_DATAW fd;
            std::wstring searchPattern = srcDir + wFilePattern;
            HANDLE hFind = FindFirstFileW(searchPattern.c_str(), &fd);

            if (hFind != INVALID_HANDLE_VALUE) {
                do {
                    if (wcscmp(fd.cFileName, L".") == 0 || wcscmp(fd.cFileName, L"..") == 0)
                        continue;

                    if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                        std::wstring srcFile = srcDir + fd.cFileName;
                        std::wstring dstFile = dstDir + fd.cFileName;

                        bool success = false;
                        for (int retry = 0; retry <= retryCount && !success; retry++) {
                            if (retry > 0) {
                                std::wcout << L"重试 " << retry << L"/" << retryCount << L": " << fd.cFileName << L"\n";
                                Sleep(retryWait * 1000);
                            }

                            if (CopyFileW(srcFile.c_str(), dstFile.c_str(), FALSE)) {
                                success = true;
                                copiedFiles++;
                                HANDLE hFile = CreateFileW(srcFile.c_str(), GENERIC_READ, FILE_SHARE_READ,
                                    NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                                if (hFile != INVALID_HANDLE_VALUE) {
                                    LARGE_INTEGER fileSize;
                                    if (GetFileSizeEx(hFile, &fileSize)) {
                                        copiedBytes += fileSize.QuadPart;
                                    }
                                    CloseHandle(hFile);
                                }
                                if (!noProgress) {
                                    std::wcout << L"复制: " << fd.cFileName << L"\n";
                                }
                            }
                            else if (GetLastError() == ERROR_FILE_EXISTS) {
                                if (mirror) {
                                    SetFileAttributesW(dstFile.c_str(), FILE_ATTRIBUTE_NORMAL);
                                    if (DeleteFileW(dstFile.c_str())) {
                                        std::wcout << L"删除: " << fd.cFileName << L"\n";
                                        continue;
                                    }
                                }
                                skippedFiles++;
                                success = true;
                            }
                        }
                        if (!success) {
                            failedFiles++;
                            std::wcout << L"失败: " << fd.cFileName << L"\n";
                        }
                    }
                } while (FindNextFileW(hFind, &fd));
                FindClose(hFind);
            }

            if (copySubdirs) {
                hFind = FindFirstFileW((srcDir + L"*").c_str(), &fd);
                if (hFind != INVALID_HANDLE_VALUE) {
                    do {
                        if (wcscmp(fd.cFileName, L".") == 0 || wcscmp(fd.cFileName, L"..") == 0)
                            continue;
                        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                            std::wstring subSrc = srcDir + fd.cFileName + L"\\";
                            std::wstring subDst = dstDir + fd.cFileName + L"\\";
                            robocopyDir(subSrc, subDst, depth + 1);
                        }
                    } while (FindNextFileW(hFind, &fd));
                    FindClose(hFind);
                }
            }

            if (mirror) {
                hFind = FindFirstFileW((dstDir + L"*").c_str(), &fd);
                if (hFind != INVALID_HANDLE_VALUE) {
                    do {
                        if (wcscmp(fd.cFileName, L".") == 0 || wcscmp(fd.cFileName, L"..") == 0)
                            continue;

                        std::wstring dstPath = dstDir + fd.cFileName;
                        std::wstring srcPath = srcDir + fd.cFileName;

                        if (GetFileAttributesW(srcPath.c_str()) == INVALID_FILE_ATTRIBUTES) {
                            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                                DeleteDirectoryRecursive(dstPath);
                                std::wcout << L"删除目录: " << fd.cFileName << L"\n";
                            }
                            else {
                                SetFileAttributesW(dstPath.c_str(), FILE_ATTRIBUTE_NORMAL);
                                if (DeleteFileW(dstPath.c_str())) {
                                    std::wcout << L"删除文件: " << fd.cFileName << L"\n";
                                }
                            }
                        }
                    } while (FindNextFileW(hFind, &fd));
                    FindClose(hFind);
                }
            }
            };

        robocopyDir(wSource, wDestination, 0);

        DWORD endTime = GetTickCount();
        DWORD elapsed = (endTime - startTime) / 1000;

        if (!noSummary) {
            log("\n-------------------------------------------------------------------------------\n");
            log("                摘要\n");
            log("-------------------------------------------------------------------------------\n");
            log("  已复制: " + std::to_string(copiedFiles) + " 个文件\n");
            log("  已跳过: " + std::to_string(skippedFiles) + " 个文件\n");
            log("  失败: " + std::to_string(failedFiles) + " 个文件\n");

            if (copiedBytes > 0) {
                if (copiedBytes < 1024)
                    log("  字节数: " + std::to_string(copiedBytes) + " 字节\n");
                else if (copiedBytes < 1024 * 1024)
                    log("  字节数: " + std::to_string(copiedBytes / 1024) + " KB\n");
                else
                    log("  字节数: " + std::to_string(copiedBytes / (1024 * 1024)) + " MB\n");
            }

            log("  用时: " + std::to_string(elapsed) + " 秒\n");
            log("-------------------------------------------------------------------------------\n");
        }

        if (logStream.is_open()) logStream.close();

        if (!noProgress) {
            std::cout << "\n复制完成: " << copiedFiles << " 个文件";
            if (failedFiles > 0) std::cout << ", 失败 " << failedFiles << " 个";
            std::cout << "\n\n";
        }

        return true;
    }

    if (_strnicmp(cmdTrimmed.c_str(), "replace", 7) == 0) {
        std::string args = cmdTrimmed.size() > 7 ? cmdTrimmed.substr(7) : "";
        size_t s = args.find_first_not_of(" \t");
        if (s != std::string::npos) args = args.substr(s);

        if (args.empty()) {
            std::cout << "用法: replace <源文件> <目标目录> [/A] [/R] [/S] [/U]\n";
            std::cout << "  /A  - 添加新文件（不替换现有文件）\n";
            std::cout << "  /R  - 替换只读文件\n";
            std::cout << "  /S  - 搜索子目录\n";
            std::cout << "  /U  - 只替换较旧的文件\n";
            std::cout << "示例: replace C:\\new\\*.txt D:\\backup /S\n\n";
            return true;
        }

        bool addOnly = false;
        bool replaceReadOnly = false;
        bool recursive = false;
        bool updateOnly = false;
        std::string source, destination;

        std::vector<std::string> parts;
        std::string current;
        bool inQuote = false;

        for (char c : args) {
            if (c == '"') {
                inQuote = !inQuote;
                if (!inQuote && !current.empty()) {
                    parts.push_back(current);
                    current.clear();
                }
            }
            else if (c == ' ' && !inQuote) {
                if (!current.empty()) {
                    parts.push_back(current);
                    current.clear();
                }
            }
            else {
                current += c;
            }
        }
        if (!current.empty()) parts.push_back(current);

        for (const auto& p : parts) {
            if (p == "/A" || p == "/a") addOnly = true;
            else if (p == "/R" || p == "/r") replaceReadOnly = true;
            else if (p == "/S" || p == "/s") recursive = true;
            else if (p == "/U" || p == "/u") updateOnly = true;
            else if (source.empty()) source = p;
            else destination = p;
        }

        if (source.empty() || destination.empty()) {
            std::cout << "错误: 必须指定源文件和目标目录\n\n";
            return true;
        }

        source = ExpandEnvironmentVars(source);
        destination = ExpandEnvironmentVars(destination);

        if (!destination.empty() && destination.back() != '\\' && destination.back() != '/')
            destination += "\\";

        std::string dirPath;
        std::string pattern;
        size_t lastSlash = source.find_last_of("\\/");
        if (lastSlash != std::string::npos) {
            dirPath = source.substr(0, lastSlash + 1);
            pattern = source.substr(lastSlash + 1);
        }
        else {
            char buf[MAX_PATH];
            GetCurrentDirectoryA(MAX_PATH, buf);
            dirPath = std::string(buf) + "\\";
            pattern = source;
        }

        std::wstring wDestination = U82W_Path(destination);

        std::function<void(const std::wstring&)> replaceInDir = [&](const std::wstring& currentDir) {
            WIN32_FIND_DATAW fd;
            HANDLE hFind = FindFirstFileW((currentDir + U82W_Path(pattern)).c_str(), &fd);
            if (hFind == INVALID_HANDLE_VALUE) return;

            do {
                if (wcscmp(fd.cFileName, L".") == 0 || wcscmp(fd.cFileName, L"..") == 0)
                    continue;

                if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    std::wstring srcFile = currentDir + fd.cFileName;
                    std::wstring dstFile = wDestination + fd.cFileName;

                    DWORD dstAttrs = GetFileAttributesW(dstFile.c_str());
                    bool dstExists = (dstAttrs != INVALID_FILE_ATTRIBUTES);

                    bool shouldReplace = false;
                    if (addOnly && !dstExists) {
                        shouldReplace = true;
                    }
                    else if (!addOnly && dstExists) {
                        if (updateOnly) {
                            HANDLE hSrc = CreateFileW(srcFile.c_str(), GENERIC_READ, FILE_SHARE_READ,
                                NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                            HANDLE hDst = CreateFileW(dstFile.c_str(), GENERIC_READ, FILE_SHARE_READ,
                                NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                            if (hSrc != INVALID_HANDLE_VALUE && hDst != INVALID_HANDLE_VALUE) {
                                FILETIME srcTime, dstTime;
                                GetFileTime(hSrc, NULL, NULL, &srcTime);
                                GetFileTime(hDst, NULL, NULL, &dstTime);
                                shouldReplace = CompareFileTime(&srcTime, &dstTime) > 0;
                                CloseHandle(hSrc);
                                CloseHandle(hDst);
                            }
                        }
                        else {
                            shouldReplace = true;
                        }

                        if (shouldReplace && (dstAttrs & FILE_ATTRIBUTE_READONLY)) {
                            if (replaceReadOnly) {
                                SetFileAttributesW(dstFile.c_str(), dstAttrs & ~FILE_ATTRIBUTE_READONLY);
                            }
                            else {
                                shouldReplace = false;
                                std::wcout << L"跳过只读文件: " << fd.cFileName << L"\n";
                            }
                        }
                    }

                    if (shouldReplace) {
                        if (CopyFileW(srcFile.c_str(), dstFile.c_str(), FALSE)) {
                            std::wcout << (addOnly ? L"已添加: " : L"已替换: ") << fd.cFileName << L"\n";
                        }
                        else {
                            std::wcout << L"失败: " << fd.cFileName << L"\n";
                        }
                    }
                }
            } while (FindNextFileW(hFind, &fd));
            FindClose(hFind);
            };

        replaceInDir(U82W_Path(dirPath));

        if (recursive) {
            std::function<void(const std::wstring&)> walkDir = [&](const std::wstring& currentDir) {
                WIN32_FIND_DATAW fd;
                HANDLE hFind = FindFirstFileW((currentDir + L"*").c_str(), &fd);
                if (hFind == INVALID_HANDLE_VALUE) return;

                do {
                    if (wcscmp(fd.cFileName, L".") == 0 || wcscmp(fd.cFileName, L"..") == 0)
                        continue;
                    if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                        std::wstring subDir = currentDir + fd.cFileName + L"\\";
                        replaceInDir(subDir);
                        walkDir(subDir);
                    }
                } while (FindNextFileW(hFind, &fd));
                FindClose(hFind);
                };
            walkDir(U82W_Path(dirPath));
        }

        std::cout << "\n";
        return true;
    }

    if (_strnicmp(cmdTrimmed.c_str(), "path", 4) == 0) {
        std::string param = cmdTrimmed.size() > 4 ? cmdTrimmed.substr(4) : "";
        size_t s = param.find_first_not_of(" \t");
        if (s != std::string::npos) param = param.substr(s);

        if (param.empty() || param == ";") {
            char* pathValue = nullptr;
            size_t len = 0;
            _dupenv_s(&pathValue, &len, "PATH");
            if (pathValue && strlen(pathValue) > 0) {
                std::cout << "PATH=" << pathValue << "\n\n";

                std::string pathStr = pathValue;
                size_t pos = 0;
                int idx = 1;
                std::cout << "搜索路径:\n";
                while (pos < pathStr.size()) {
                    size_t semi = pathStr.find(';', pos);
                    if (semi == std::string::npos) semi = pathStr.size();
                    std::string dir = pathStr.substr(pos, semi - pos);
                    if (!dir.empty()) {
                        std::cout << "  " << idx++ << ". " << dir << "\n";
                    }
                    pos = semi + 1;
                }
            }
            else {
                std::cout << "PATH 未设置\n";
            }
            free(pathValue);
            std::cout << "\n";
        }
        else if (param.find("=") != std::string::npos) {
            std::string newPath = param.substr(param.find('=') + 1);
            newPath = ExpandEnvironmentVars(newPath);
            if (SetEnvironmentVariableA("PATH", newPath.c_str())) {
                std::cout << "PATH 已设置\n\n";
            }
            else {
                std::cout << "设置 PATH 失败\n\n";
            }
        }
        else {
            std::string addPath = ExpandEnvironmentVars(param);
            char* currentPath = nullptr;
            size_t len = 0;
            _dupenv_s(&currentPath, &len, "PATH");
            std::string newPath;
            if (currentPath && strlen(currentPath) > 0) {
                newPath = std::string(currentPath) + ";" + addPath;
            }
            else {
                newPath = addPath;
            }
            free(currentPath);
            if (SetEnvironmentVariableA("PATH", newPath.c_str())) {
                std::cout << "已添加路径到 PATH: " << addPath << "\n\n";
            }
            else {
                std::cout << "添加路径失败\n\n";
            }
        }
        return true;
    }

    if (_strnicmp(cmdTrimmed.c_str(), "ren", 3) == 0 ||
        _strnicmp(cmdTrimmed.c_str(), "rename", 6) == 0) {

        std::string args = cmdTrimmed.substr(cmdTrimmed[0] == 'r' ? 3 : 6);
        size_t s = args.find_first_not_of(" \t");
        if (s != std::string::npos) args = args.substr(s);
        else {
            std::cout << "用法: ren <旧文件名> <新文件名>\n";
            std::cout << "      rename <旧文件名> <新文件名>\n";
            std::cout << "示例: ren old.txt new.txt\n";
            std::cout << "      rename \"C:\\my file.txt\" \"new file.txt\"\n\n";
            return true;
        }

        std::string oldName, newName;
        bool inQuote = false;
        std::string current;
        int part = 0;

        for (size_t i = 0; i <= args.size(); i++) {
            char c = (i < args.size()) ? args[i] : ' ';
            if (c == '"') {
                inQuote = !inQuote;
                if (!inQuote && !current.empty()) {
                    if (part == 0) oldName = current;
                    else if (part == 1) newName = current;
                    current.clear();
                    part++;
                }
            }
            else if (c == ' ' && !inQuote) {
                if (!current.empty()) {
                    if (part == 0) oldName = current;
                    else if (part == 1) newName = current;
                    current.clear();
                    part++;
                }
            }
            else {
                current += c;
            }
        }

        if (oldName.empty()) {
            std::cout << "错误: 未指定源文件名\n\n";
            return true;
        }
        if (newName.empty()) {
            std::cout << "错误: 未指定目标文件名\n\n";
            return true;
        }

        oldName = ExpandEnvironmentVars(oldName);
        newName = ExpandEnvironmentVars(newName);

        bool hasWildcard = (oldName.find('*') != std::string::npos ||
            oldName.find('?') != std::string::npos);

        if (hasWildcard) {
            std::string dirPath;
            std::string pattern;
            size_t lastSlash = oldName.find_last_of("\\/");
            if (lastSlash != std::string::npos) {
                dirPath = oldName.substr(0, lastSlash + 1);
                pattern = oldName.substr(lastSlash + 1);
            }
            else {
                char buf[MAX_PATH];
                GetCurrentDirectoryA(MAX_PATH, buf);
                dirPath = std::string(buf) + "\\";
                pattern = oldName;
            }

            std::wstring wDirPath = U82W_Path(dirPath);
            std::wstring wPattern = U82W_Path(pattern);
            std::wstring wNewName = U82W_Path(newName);

            WIN32_FIND_DATAW fd;
            HANDLE hFind = FindFirstFileW((wDirPath + wPattern).c_str(), &fd);
            if (hFind == INVALID_HANDLE_VALUE) {
                std::cout << "找不到文件: " << oldName << "\n\n";
                return true;
            }

            int count = 0;
            do {
                if (wcscmp(fd.cFileName, L".") == 0 || wcscmp(fd.cFileName, L"..") == 0)
                    continue;

                std::wstring wFullOld = wDirPath + fd.cFileName;
                std::wstring wFullNew = wDirPath + wNewName;

                size_t starPos = wFullNew.find(L'*');
                if (starPos != std::wstring::npos) {
                    std::wstring baseName = fd.cFileName;
                    size_t dotPos = baseName.find_last_of(L'.');
                    std::wstring namePart = (dotPos != std::wstring::npos) ? baseName.substr(0, dotPos) : baseName;
                    wFullNew.replace(starPos, 1, namePart);
                }

                if (MoveFileW(wFullOld.c_str(), wFullNew.c_str())) {
                    std::wcout << L"已重命名: " << fd.cFileName << L" -> " << wFullNew << L"\n";
                    count++;
                }
            } while (FindNextFileW(hFind, &fd));
            FindClose(hFind);
            std::cout << "共重命名 " << count << " 个文件\n\n";
        }
        else {
            if (MoveFileW(U82W_Path(oldName).c_str(), U82W_Path(newName).c_str())) {
                std::cout << "已重命名: " << oldName << " -> " << newName << "\n\n";
            }
            else {
                DWORD err = GetLastError();
                if (err == ERROR_FILE_NOT_FOUND)
                    std::cout << "找不到文件: " << oldName << "\n\n";
                else if (err == ERROR_ALREADY_EXISTS)
                    std::cout << "目标文件已存在: " << newName << "\n\n";
                else
                    std::cout << "重命名失败，错误码: " << err << "\n\n";
            }
        }
        return true;
    }

    std::string lowerCmdForEcho = cmdTrimmed;
    std::transform(lowerCmdForEcho.begin(), lowerCmdForEcho.end(), lowerCmdForEcho.begin(), ::tolower);
    if (cmdTrimmed.substr(0, 5) == "echo " || cmdTrimmed.substr(0, 5) == "ECHO ") {
        std::string msg = cmdTrimmed.substr(5);

        if (!msg.empty() && (msg[0] == '.' || msg[0] == ',' || msg[0] == ';' ||
            msg[0] == '=' || msg[0] == '+' || msg[0] == '/' || msg[0] == ':')) {
            std::cout << "\n";
            std::cout.flush();
            return true;
        }

        size_t startMsg = msg.find_first_not_of(" \t");
        if (startMsg != std::string::npos) msg = msg.substr(startMsg);
        size_t endMsg = msg.find_last_not_of(" \t");
        if (endMsg != std::string::npos) msg = msg.substr(0, endMsg + 1);

        if (msg.size() >= 2 && msg.front() == '"' && msg.back() == '"') {
            msg = msg.substr(1, msg.size() - 2);
        }

        msg = ExpandEnvironmentVars(msg);

        std::cout << msg << "\n";
        std::cout.flush();
        return true;
    }

    if (_strnicmp(cmdTrimmed.c_str(), "_admin_", 7) == 0 || _strnicmp(cmdTrimmed.c_str(), "_Admin_", 7) == 0 || _strnicmp(cmdTrimmed.c_str(), "_ADMIN_", 7) == 0) {
        if (IsRunningAsAdmin()) {

        }
        else {
            wchar_t szPath[MAX_PATH];
            GetModuleFileNameW(nullptr, szPath, MAX_PATH);
            SHELLEXECUTEINFOW sei = { sizeof(sei) };
            sei.lpVerb = L"runas";
            sei.lpFile = szPath;
            sei.nShow = SW_SHOWNORMAL;
            if (!ShellExecuteExW(&sei)) {
                DWORD err = GetLastError();
                if (err == ERROR_CANCELLED) {

                }
                else {

                }
            }
        }
        return true;
    }

    if (_strnicmp(cmdTrimmed.c_str(), "ctty", 4) == 0) {
        std::string param = cmdTrimmed.size() > 4 ? cmdTrimmed.substr(4) : "";
        size_t s = param.find_first_not_of(" \t");
        if (s != std::string::npos) param = param.substr(s);
        else param = "";

        if (param.size() >= 2 && param.front() == '"' && param.back() == '"') {
            param = param.substr(1, param.size() - 2);
        }

        if (param.empty()) {
            std::cout << "当前控制台设备: CON\n";
            std::cout << "(ctty 命令在现代 Windows 中已废弃)\n\n";
            return true;
        }

        std::string paramUpper = param;
        std::transform(paramUpper.begin(), paramUpper.end(), paramUpper.begin(), ::toupper);

        if (paramUpper == "CON" || paramUpper == "CONIN$" || paramUpper == "CONOUT$") {
            std::cout << "控制台设备已设置为 " << param << "\n";
            std::cout << "(ctty 命令在现代 Windows 中已废弃，此操作为空操作)\n\n";
        }
        else if (paramUpper.find("COM") == 0 || paramUpper.find("AUX") == 0 ||
            paramUpper.find("PRN") == 0 || paramUpper.find("LPT") == 0) {
            std::cout << "警告: ctty " << param << " 命令已过时\n";
            std::cout << "此命令用于 MS-DOS 时代切换控制台到串口设备\n";
            std::cout << "在现代 Windows 中无效，请使用其他方法:\n";
            std::cout << "  - 使用 mode 命令配置串口\n";
            std::cout << "  - 使用 PowerShell 的 COM 端口操作\n\n";
        }
        else {
            std::cout << "无效的设备名称: " << param << "\n";
            std::cout << "用法: ctty [设备]\n";
            std::cout << "有效设备: CON, AUX, COM1-COM9, PRN, LPT1-LPT9\n";
            std::cout << "(此命令已废弃，仅用于兼容性)\n\n";
        }
        return true;
    }

    if (_strnicmp(cmdTrimmed.c_str(), "chcp", 4) == 0) {
        std::string param = cmdTrimmed.size() > 4 ? cmdTrimmed.substr(4) : "";
        size_t s = param.find_first_not_of(" \t");
        if (s != std::string::npos) param = param.substr(s);

        if (param.empty()) {
            UINT currentCP = GetConsoleOutputCP();
            std::cout << "活动代码页: " << currentCP << "\n\n";
        }
        else {
            int newCodePage = atoi(param.c_str());
            if (newCodePage <= 0) {
                std::cout << "无效的参数: " << param << "\n";
                std::cout << "用法: chcp [nnn]\n";
                std::cout << "  nnn    - 指定代码页编号\n";
                std::cout << "常用代码页:\n";
                std::cout << "  437    - 美国英语 (OEM)\n";
                std::cout << "  65001  - UTF-8\n";
                std::cout << "  936    - 简体中文 (GBK)\n";
                std::cout << "  950    - 繁体中文 (Big5)\n";
                std::cout << "  1252   - 西欧 (ANSI)\n\n";
                return true;
            }

            BOOL resultIn = SetConsoleCP(newCodePage);
            BOOL resultOut = SetConsoleOutputCP(newCodePage);

            if (resultIn && resultOut) {
                std::cout << "活动代码页: " << newCodePage << "\n\n";
            }
            else {
                DWORD err = GetLastError();
                std::cout << "设置代码页失败，错误码: " << err << "\n";
                std::cout << "可能不支持的代码页或需要管理员权限\n\n";
            }
        }
        return true;
    }

    if (_strnicmp(cmdTrimmed.c_str(), "home", 4) == 0) {
        std::string param = cmdTrimmed.size() > 4 ? cmdTrimmed.substr(4) : "";
        size_t s = param.find_first_not_of(" \t");
        if (s != std::string::npos) param = param.substr(s);

        std::string userProfile = "%USERPROFILE%";
        std::string homePath = ExpandEnvironmentVars(userProfile);

        if (homePath.empty() || homePath == "%USERPROFILE%") {
            char* username = nullptr;
            size_t len = 0;
            _dupenv_s(&username, &len, "USERNAME");
            if (username) {
                homePath = "C:\\Users\\" + std::string(username);
                free(username);
            }
            else {
                std::cout << "无法获取用户目录\n\n";
                return true;
            }
        }

        bool openExplorer = false;
        if (param == "/e" || param == "/E" || param == "-e") {
            openExplorer = true;
        }

        if (openExplorer) {
            std::string cmd = "explorer \"" + homePath + "\"";
            SHELLEXECUTEINFOA sei = { sizeof(sei) };
            sei.lpVerb = "open";
            sei.lpFile = homePath.c_str();
            sei.nShow = SW_SHOWNORMAL;
            ShellExecuteExA(&sei);
            std::cout << "已在资源管理器中打开用户目录\n\n";
        }
        else {
            if (SetCurrentDirectoryA(homePath.c_str())) {
                std::cout << "已切换到用户目录: " << homePath << "\n\n";
            }
            else {
                std::cout << "切换失败！\n\n";
            }
        }

        return true;
    }

    if (cmdTrimmed == "~" || cmdTrimmed == "cd ~") {
        std::string userProfile = "%USERPROFILE%";
        std::string homePath = ExpandEnvironmentVars(userProfile);

        if (homePath.empty() || homePath == "%USERPROFILE%") {
            char* username = nullptr;
            size_t len = 0;
            _dupenv_s(&username, &len, "USERNAME");
            if (username) {
                homePath = "C:\\Users\\" + std::string(username);
                free(username);
            }
            else {
                std::cout << "无法获取用户目录\n\n";
                return true;
            }
        }

        if (SetCurrentDirectoryA(homePath.c_str())) {
            std::cout << "已切换到用户目录: " << homePath << "\n\n";
        }
        else {
            std::cout << "切换失败！\n\n";
        }

        return true;
    }

    if (_strnicmp(cmdTrimmed.c_str(), "hex", 3) == 0) {
        std::string param = cmdTrimmed.size() > 3 ? cmdTrimmed.substr(3) : "";
        size_t s = param.find_first_not_of(" \t");
        if (s == std::string::npos || param.empty()) {
            std::cout << "用法: hex <十进制数>\n";
            std::cout << "      hex 0x<十六进制>\n";
            std::cout << "示例: hex 255\n";
            std::cout << "      hex 0xFF\n\n";
            return true;
        }
        param = param.substr(s);

        if (param.size() >= 2 && param.front() == '"' && param.back() == '"') {
            param = param.substr(1, param.size() - 2);
        }

        if (param.size() >= 2 && (param[0] == '0' && (param[1] == 'x' || param[1] == 'X'))) {
            std::string dec = HexToDec(param);
            std::cout << param << " = " << dec << " (十进制)\n\n";
            return true;
        }

        bool isDecimal = true;
        for (char c : param) {
            if (!isdigit(c)) {
                isDecimal = false;
                break;
            }
        }

        if (isDecimal) {
            try {
                unsigned int num = std::stoul(param);
                std::cout << num << " = " << DecToHex(num) << " (十六进制)\n\n";
            }
            catch (...) {
                std::cout << "错误：无效的数字格式\n\n";
            }
        }
        else if (IsHexString(param)) {
            std::string dec = HexToDec(param);
            std::cout << param << " = " << dec << " (十进制)\n\n";
        }
        else {
            std::cout << "错误：无效的十六进制数\n\n";
        }
        return true;
    }

    if (_strnicmp(cmdTrimmed.c_str(), "dec", 3) == 0) {
        std::string param = cmdTrimmed.size() > 3 ? cmdTrimmed.substr(3) : "";
        size_t s = param.find_first_not_of(" \t");
        if (s == std::string::npos || param.empty()) {
            std::cout << "用法: dec <十六进制数>\n";
            std::cout << "      dec 0x<十六进制>\n";
            std::cout << "      dec <十进制数>\n";
            std::cout << "示例: dec FF\n";
            std::cout << "      dec 0xFF\n";
            std::cout << "      dec 255\n\n";
            return true;
        }
        param = param.substr(s);

        if (param.size() >= 2 && param.front() == '"' && param.back() == '"') {
            param = param.substr(1, param.size() - 2);
        }

        if (param.size() >= 2 && (param[0] == '0' && (param[1] == 'x' || param[1] == 'X'))) {
            std::string dec = HexToDec(param);
            std::cout << param << " = " << dec << " (十进制)\n\n";
            return true;
        }

        bool isDecimal = true;
        for (char c : param) {
            if (!isdigit(c)) {
                isDecimal = false;
                break;
            }
        }

        if (isDecimal) {
            try {
                unsigned int num = std::stoul(param);
                std::cout << param << " = " << DecToHex(num) << " (十六进制)\n\n";
            }
            catch (...) {
                std::cout << "错误：无效的数字格式\n\n";
            }
        }
        else if (IsHexString(param)) {
            std::string dec = HexToDec(param);
            std::cout << param << " = " << dec << " (十进制)\n\n";
        }
        else {
            std::cout << "错误：无效的十六进制数\n\n";
        }
        return true;
    }

    if (_strnicmp(cmdTrimmed.c_str(), "bin", 3) == 0) {
        std::string param = cmdTrimmed.size() > 3 ? cmdTrimmed.substr(3) : "";
        size_t s = param.find_first_not_of(" \t");
        if (s == std::string::npos || param.empty()) {
            std::cout << "用法: bin <十进制数>\n";
            std::cout << "      bin 0x<十六进制>\n";
            std::cout << "示例: bin 255\n";
            std::cout << "      bin 0xFF\n\n";
            return true;
        }
        param = param.substr(s);

        if (param.size() >= 2 && param.front() == '"' && param.back() == '"') {
            param = param.substr(1, param.size() - 2);
        }

        unsigned int num;

        if (param.size() >= 2 && (param[0] == '0' && (param[1] == 'x' || param[1] == 'X'))) {
            std::string decStr = HexToDec(param);
            num = std::stoul(decStr);
        }
        else {
            bool isDecimal = true;
            for (char c : param) {
                if (!isdigit(c)) {
                    isDecimal = false;
                    break;
                }
            }

            if (isDecimal) {
                try {
                    num = std::stoul(param);
                }
                catch (...) {
                    std::cout << "错误：无效的数字格式\n\n";
                    return true;
                }
            }
            else {
                if (IsHexString(param)) {
                    std::string decStr = HexToDec(param);
                    num = std::stoul(decStr);
                }
                else {
                    std::cout << "错误：无效的十六进制数\n\n";
                    return true;
                }
            }
        }

        if (num == 0) {
            std::cout << "0 = 0b0\n\n";
            return true;
        }

        std::string binary;
        unsigned int n = num;
        while (n > 0) {
            binary = (n % 2 == 0 ? "0" : "1") + binary;
            n /= 2;
        }

        std::cout << num << " = 0b" << binary << " (二进制)\n\n";
        return true;
    }

    if (cmdTrimmed.substr(0, 12) == "st translate" || cmdTrimmed.substr(0, 12) == "ST TRANSLATE") {
        std::string param = cmdTrimmed.size() > 12 ? cmdTrimmed.substr(12) : "";
        HandleTranslate(param);
        return true;
    }

    if (cmdTrimmed == "st news" || cmdTrimmed == "ST NEWS" || cmdTrimmed == "st News") {
        HandleNews();
        return true;
    }

    if (cmdTrimmed.substr(0, 7) == "_askai " || cmdTrimmed.substr(0, 7) == "_ASKAI ") {
        std::string input = cmdTrimmed.substr(7);
        AskAIParams params = ParseAskAICommand(input);

        if (params.question == "__NEW_TOPIC__") {
            g_chatContext.Clear();
            std::cout << "\n 已开启新话题，对话上下文已清空\n\n";
            return true;
        }

        if (params.question.empty() && params.imagePath.empty()) {
            std::cout << "\n用法:\n";
            std::cout << "  _askai <问题>              - 普通问答\n";
            std::cout << "  _askai /deep <问题>        - 深度思考模式\n";
            std::cout << "  _askai picture:\"<路径>\" <问题> - 图片识别模式\n";
            std::cout << "  _askai /new                - 开启新话题\n";
            std::cout << "  _askai /run <自然语言>     - 生成并执行 ZJHCMD 命令\n";
            std::cout << "  _askai /run:q <自然语言>   - 静默模式（批处理专用）\n";
            std::cout << "  _askai /run /deep <自然语言> - 深度思考 + 命令生成\n";
            std::cout << "  _askai /deep picture:\"<路径>\" <问题> - 深度思考+图片识别\n\n";
            std::cout << "示例:\n";
            std::cout << "  _askai 什么是RAII？\n";
            std::cout << "  _askai /deep 帮我用C++写一个快速排序\n";
            std::cout << "  _askai picture:\"C:\\Users\\User\\Pictures\\pic.png\" 解析图片内容\n";
            std::cout << "  _askai /run 查一下C盘还剩多少空间\n";
            std::cout << "  _askai /run:q 查一下系统状态\n\n";
            return true;
        }

        if (!params.quietMode) {
            std::cout << "\n    正在思考";
            if (params.deepThink) std::cout << " (深度思考模式)";
            if (!params.imagePath.empty()) std::cout << " (图片识别模式)";
            if (params.runMode) std::cout << " (命令生成模式)";
            std::cout << "...\n";

            if (params.deepThink) {
                std::cout << "      深度思考需要较长时间，请耐心等待...\n";
            }
        }

        int promptTokens = 0, completionTokens = 0, totalTokens = 0;

        std::string answer = AskDeepSeekEnhanced(
            params.question,
            params.deepThink,
            params.imagePath,
            params.newTopic,
            params.runMode,
            promptTokens,
            completionTokens,
            totalTokens
        );

        if (params.runMode) {
            size_t start = answer.find_first_not_of(" \t\r\n");
            if (start != std::string::npos) answer = answer.substr(start);
            size_t end = answer.find_last_not_of(" \t\r\n");
            if (end != std::string::npos) answer = answer.substr(0, end + 1);

            size_t firstLineEnd = answer.find('\n');
            std::string firstLine = (firstLineEnd != std::string::npos)
                ? answer.substr(0, firstLineEnd)
                : answer;
            if (!firstLine.empty() && firstLine.back() == '\r') firstLine.pop_back();

            if (firstLine == NO_WAY_TAG) {
                std::string reason;
                if (firstLineEnd != std::string::npos) {
                    reason = answer.substr(firstLineEnd + 1);
                    size_t rs = reason.find_first_not_of(" \t\r\n");
                    if (rs != std::string::npos) reason = reason.substr(rs);
                    size_t re = reason.find_last_not_of(" \t\r\n");
                    if (re != std::string::npos) reason = reason.substr(0, re + 1);
                }

                if (params.quietMode) {
                    std::cerr << "[JHNoWay] " << reason << "\n";
                    return true;
                }

                std::cout << "\n  ┌─────────────────────────────────────┐\n";
                std::cout << "  │  AI 无法生成命令                  │\n";
                std::cout << "  └─────────────────────────────────────┘\n\n";

                if (!reason.empty()) {
                    std::istringstream rs(reason);
                    std::string line;
                    while (std::getline(rs, line)) {
                        while (!line.empty() && (line.back() == '\r' || line.back() == '\n'))
                            line.pop_back();
                        std::cout << "  " << line << "\n";
                    }
                }
                else {
                    std::cout << "  AI 未提供原因\n";
                }
                std::cout << "\n";

                if (totalTokens > 0) {
                    std::cout << "  Token: " << promptTokens << " + "
                        << completionTokens << " = " << totalTokens << "\n\n";
                }
                return true;
            }

            if (params.quietMode) {
                if (!IsCommandSafe(answer)) {
                    std::cerr << "[白名单拒绝] " << answer << "\n";
                    return true;
                }
                bool executed = false;

                if (HandleBuiltinCommand(answer, true)) {
                    executed = true;
                }

                if (!executed) {
                    std::wstring wAnswer = U82W_Path(answer);
                    std::vector<wchar_t> cmdBuf(wAnswer.begin(), wAnswer.end());
                    cmdBuf.push_back(L'\0');

                    STARTUPINFOW si = { sizeof(si) };
                    PROCESS_INFORMATION pi = { 0 };
                    si.dwFlags = STARTF_USESTDHANDLES;
                    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
                    si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
                    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

                    if (CreateProcessW(NULL, cmdBuf.data(), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
                        WaitForSingleObject(pi.hProcess, INFINITE);
                        CloseHandle(pi.hProcess);
                        CloseHandle(pi.hThread);
                        executed = true;
                    }
                }

                if (!executed) {
                    std::string cmdName = answer;
                    std::string args;
                    size_t spacePos = answer.find(' ');
                    if (spacePos != std::string::npos) {
                        cmdName = answer.substr(0, spacePos);
                        args = answer.substr(spacePos);
                    }

                    std::string zjhCmdDir = GetZJHCMDConfigPath();
                    char exePath[MAX_PATH];
                    GetModuleFileNameA(NULL, exePath, MAX_PATH);

                    const char* exts[] = { ".exe", ".bat", ".cmd", ".zjhcmd", nullptr };
                    for (int i = 0; exts[i] != nullptr && !executed; i++) {
                        std::string testPath = zjhCmdDir + cmdName + exts[i];
                        DWORD attrs = GetFileAttributesA(testPath.c_str());
                        if (attrs == INVALID_FILE_ATTRIBUTES || (attrs & FILE_ATTRIBUTE_DIRECTORY)) {
                            continue;
                        }

                        std::string cmdLine;
                        if (std::string(exts[i]) == ".zjhcmd") {
                            cmdLine = "\"" + std::string(exePath) + "\" /c \"" + testPath + "\"" + args;
                        }
                        else {
                            cmdLine = "\"" + testPath + "\"" + args;
                        }

                        std::wstring wCmdLine = U82W_Path(cmdLine);
                        std::vector<wchar_t> cmdBuf2(wCmdLine.begin(), wCmdLine.end());
                        cmdBuf2.push_back(L'\0');

                        STARTUPINFOW si2 = { sizeof(si2) };
                        PROCESS_INFORMATION pi2 = { 0 };
                        si2.dwFlags = STARTF_USESTDHANDLES;
                        si2.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
                        si2.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
                        si2.hStdError = GetStdHandle(STD_ERROR_HANDLE);

                        if (CreateProcessW(NULL, cmdBuf2.data(), NULL, NULL, TRUE, 0, NULL, NULL, &si2, &pi2)) {
                            WaitForSingleObject(pi2.hProcess, INFINITE);
                            CloseHandle(pi2.hProcess);
                            CloseHandle(pi2.hThread);
                            executed = true;
                        }
                    }
                }

                if (!executed) {
                    std::cerr << "[无法执行] " << answer << "\n";
                }

                return true;
            }

            std::cout << "\n  ┌─────────────────────────────────────┐\n";
            std::cout << "  │  AI 生成的命令                  │\n";
            std::cout << "  ├─────────────────────────────────────┤\n";
            std::cout << "  │  " << answer << "\n";
            std::cout << "  └─────────────────────────────────────┘\n\n";

            if (!IsCommandSafe(answer)) {
                std::cout << "\n  ┌─────────────────────────────────────┐\n";
                std::cout << "  │  拒绝执行：命令不在白名单内        │\n";
                std::cout << "  └─────────────────────────────────────┘\n\n";
                std::cout << "  命令: " << answer << "\n";
                std::cout << "  原因: 该命令不在允许列表，或包含危险结构（管道/重定向/连接符）\n\n";
                return true;
            }

            CommandInfo info = ParseCommand(answer);
            bool isBuiltin = IsBuiltinCommand(info.command);
            bool isExternal = CheckCommandExists(info.command);

            bool isCustomCmd = false;
            if (!isBuiltin && !isExternal) {
                std::string zjhCmdDir = GetZJHCMDConfigPath();
                const char* exts[] = { ".exe", ".bat", ".cmd", ".zjhcmd", nullptr };
                for (int i = 0; exts[i] != nullptr; i++) {
                    std::string testPath = zjhCmdDir + info.command + exts[i];
                    DWORD attrs = GetFileAttributesA(testPath.c_str());
                    if (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY)) {
                        isCustomCmd = true;
                        break;
                    }
                }
            }

            if (!isBuiltin && !isExternal && !isCustomCmd) {
                std::cout << "  !   警告: AI 生成的命令不是 ZJHCMD 内置命令，\n";
                std::cout << "     也不是可执行文件或自定义命令\n\n";
            }

            std::cout << "  确认执行？(Y/N): ";
            int c = _getch();
            if (c != 'Y' && c != 'y') {
                std::cout << "\n  已取消\n\n";
                return true;
            }

            std::cout << "\n";
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);

            bool executed = false;

            if (HandleBuiltinCommand(answer, false)) {
                executed = true;
            }

            if (!executed) {
                char* cmdBuf = _strdup(answer.c_str());
                if (cmdBuf) {
                    STARTUPINFOA si = { sizeof(si) };
                    PROCESS_INFORMATION pi = { 0 };
                    si.dwFlags = STARTF_USESTDHANDLES;
                    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
                    si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
                    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

                    if (CreateProcessA(NULL, cmdBuf, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
                        WaitForSingleObject(pi.hProcess, INFINITE);
                        CloseHandle(pi.hProcess);
                        CloseHandle(pi.hThread);
                        executed = true;
                    }
                    free(cmdBuf);
                }
            }

            if (!executed) {
                std::string cmdName = answer;
                std::string args;
                size_t spacePos = answer.find(' ');
                if (spacePos != std::string::npos) {
                    cmdName = answer.substr(0, spacePos);
                    args = answer.substr(spacePos);
                }

                std::string zjhCmdDir = GetZJHCMDConfigPath();
                char exePath[MAX_PATH];
                GetModuleFileNameA(NULL, exePath, MAX_PATH);

                const char* exts[] = { ".exe", ".bat", ".cmd", ".zjhcmd", nullptr };
                for (int i = 0; exts[i] != nullptr && !executed; i++) {
                    std::string testPath = zjhCmdDir + cmdName + exts[i];
                    DWORD attrs = GetFileAttributesA(testPath.c_str());
                    if (attrs == INVALID_FILE_ATTRIBUTES || (attrs & FILE_ATTRIBUTE_DIRECTORY)) {
                        continue;
                    }

                    std::string cmdLine;
                    if (std::string(exts[i]) == ".zjhcmd") {
                        cmdLine = "\"" + std::string(exePath) + "\" /c \"" + testPath + "\"" + args;
                    }
                    else {
                        cmdLine = "\"" + testPath + "\"" + args;
                    }

                    char* cmdBuf2 = _strdup(cmdLine.c_str());
                    if (cmdBuf2) {
                        STARTUPINFOA si = { sizeof(si) };
                        PROCESS_INFORMATION pi = { 0 };
                        si.dwFlags = STARTF_USESTDHANDLES;
                        si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
                        si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
                        si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

                        if (CreateProcessA(NULL, cmdBuf2, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
                            WaitForSingleObject(pi.hProcess, INFINITE);
                            CloseHandle(pi.hProcess);
                            CloseHandle(pi.hThread);
                            executed = true;
                        }
                        free(cmdBuf2);
                    }
                }
            }

            if (!executed) {
                std::cout << "  无法执行: " << answer << "\n";
            }

            if (totalTokens > 0) {
                std::cout << "\n  Token: " << promptTokens << " + "
                    << completionTokens << " = " << totalTokens << "\n";
            }
            std::cout << "\n";
            return true;
        }

        if (params.quietMode) {
            std::cout << answer << "\n";
            return true;
        }

        std::cout << "\n╔══════════════════════════════════════════════════════╗\n";
        std::cout << "║                 DeepSeek AI 回答                    ║\n";
        if (params.deepThink) std::cout << "║     深度思考模式                          ║\n";
        if (!params.imagePath.empty()) std::cout << "║     图片识别模式                          ║\n";
        std::cout << "╚══════════════════════════════════════════════════════╝\n";
        std::cout << answer << "\n";
        std::cout << "╔══════════════════════════════════════════════════════╗\n";

        if (totalTokens > 0) {
            std::cout << "║   Token 消耗: 输入 " << promptTokens
                << " | 输出 " << completionTokens
                << " | 总计 " << totalTokens << "                    ║\n";
        }

        if (g_chatContext.hasContext) {
            std::cout << "║      对话上下文已保存 (共 " << g_chatContext.messages.size() / 2
                << " 轮对话)                        ║\n";
        }

        std::cout << "║  回答由 DeepSeek AI 生成，请注意核实              ║\n";
        std::cout << "╚══════════════════════════════════════════════════════╝\n\n";

        return true;
    }

    if (cmdTrimmed == "help" || cmdTrimmed == "help" || cmdTrimmed == "HELP" ||
        cmdTrimmed == "?" || cmdTrimmed.find("_Help_") != std::string::npos ||
        cmdTrimmed.find("_help_") != std::string::npos ||
        cmdTrimmed.find("_HELP_") != std::string::npos) {
        std::cout << R"(
            有关某个命令的详细信息，请键入 HELP 命令名
ASSOC          显示或修改文件扩展名关联。
ATTRIB         显示或更改文件属性。
BREAK          设置或清除扩展式 CTRL+C 检查。
BCDEDIT        设置启动数据库中的属性以控制启动加载。
CACLS          显示或修改文件的访问控制列表(ACL)。
CALL           从另一个批处理程序调用这一个。
CD             显示当前目录的名称或将其更改。
CHCP           显示或设置活动代码页数。
CHDIR          显示当前目录的名称或将其更改。
CHKDSK         检查磁盘并显示状态报告。
CHKNTFS        显示或修改启动时间磁盘检查。
CLS            清除屏幕。
CMD            打开另一个 Windows 命令解释程序窗口。
COLOR          设置默认控制台前景和背景颜色。
COMP           比较两个或两套文件的内容。
COMPACT        显示或更改 NTFS 分区上文件的压缩。
CONVERT        将 FAT 卷转换成 NTFS。你不能转换
               当前驱动器。
COPY           将至少一个文件复制到另一个位置。
DATE           显示或设置日期。
DEL            删除至少一个文件。
DIR            显示一个目录中的文件和子目录。
DISKPART       显示或配置磁盘分区属性。
DOSKEY         编辑命令行、撤回 Windows 命令并
               创建宏。
DRIVERQUERY    显示当前设备驱动程序状态和属性。
ECHO           显示消息，或将命令回显打开或关闭。
ENDLOCAL       结束批文件中环境更改的本地化。
ERASE          删除一个或多个文件。
EXIT           退出 CMD.EXE 程序(命令解释程序)。
FC             比较两个文件或两个文件集并显示
               它们之间的不同。
FIND           在一个或多个文件中搜索一个文本字符串。
FINDSTR        在多个文件中搜索字符串。
FOR            为一组文件中的每个文件运行一个指定的命令。
FORMAT         格式化磁盘，以便用于 Windows。
FSUTIL         显示或配置文件系统属性。
FTYPE          显示或修改在文件扩展名关联中使用的文件
               类型。
GOTO           将 Windows 命令解释程序定向到批处理程序
               中某个带标签的行。
GPRESULT       显示计算机或用户的组策略信息。
HELP           提供 Windows 命令的帮助信息。
ICACLS         显示、修改、备份或还原文件和
               目录的 ACL。
IF             在批处理程序中执行有条件的处理操作。
LABEL          创建、更改或删除磁盘的卷标。
MD             创建一个目录。
MKDIR          创建一个目录。
MKLINK         创建符号链接和硬链接
MODE           配置系统设备。
MORE           逐屏显示输出。
MOVE           将一个或多个文件从一个目录移动到另一个
               目录。
OPENFILES      显示远程用户为了文件共享而打开的文件。
PATH           为可执行文件显示或设置搜索路径。
PAUSE          暂停批处理文件的处理并显示消息。
POPD           还原通过 PUSHD 保存的当前目录的上一个
               值。
PRINT          打印一个文本文件。
PROMPT         更改 Windows 命令提示。
PUSHD          保存当前目录，然后对其进行更改。
RD             删除目录。
RECOVER        从损坏的或有缺陷的磁盘中恢复可读信息。
REM            记录批处理文件或 CONFIG.SYS 中的注释(批注)。
REN            重命名文件。
RENAME         重命名文件。
REPLACE        替换文件。
RMDIR          删除目录。
ROBOCOPY       复制文件和目录树的高级实用工具
SET            显示、设置或删除 Windows 环境变量。
SETLOCAL       开始本地化批处理文件中的环境更改。
SC             显示或配置服务(后台进程)。
SCHTASKS       安排在一台计算机上运行命令和程序。
SHIFT          调整批处理文件中可替换参数的位置。
SHUTDOWN       允许通过本地或远程方式正确关闭计算机。
SORT           对输入排序。
START          启动单独的窗口以运行指定的程序或命令。
SUBST          将路径与驱动器号关联。
SYSTEMINFO     显示计算机的特定属性和配置。
TASKLIST       显示包括服务在内的所有当前运行的任务。
TASKKILL       中止或停止正在运行的进程或应用程序。
TIME           显示或设置系统时间。
TITLE          设置 CMD.EXE 会话的窗口标题。
TREE           以图形方式显示驱动程序或路径的目录
               结构。
TYPE           显示文本文件的内容。
VER            显示 Windows 的版本。
VERIFY         告诉 Windows 是否进行验证，以确保文件
               正确写入磁盘。
VOL            显示磁盘卷标和序列号。
XCOPY          复制文件和目录树。
WMIC           在交互式命令 shell 中显示 WMI 信息。

键入 zjhhelp 查看该命令提示符的帮助信息
或键入 _askai /run <问题> 让AI自动生成ZJHCMD命令，前提是API_KEY
有关工具的详细信息，请参阅联机帮助中的命令行参考。
)";
        return true;
    }

    if (cmdTrimmed == "allhelp" || cmdTrimmed == "ALLHELP" || cmdTrimmed == "AllHelp") {
        for (int i = 0; helpcommand[i] != nullptr; i++) {
            std::cout << helpcommand[i] << "\n";
        }
        std::cout << "\n";
        return true;
    }

    if (cmdTrimmed == "JHversion" || cmdTrimmed == "jhversion" ||
        cmdTrimmed == "JHVERSION" || cmdTrimmed == "JHVersion") {

        std::cout << "\n";
        std::cout << "================================================================================\n";
        std::cout << "                    ZJHCMD - JH Developer Command Prompt\n";
        std::cout << "================================================================================\n";
        std::cout << "\n";
        std::cout << "  Product:     ZJHCMD (JH Developer Command Prompt)\n";
        std::cout << "  Version:     " << version << "\n";
        std::cout << "  Team:        JH Developer Group\n";
        std::cout << "  Founder:     Leo Carter\n";
        std::cout << "  Location:    Tianshui, Gansu, China\n";
        std::cout << "  Members:     Distributed across China\n";
        std::cout << "\n";
        std::cout << "  Slogan:      JH · Programming · Math · Technology\n";
        std::cout << "  Motto:       A bosom friend afar brings a distant land near.\n";
        std::cout << "\n";
        std::cout << "  Core Features:\n";
        std::cout << "    · High-precision math (BigNum)\n";
        std::cout << "    · Equation solver (linear/quadratic/system)\n";
        std::cout << "    · System control (st module)\n";
        std::cout << "    · Command ecosystem (apt install)\n";
        std::cout << "    · AI agent (_askai)\n";
        std::cout << "\n";
        std::cout << "  Related Works:\n";
        std::cout << "    · Jin Han Whiteboard · Touch Edition (Win32 + GDI)\n";
        std::cout << "    · JH5 Programming Language (self-made compiler)\n";
        std::cout << "    · JH Code Editor (Win32 + C++)\n";
        std::cout << "\n";
        std::cout << "  Copyright:\n";
        std::cout << "    Copyright (c) 2026 JH Developer Group\n";
        std::cout << "    All rights reserved.\n";
        std::cout << "\n";
        std::cout << "================================================================================\n";
        std::cout << "\n";

        return true;
    }

    if (cmdTrimmed == "zjhhelp" || cmdTrimmed == "ZJHhelp" || cmdTrimmed == "ZJHHELP" ||
        cmdTrimmed == "zjh?" || cmdTrimmed.find("_zjhHelp_") != std::string::npos ||
        cmdTrimmed.find("_zjhhelp_") != std::string::npos ||
        cmdTrimmed.find("_ZJHHELP_") != std::string::npos) {

        std::string helpTarget;
        size_t spacePos = cmdTrimmed.find(' ');
        if (spacePos != std::string::npos) {
            helpTarget = cmdTrimmed.substr(spacePos + 1);

            size_t start = helpTarget.find_first_not_of(" \t");
            if (start != std::string::npos) helpTarget = helpTarget.substr(start);
            size_t end = helpTarget.find_last_not_of(" \t");
            if (end != std::string::npos) helpTarget = helpTarget.substr(0, end + 1);
        }

        if (helpTarget.empty()) {
            std::cout << "\n";
            std::cout << "  JH Developer Command Prompt " << version << " - 帮助信息\n";
            std::cout << "================================================================================\n\n";

            std::cout << "内置命令：\n";
            std::cout << "  cd [路径]               - 切换当前目录，无参数时显示当前目录\n";
            std::cout << "  dir [路径] [/s]         - 列出目录内容，/s 递归显示子文件夹\n";
            std::cout << "  echo [文本]             - 输出文本，支持环境变量 %VAR%\n";
            std::cout << "  set [变量=值]           - 显示所有环境变量或设置变量\n";
            std::cout << "  exit                   - 退出程序\n";
            std::cout << "  taskkill [/F] /PID PID | /IM 进程名 - 终止进程\n";
            std::cout << "  _Internet_             - 用Edge浏览器上网（仅适用于Windows 10/11）\n";
            std::cout << "  home [/e]               - 切换到用户目录（/e 在资源管理器中打开）\n";
            std::cout << "  ~ / cd ~                - 快速切换到用户目录\n\n";

            std::cout << "系统控制命令：\n";
            std::cout << "  st mouse=X Y           - 移动鼠标到指定坐标\n";
            std::cout << "  st theme=0/1           - 切换主题（0=深色，1=浅色）\n";
            std::cout << "  st wallpaper=路径      - 设置桌面壁纸（支持环境变量）\n";
            std::cout << "  st volume           - 显示当前音量和静音状态\n";
            std::cout << "  st notify \"标题\" \"内容\" - 发送 Windows 通知弹窗\n";
            std::cout << "  st battery             - 查看电池状态（电量、充电状态等）\n";
            std::cout << "  st speak \"文本\"        - 语音朗读指定文本\n";
            std::cout << "  st lock                - 锁定电脑屏幕\n";
            std::cout << "  st uptime              - 显示系统运行时间\n";
            std::cout << "  st clipboard           - 剪贴板操作（复制/粘贴/清空）\n";
            std::cout << "  st size <路径> [-h]   - 显示文件或文件夹大小（递归统计，-h 人类可读格式）\n";
            std::cout << "  st mac                - 显示所有网卡的 MAC 地址\n";
            std::cout << "  st adapter            - 显示所有网卡的详细信息（IP、网关、DNS 等）\n";
            std::cout << "  st clean [/f]         - 清空回收站（/f 强制清空，不提示）\n";
            std::cout << "  st drives             - 显示所有盘符及其类型、剩余空间\n";
            std::cout << "  st path [add <目录>]  - 显示/添加 PATH 环境变量\n";
            std::cout << "  st timer <秒数>         - 倒计时计时器（带蜂鸣提醒）\n";
            std::cout << "  st alarm <秒数> [消息]  - 设置闹钟（蜂鸣+通知弹窗）\n";
            std::cout << "  st weather <城市>       - 查询天气（纯文本英文输出）\n";
            std::cout << "  st quote               - 随机显示名人名言\n";
            std::cout << "  st fortune             - 随机显示科技小知识\n";
            std::cout << "  st matrix [秒数]        - 黑客帝国数字雨特效（默认10秒）\n";
            std::cout << "  st compress <文件> [-o 输出] [/u] - 压缩/解压文件 (makecab)（v40新增）\n";
            std::cout << "  sleep [秒数]           - 延时执行（默认1秒，最大3600秒）\n";
            std::cout << "  whoami                 - 显示当前用户名\n";
            std::cout << "  hostname               - 显示计算机名\n";
            std::cout << "  beep [频率] [毫秒]     - 播放蜂鸣声（默认800Hz 500ms）\n";
            std::cout << "  rainbow [-on/off]     - 开启/关闭命令行彩虹模式\n";
            std::cout << "  _admin_               - 以管理员身份重新运行此程序\n";
            std::cout << "  vol [驱动器:]        - 显示磁盘卷标和序列号\n\n";

            std::cout << "实用工具：\n";
            std::cout << "  solve1 / Jiefangcheng11  - 一元一次方程求解器\n";
            std::cout << "  solve2 / Jiefangcheng12  - 一元二次方程求解器\n";
            std::cout << "  solve2var / Jiefangcheng21 - 二元一次方程组求解器\n";
            std::cout << "  solve3var / Jiefangcheng31 - 三元一次方程组求解器\n";
            std::cout << "  _calc                   - 执行表达式计算\n";
            std::cout << "  dumpbin                 - PE 文件分析工具\n\n";

            std::cout << "数学增强命令（v34 新增）：\n";
            std::cout << "  sin <数字>           - 正弦函数（同时显示弧度和角度）\n";
            std::cout << "  cos <数字>           - 余弦函数\n";
            std::cout << "  tan <数字>           - 正切函数\n";
            std::cout << "  asin <数字>          - 反正弦函数\n";
            std::cout << "  acos <数字>          - 反余弦函数\n";
            std::cout << "  atan <数字>          - 反正切函数\n";
            std::cout << "  exp <数字>           - e 的幂次\n";
            std::cout << "  log <数字>           - 自然对数（ln）\n";
            std::cout << "  log10 <数字>         - 常用对数（以10为底）\n";
            std::cout << "  log2 <数字>          - 以2为底的对数\n";
            std::cout << "  abs <数字>           - 绝对值\n";
            std::cout << "  ceil <数字>          - 向上取整\n";
            std::cout << "  floor <数字>         - 向下取整\n";
            std::cout << "  round <数字>         - 四舍五入\n";
            std::cout << "  sqrt <数字> <显示小数点后多少位> - 平方根\n";
            std::cout << "  cbrt <数字> <显示小数点后多少位> - 立方根\n";
            std::cout << "  fmod a b             - 浮点数取余\n";
            std::cout << "  gen2 <数字>          - 平方根（支持有理数和虚数）\n";
            std::cout << "  gen3 <数字>          - 立方根（支持有理数）\n";
            std::cout << "  solvei <不等式>      - 一元一次不等式求解\n";
            std::cout << "  示例: sin 30, log 100, gen2 12, solvei 2x+3>7\n\n";

            std::cout << "进制转换：\n";
            std::cout << "  hex <数字>           - 十进制转十六进制\n";
            std::cout << "  dec <十六进制>       - 十六进制转十进制\n";
            std::cout << "  bin <数字>           - 十进制转二进制\n\n";
            std::cout << "  JH_Shutdown + 数字 + 属性  - 执行关机，重启等操作\n\n";

            std::cout << "外部命令：\n";
            std::cout << "  支持任何可执行程序、PowerShell命令、CMD命令\n";
            std::cout << "  执行顺序：直接创建进程 → PowerShell → CMD\n\n";

            std::cout << "特殊功能：\n";
            std::cout << "  环境变量：支持 %VAR% 格式，自动从注册表读取\n";
            std::cout << "  管道：    支持 | 符号\n";
            std::cout << "  重定向：  支持 > >> < 符号\n";
            std::cout << "  在程序同目录创建名为ZJHCMD_TITLE的文件并写自定义内容可以应用于ZJHCMD的标题\n";
            std::cout << "  命令行：  支持 /c 模式调用\n\n";

            std::cout << "示例：\n";
            std::cout << "  dir                           - 列出当前目录\n";
            std::cout << "  dir C:\\Windows /s             - 递归列出 Windows 目录\n";
            std::cout << "  cd %USERPROFILE%              - 切换到用户目录\n";
            std::cout << "  echo %CD%                     - 显示当前目录\n";
            std::cout << "  set MYVAR=Hello               - 设置临时变量\n";
            std::cout << "  dir | find \".exe\"            - 管道查找 exe 文件\n";
            std::cout << "  dir > list.txt                - 输出重定向到文件\n\n";

            std::cout << "文件操作命令：\n";
            std::cout << "  cd [路径]               - 切换当前目录\n";
            std::cout << "  dir [路径] [/s]         - 列出目录内容\n";
            std::cout << "  copy <源> <目标>        - 复制文件\n";
            std::cout << "  move <源> <目标>        - 移动/重命名文件\n";
            std::cout << "  mkdir/md <目录>         - 创建新目录\n";
            std::cout << "  rmdir/rd <目录> [/s]    - 删除目录（/s递归删除）\n";
            std::cout << "  del <文件>              - 删除文件\n";
            std::cout << "  type <文件>             - 显示文本文件内容\n";
            std::cout << "  tree [路径] [/f]        - 显示目录树\n";
            std::cout << "  attrib [+R|-R]... [文件] - 修改文件属性\n";
            std::cout << "  mklink [/d] <链接> <目标> - 创建符号链接\n";
            std::cout << "  find \"字符串\" [文件]    - 查找字符串\n\n";

            std::cout << "个性化配置（v32 新增）：\n";
            std::cout << "  friend                - 友情日记（支持个性化配置）\n";
            std::cout << "  ZJHCMDTIPS            - 自定义提示符（文本文件配置）\n";
            std::cout << "  ZJHCMD_TITLE          - 自定义窗口标题（文本文件配置）\n";
            std::cout << "  配置文件位于：ZJHCMD.exe 同目录下的 .zjhcmd\\ 文件夹\n";
            std::cout << "  详细配置：help friend 或 help 个性化\n\n";

            std::cout << "方程求解命令（v33 国际化）：\n";
            std::cout << "  solve1    - 一元一次方程求解（旧名称：Jiefangcheng11）\n";
            std::cout << "  solve2    - 一元二次方程求解（旧名称：Jiefangcheng12）\n";
            std::cout << "  solve2var - 二元一次方程组求解（旧名称：Jiefangcheng21）\n";
            std::cout << "  solve3var - 三元一次方程组求解（旧名称：Jiefangcheng31）\n";
            std::cout << "  旧命令名称仍然可用，以确保向后兼容性。\n\n";

            std::cout << "网络与在线命令（v36 新增）：\n";
            std::cout << "  st news                - 获取今日头条新闻\n";
            std::cout << "  st weather <城市名>    - 查询天气（英文城市名）(v35添加)\n";
            std::cout << "  st translate \"文本\" [语言] - 多语言翻译\n";
            std::cout << "  示例: st translate \"Hello\" zh-CN\n";
            std::cout << "        st translate \"你好\" en\n";
            std::cout << "        st weather Beijing\n\n";

            std::cout << "AI 对话（v37 新增）：\n";
            std::cout << "  _askai <问题>        - 向 DeepSeek AI 提问（支持中英文）\n";
            std::cout << "  示例: _askai 什么是C++的RAII？\n";
            std::cout << "        _askai 用Python写一个快速排序\n";
            std::cout << "        _askai 解释一下量子计算\n";
            std::cout << "  配置：\n";
            std::cout << "    1. 在 ZJHCMD.exe 同目录下创建 API_KEY 文件\n";
            std::cout << "    2. 文件中填入你的 DeepSeek API Key（格式：sk-xxxxxxxx）\n";
            std::cout << "    3. 获取 API Key：访问 https://platform.deepseek.com/api_keys\n";
            std::cout << "    4. 需要先在 DeepSeek 平台充值 API 额度\n";
            std::cout << "  提示：Token 消耗会在回答后自动显示\n";
            std::cout << "        API Key 仅从本地文件读取，不会上传或泄露\n\n";

            std::cout << "翻译支持的语言代码：\n";
            std::cout << "  zh-CN  简体中文    zh-TW  繁体中文\n";
            std::cout << "  en     英语        ja     日语\n";
            std::cout << "  ko     韩语        fr     法语\n";
            std::cout << "  de     德语        es     西班牙语\n";
            std::cout << "  ru     俄语        it     意大利语\n";
            std::cout << "  pt     葡萄牙语    ar     阿拉伯语\n\n";

            std::cout << "  MyCommand         - 自定义命令模块 (v41新增)\n";
            std::cout << "  使用方法: 在 .zjhcmd 文件夹创建 命令名.zjhcmd\n";
            std::cout << "  功能说明: 即写即用，无需编译，彻底沙盒化\n";
            std::cout << "  安全限制: 物理封杀call，禁止无限递归\n";

            std::cout << "JHname -显示JH品牌图标和广告";
            std::cout << "输入 'zjhhelp <命令>' 查看详细帮助，如：help cd\n";
            std::cout << "================================================================================\n\n";
        }

        else if (helpTarget == "cd" || helpTarget == "CD") {
            std::cout << "\n";
            std::cout << "cd 命令 - 切换当前目录\n";
            std::cout << "================================================================================\n\n";
            std::cout << "语法：\n";
            std::cout << "  cd               - 显示当前目录路径\n";
            std::cout << "  cd [路径]        - 切换到指定目录\n\n";
            std::cout << "支持的环境变量：\n";
            std::cout << "  %USERPROFILE%    - 用户目录\n";
            std::cout << "  %CD%             - 当前目录\n";
            std::cout << "  %ProgramFiles%   - 程序文件目录\n\n";
            std::cout << "示例：\n";
            std::cout << "  cd                - 显示当前目录\n";
            std::cout << "  cd C:\\Windows     - 切换到 Windows 目录\n";
            std::cout << "  cd %USERPROFILE%  - 切换到用户目录\n";
            std::cout << "  cd ..             - 切换到上级目录\n";
            std::cout << "  cd Desktop        - 切换到当前目录下的 Desktop 文件夹\n";
            std::cout << "================================================================================\n\n";
        }
        else if (helpTarget == "battery" || helpTarget == "BATTERY") {
            std::cout << "\n";
            std::cout << "st battery 命令 - 查看电池状态\n";
            std::cout << "================================================================================\n\n";
            std::cout << "语法：\n";
            std::cout << "  st battery    - 显示电池电量、充电状态、剩余时间\n\n";
            std::cout << "示例：\n";
            std::cout << "  st battery    - 查看当前电池状态\n\n";
            std::cout << "输出信息：\n";
            std::cout << "  - 电量百分比（带进度条）\n";
            std::cout << "  - 充电状态（使用电池/充电中）\n";
            std::cout << "  - 剩余使用时间或充满所需时间\n";
            std::cout << "  - 电池状态建议\n";
            std::cout << "================================================================================\n\n";
        }
        else if (helpTarget == "speak" || helpTarget == "SPEAK") {
            std::cout << "\n";
            std::cout << "st speak 命令 - 语音朗读文本\n";
            std::cout << "================================================================================\n\n";
            std::cout << "语法：\n";
            std::cout << "  st speak \"文本\"    - 朗读指定文本\n\n";
            std::cout << "示例：\n";
            std::cout << "  st speak \"Hello World\"\n";
            std::cout << "  st speak \"计算完成，结果为42\"\n";
            std::cout << "  st speak \"注意，文件已删除\"\n\n";
            std::cout << "注意：\n";
            std::cout << "  - 需要 Windows 语音组件支持\n";
            std::cout << "  - 支持中文和英文朗读\n";
            std::cout << "  - 可以在批处理中使用，实现语音提示\n";
            std::cout << "================================================================================\n\n";
        }
        else if (helpTarget == "lock" || helpTarget == "LOCK") {
            std::cout << "\n";
            std::cout << "st lock 命令 - 锁定电脑屏幕\n";
            std::cout << "================================================================================\n\n";
            std::cout << "语法：\n";
            std::cout << "  st lock       - 显示确认提示后锁定\n";
            std::cout << "  st lock /f    - 直接锁定，不提示\n\n";
            std::cout << "示例：\n";
            std::cout << "  st lock       - 提示后锁定\n";
            std::cout << "  st lock /f    - 立即锁定\n\n";
            std::cout << "说明：\n";
            std::cout << "  - 效果等同于按 Win+L\n";
            std::cout << "  - 锁定后需要输入密码才能解锁\n";
            std::cout << "================================================================================\n\n";
        }
        else if (helpTarget == "uptime" || helpTarget == "UPTIME") {
            std::cout << "\n";
            std::cout << "st uptime 命令 - 显示系统运行时间\n";
            std::cout << "================================================================================\n\n";
            std::cout << "语法：\n";
            std::cout << "  st uptime      - 显示系统从开机到现在的运行时间\n\n";
            std::cout << "输出信息：\n";
            std::cout << "  - 天数、小时数、分钟数、秒数\n";
            std::cout << "  - 精确毫秒数\n";
            std::cout << "  - 上次开机时间\n\n";
            std::cout << "示例：\n";
            std::cout << "  st uptime\n";
            std::cout << "  输出：3 天 5 小时 12 分钟 30 秒\n\n";
            std::cout << "用途：\n";
            std::cout << "  - 查看电脑多久没关机了\n";
            std::cout << "  - 判断是否需要重启清理内存\n";
            std::cout << "================================================================================\n\n";
        }
        else if (helpTarget == "clipboard" || helpTarget == "CLIPBOARD" ||
            helpTarget == "clip" || helpTarget == "CLIP") {
            std::cout << "\n";
            std::cout << "st clipboard 命令 - 剪贴板操作（复制/粘贴/清空）\n";
            std::cout << "================================================================================\n\n";
            std::cout << "语法：\n";
            std::cout << "  st clipboard               - 显示当前剪贴板内容\n";
            std::cout << "  st clipboard set \"文本\"    - 复制文本到剪贴板\n";
            std::cout << "  st clipboard clear         - 清空剪贴板\n\n";
            std::cout << "示例：\n";
            std::cout << "  st clipboard                          - 查看剪贴板\n";
            std::cout << "  st clipboard set \"Hello World\"       - 复制英文\n";
            std::cout << "  st clipboard set \"你好世界\"          - 复制中文\n";
            std::cout << "  st clipboard set \"计算结果: 42\"      - 复制混合内容\n";
            std::cout << "  st clipboard clear                    - 清空剪贴板\n\n";
            std::cout << "用途：\n";
            std::cout << "  - 在批处理中复制输出结果到剪贴板\n";
            std::cout << "  - 快速查看剪贴板内容\n";
            std::cout << "  - 清空敏感信息\n";
            std::cout << "================================================================================\n\n";
        }
        else if (helpTarget == "dir" || helpTarget == "DIR") {
            std::cout << "\n";
            std::cout << "dir 命令 - 列出目录内容\n";
            std::cout << "================================================================================\n\n";
            std::cout << "语法：\n";
            std::cout << "  dir              - 列出当前目录内容\n";
            std::cout << "  dir [路径]       - 列出指定目录内容\n";
            std::cout << "  dir [路径] /s    - 递归列出所有子文件夹内容\n\n";
            std::cout << "支持的通配符：\n";
            std::cout << "  *                - 匹配任意字符\n";
            std::cout << "  ?                - 匹配单个字符\n\n";
            std::cout << "示例：\n";
            std::cout << "  dir                        - 列出当前目录\n";
            std::cout << "  dir C:\\Windows             - 列出 Windows 目录\n";
            std::cout << "  dir *.exe                  - 列出所有 exe 文件\n";
            std::cout << "  dir C:\\Windows /s          - 递归列出 Windows 目录\n";
            std::cout << "  dir *.cpp /s               - 递归查找所有 cpp 文件\n";
            std::cout << "  dir \"%USERPROFILE%\"        - 列出用户目录（路径有空格时加引号）\n";
            std::cout << "================================================================================\n\n";
        }
        else if (helpTarget == "echo" || helpTarget == "ECHO") {
            std::cout << "\n";
            std::cout << "echo 命令 - 输出文本\n";
            std::cout << "================================================================================\n\n";
            std::cout << "语法：\n";
            std::cout << "  echo [文本]      - 输出文本到控制台\n\n";
            std::cout << "支持的环境变量：\n";
            std::cout << "  %VAR%            - 输出环境变量的值\n";
            std::cout << "  %CD%             - 输出当前目录\n";
            std::cout << "  %USERNAME%       - 输出用户名\n";
            std::cout << "  %COMPUTERNAME%   - 输出计算机名\n\n";
            std::cout << "示例：\n";
            std::cout << "  echo Hello World           - 输出 Hello World\n";
            std::cout << "  echo %USERNAME%            - 输出用户名\n";
            std::cout << "  echo \"Hello %USERNAME%\"    - 输出带引号和变量的文本\n";
            std::cout << "  echo 当前目录：%CD%        - 输出当前目录\n";
            std::cout << "================================================================================\n\n";
        }
        else if (helpTarget == "set" || helpTarget == "SET") {
            std::cout << "\n";
            std::cout << "set 命令 - 显示/设置环境变量\n";
            std::cout << "================================================================================\n\n";
            std::cout << "语法：\n";
            std::cout << "  set              - 显示所有环境变量\n";
            std::cout << "  set [变量=值]    - 设置环境变量（仅当前会话）\n\n";
            std::cout << "注意：\n";
            std::cout << "  - 设置的变量只影响当前进程及子进程\n";
            std::cout << "  - 不会影响系统注册表中的永久变量\n";
            std::cout << "  - 程序启动时会自动加载注册表中的所有变量\n\n";
            std::cout << "示例：\n";
            std::cout << "  set                          - 显示所有环境变量\n";
            std::cout << "  set MYVAR=Hello              - 设置变量 MYVAR=Hello\n";
            std::cout << "  set PATH=%PATH%;C:\\MyTools   - 追加路径到 PATH\n";
            std::cout << "  echo %MYVAR%                 - 输出 Hello\n";
            std::cout << "================================================================================\n\n";
        }
        else if (helpTarget == "mouse" || helpTarget == "MOUSE") {
            std::cout << "\n";
            std::cout << "st mouse 命令 - 移动鼠标\n";
            std::cout << "================================================================================\n\n";
            std::cout << "语法：\n";
            std::cout << "  st mouse=X Y    - 移动鼠标到屏幕坐标 (X, Y)\n\n";
            std::cout << "注意：\n";
            std::cout << "  - X 和 Y 必须是数字\n";
            std::cout << "  - 坐标不能超出屏幕范围\n\n";
            std::cout << "示例：\n";
            std::cout << "  st mouse=500 300      - 移动鼠标到 x=500, y=300\n";
            std::cout << "  st mouse=100 100      - 移动鼠标到屏幕左上角附近\n";
            std::cout << "================================================================================\n\n";
        }
        else if (helpTarget == "theme" || helpTarget == "THEME") {
            std::cout << "\n";
            std::cout << "st theme 命令 - 切换 Windows 主题\n";
            std::cout << "================================================================================\n\n";
            std::cout << "语法：\n";
            std::cout << "  st theme=0      - 切换到深色模式\n";
            std::cout << "  st theme=1      - 切换到浅色模式\n\n";
            std::cout << "注意：\n";
            std::cout << "  - 立即生效，不需要重启\n";
            std::cout << "  - 影响系统主题和应用程序主题\n";
            std::cout << "================================================================================\n\n";
        }
        else if (helpTarget == "wallpaper" || helpTarget == "WALLPAPER") {
            std::cout << "\n";
            std::cout << "st wallpaper 命令 - 设置桌面壁纸\n";
            std::cout << "================================================================================\n\n";
            std::cout << "语法：\n";
            std::cout << "  st wallpaper=路径    - 设置桌面壁纸\n\n";
            std::cout << "支持：\n";
            std::cout << "  - 绝对路径\n";
            std::cout << "  - 环境变量（如 %USERPROFILE%\\Pictures\\bg.jpg）\n";
            std::cout << "  - 图片格式：.jpg, .png, .bmp 等\n\n";
            std::cout << "示例：\n";
            std::cout << "  st wallpaper=C:\\Wallpapers\\bg.jpg\n";
            std::cout << "  st wallpaper=%USERPROFILE%\\Pictures\\wallpaper.png\n";
            std::cout << "================================================================================\n\n";
        }
        else if (helpTarget == "friend" || helpTarget == "FRIEND" || helpTarget == "friends") {
            std::cout << "\n";
            std::cout << "friend 命令 - 友情秘密模式\n";
            std::cout << "================================================================================\n\n";
            std::cout << "这是彩蛋功能，需保密！\n\n";
        }
        else if (helpTarget == "solve1" || helpTarget == "solve1") {
            std::cout << "\n";
            std::cout << "solve1 / Jiefangcheng11 - 一元一次方程求解器\n";
            std::cout << "================================================================================\n\n";
            std::cout << "语法：\n";
            std::cout << "  solve1 <方程>     - 直接求解\n";
            std::cout << "  solve1            - 进入交互式求解器\n\n";
            std::cout << "示例：\n";
            std::cout << "  solve1 2x+3=7\n";
            std::cout << "  输出：x = 2\n\n";
            std::cout << "  solve1\n";
            std::cout << "  输入方程: 3x-5=10\n";
            std::cout << "  输出：x = 5\n\n";
            std::cout << "支持的格式：\n";
            std::cout << "  - 标准形式：2x+3=7\n";
            std::cout << "  - 分数系数：2/3x+1/2=5/6\n";
            std::cout << "  - 带括号：2(x+1)=3x\n";
            std::cout << "  - 移项形式：2x+3=4x-1\n\n";
            std::cout << "旧名称：Jiefangcheng11（仍然可用）\n";
            std::cout << "================================================================================\n\n";
        }
        else if (helpTarget == "solve2" || helpTarget == "SOLVE2") {
            std::cout << "\n";
            std::cout << "solve2 / Jiefangcheng12 - 一元二次方程求解器\n";
            std::cout << "================================================================================\n\n";
            std::cout << "语法：\n";
            std::cout << "  solve2 <方程>     - 直接求解\n";
            std::cout << "  solve2            - 进入交互式求解器\n\n";
            std::cout << "示例：\n";
            std::cout << "  solve2 2x^2+3x-5=0\n";
            std::cout << "  输出：\n";
            std::cout << "    方程: 2x^2 + 3x + -5 = 0\n";
            std::cout << "    判别式 Δ = b^2 - 4ac = 49\n";
            std::cout << "    x1 = 1\n";
            std::cout << "    x2 = -2.5\n\n";
            std::cout << "支持的格式：\n";
            std::cout << "  - 标准形式：2x^2+3x-5=0\n";
            std::cout << "  - 简写形式：x^2-4=0, 2x^2=8\n";
            std::cout << "  - 缺省系数：x^2+2x=0, x^2-1=0\n";
            std::cout << "  - 小数系数：1.5x^2-2.3x+0.5=0\n\n";
            std::cout << "旧名称：Jiefangcheng12（仍然可用）\n";
            std::cout << "================================================================================\n\n";
        }
        else if (helpTarget == "solve2var" || helpTarget == "SOLVE2VAR" || helpTarget == "jiefangcheng21" || helpTarget == "Jiefangcheng21") {
            std::cout << "\n";
            std::cout << "solve2var / Jiefangcheng21 - 二元一次方程组求解器\n";
            std::cout << "================================================================================\n\n";
            std::cout << "语法：\n";
            std::cout << "  solve2var        - 进入交互式求解器\n\n";
            std::cout << "支持的格式：\n";
            std::cout << "  - 标准形式：2x+3y=5\n";
            std::cout << "  - 分数系数：1/2x+2/3y=5/6\n";
            std::cout << "  - 带括号：2(x+1)=3y\n";
            std::cout << "  - 移项形式：2x+3=4y-1\n\n";
            std::cout << "示例：\n";
            std::cout << "  方程1: 2x+3y=8\n";
            std::cout << "  方程2: 3x-2y=1\n";
            std::cout << "  输出：x = 19/13 ≈ 1.46154, y = 22/13 ≈ 1.69231\n\n";
            std::cout << "提示：\n";
            std::cout << "  - 输入 'exit' 退出求解器\n";
            std::cout << "  - 只支持变量 x 和 y\n";
            std::cout << "旧名称：Jiefangcheng21（仍然可用）\n";
            std::cout << "================================================================================\n\n";
        }
        else if (helpTarget == "solve3var" || helpTarget == "SOLVE3VAR" || helpTarget == "jiefangcheng31" || helpTarget == "Jiefangcheng31") {
            std::cout << "\n";
            std::cout << "solve3var / Jiefangcheng31 - 三元一次方程组求解器\n";
            std::cout << "================================================================================\n\n";
            std::cout << "语法：\n";
            std::cout << "  solve3var        - 进入交互式求解器\n\n";
            std::cout << "支持的格式：\n";
            std::cout << "  - 标准形式：2x+3y-z=5\n";
            std::cout << "  - 分数系数：2x+3/4y-1/2z=5/2\n";
            std::cout << "  - 支持括号、小数、分数、系数省略\n\n";
            std::cout << "示例：\n";
            std::cout << "  方程1: 2x+3y-z=8\n";
            std::cout << "  方程2: 3x-2y+4z=1\n";
            std::cout << "  方程3: x+y+z=6\n";
            std::cout << "  输出：x = 1, y = 2, z = 3\n\n";
            std::cout << "提示：\n";
            std::cout << "  - 输入 'exit' 退出求解器\n";
            std::cout << "  - 只支持变量 x、y 和 z\n";
            std::cout << "旧名称：Jiefangcheng31（仍然可用）\n";
            std::cout << "================================================================================\n\n";
        }
        else if (helpTarget == "notify" || helpTarget == "NOTIFY" || helpTarget == "toast" || helpTarget == "TOAST" || helpTarget == "popup" || helpTarget == "POPUP") {
            std::cout << "\n";
            std::cout << "st notify 命令 - 发送 Windows 通知弹窗\n";
            std::cout << "================================================================================\n\n";
            std::cout << "语法：\n";
            std::cout << "  st notify \"标题\" \"内容\" [时长]    - 发送通知\n";
            std::cout << "  st toast \"标题\" \"内容\" [时长]     - 同 notify（别名）\n";
            std::cout << "  st popup \"标题\" \"内容\" [时长]     - 同 notify（别名）\n\n";
            std::cout << "参数：\n";
            std::cout << "  标题   - 通知的标题文字（必填）\n";
            std::cout << "  内容   - 通知的具体内容（可选，默认为空）\n";
            std::cout << "  时长   - 通知显示秒数（可选，1-30，默认5秒）\n\n";
            std::cout << "示例：\n";
            std::cout << "  st notify \"提醒\" \"任务已完成\"              - 发送普通通知\n";
            std::cout << "  st toast \"完成\" \"备份成功\" 10              - 显示10秒\n";
            std::cout << "  st popup \"信息\"                             - 只有标题\n";
            std::cout << "  st notify \"警告\" \"系统将在5分钟后重启\" 8   - 警告通知\n\n";
            std::cout << "注意：\n";
            std::cout << "  - 需要在 Windows 8/10/11 上运行\n";
            std::cout << "  - 标题和内容建议用英文双引号括起来\n";
            std::cout << "  - 通知会显示在 Windows 右下角通知区域\n";
            std::cout << "  - 如果开启了专注助手，通知可能不会弹窗\n";
            std::cout << "================================================================================\n\n";
        }
        else {
            std::cout << "未知命令: " << helpTarget << "\n";
            std::cout << "输入 'help' 查看所有可用命令\n\n";
        }

        return true;
    }

    if (_strnicmp(cmdTrimmed.c_str(), "more", 4) == 0) {
        std::string param = cmdTrimmed.size() > 4 ? cmdTrimmed.substr(4) : "";
        size_t s = param.find_first_not_of(" \t");
        if (s != std::string::npos) {
            param = param.substr(s);
        }
        else {
            std::cout << "用法: more <文件名>\n";
            std::cout << "示例: more readme.txt\n\n";
            return true;
        }

        if (param.size() >= 2 && param.front() == '"' && param.back() == '"') {
            param = param.substr(1, param.size() - 2);
        }

        MorePager(param);
        return true;
    }

    if (_strnicmp(cmdTrimmed.c_str(), "dumpbin", 7) == 0) {
        std::string param = cmdTrimmed.size() > 7 ? cmdTrimmed.substr(7) : "";
        size_t s = param.find_first_not_of(" \t");
        if (s != std::string::npos) param = param.substr(s);
        else param = "";

        if (param.empty() || param == "/?" || param == "-?") {
            std::cout << "\ndumpbin - PE 文件分析工具\n";
            std::cout << "========================================\n";
            std::cout << "用法: dumpbin <选项> <文件路径>\n\n";
            std::cout << "选项:\n";
            std::cout << "  /dependents  显示依赖的 DLL\n";
            std::cout << "  /exports     显示导出表\n";
            std::cout << "  /imports     显示导入表\n";
            std::cout << "  /sections    显示节区信息\n";
            std::cout << "  /headers     显示 PE 头信息\n";
            std::cout << "  /resources   显示资源\n";
            std::cout << "  /all         显示所有信息\n\n";
            std::cout << "示例:\n";
            std::cout << "  dumpbin /dependents ZJHCMD.exe\n";
            std::cout << "  dumpbin /headers C:\\Windows\\System32\\notepad.exe\n";
            std::cout << "  dumpbin /all ZJHCMD.exe\n\n";
            return true;
        }

        std::string option;
        std::string filePath;

        size_t spacePos = param.find(' ');
        if (spacePos != std::string::npos) {
            option = param.substr(0, spacePos);
            filePath = param.substr(spacePos + 1);

            size_t fs = filePath.find_first_not_of(" \t");
            if (fs != std::string::npos) filePath = filePath.substr(fs);
            size_t fe = filePath.find_last_not_of(" \t");
            if (fe != std::string::npos) filePath = filePath.substr(0, fe + 1);
        }
        else {
            option = param;
            filePath = "";
        }

        if (option[0] != '/') {
            std::cout << "错误: 请指定选项 (/dependents, /exports, /headers 等)\n";
            std::cout << "输入 dumpbin /? 查看帮助\n\n";
            return true;
        }

        if (!filePath.empty() && filePath.size() >= 2 && filePath.front() == '"' && filePath.back() == '"') {
            filePath = filePath.substr(1, filePath.size() - 2);
        }

        if (filePath.empty()) {
            size_t optLen = option.length();
            std::string remaining = param.substr(optLen);
            size_t rs = remaining.find_first_not_of(" \t");
            if (rs != std::string::npos) {
                filePath = remaining.substr(rs);
                size_t re = filePath.find_last_not_of(" \t");
                if (re != std::string::npos) filePath = filePath.substr(0, re + 1);
            }
        }

        if (filePath.empty()) {
            std::cout << "错误: 请指定要分析的文件路径\n";
            std::cout << "示例: dumpbin /headers ZJHCMD.exe\n\n";
            return true;
        }

        filePath = ExpandEnvironmentVars(filePath);

        std::wstring wFilePath = U82W_Path(filePath);
        DWORD attrs = GetFileAttributesW(wFilePath.c_str());
        if (attrs == INVALID_FILE_ATTRIBUTES) {
            DWORD err = GetLastError();
            std::cout << "错误: 文件不存在 - " << filePath << "\n";
            std::cout << "  错误码: " << err << "\n\n";
            return true;
        }
        if (attrs & FILE_ATTRIBUTE_DIRECTORY) {
            std::cout << "错误: 路径指向目录，请指定文件 - " << filePath << "\n\n";
            return true;
        }

        ShowDependencies(filePath, option);
        return true;
    }

    if (cmdTrimmed.substr(0, 6) == "_calc ") {
        std::string expr = cmdTrimmed.substr(5);
        try {
            std::vector<std::string> rpn = toRPN(expr);
            BigNum result = evalRPN(rpn);
            std::cout << "结果: " << toString(result) << std::endl;
        }
        catch (const std::exception& e) {
            std::cout << "错误: " << e.what() << std::endl;
        }
        return true;
    }

    if (cmdTrimmed.find("Jiefangcheng21") != std::string::npos || cmdTrimmed.find("jiefangcheng21") != std::string::npos || cmdTrimmed.find("solve2var") != std::string::npos || cmdTrimmed.find("SOLVE2VAR") != std::string::npos) {
        std::string s1, s2;

        std::cout << "\n===== 二元一次方程组求解器 =====\n";
        std::cout << "支持格式：2x+3y=5 或 2x+3/4y=5/2 或带括号如 2(x+1)=3y\n";
        std::cout << "输入 'exit' 退出求解器\n\n";

        while (true) {
            if (!IsRunningAsAdmin()) {
                SetConsoleTitleA("二元一次方程组求解");
            }
            else {
                SetConsoleTitleA("管理员: 二元一次方程组求解");
            }
            std::cout << "方程1: ";
            std::getline(std::cin, s1);
            if (s1 == "exit" || s1 == "quit") break;

            std::cout << "方程2: ";
            std::getline(std::cin, s2);
            if (s2 == "exit" || s2 == "quit") break;

            if (s1.empty() || s2.empty()) {
                std::cout << "方程不能为空！\n\n";
                continue;
            }

            auto hasOtherLetter = [](const std::string& s) -> bool {
                for (char c : s) {
                    if (isalpha(c) && c != 'x' && c != 'y') return true;
                }
                return false;
                };

            if (hasOtherLetter(s1) || hasOtherLetter(s2)) {
                std::cout << "错误：方程只能包含变量 x 和 y！\n\n";
                continue;
            }

            try {
                Frac a1, b1, c1, a2, b2, c2;
                if (!parseEq(s1, a1, b1, c1) || !parseEq(s2, a2, b2, c2)) {
                    std::cout << "格式错误！请使用格式如：2x+3y=5 或 2x=3y+1\n\n";
                    continue;
                }

                std::cout << "\n方程1: " << a1 << "x + " << b1 << "y = " << c1 << std::endl;
                std::cout << "方程2: " << a2 << "x + " << b2 << "y = " << c2 << std::endl;
                std::cout << "\n计算结果：\n";

                Frac D = a1 * b2 - a2 * b1;
                Frac Dx = c1 * b2 - c2 * b1;
                Frac Dy = a1 * c2 - a2 * c1;

                if (D.n == 0) {
                    if (Dx.n == 0 && Dy.n == 0)
                        std::cout << "无穷多解\n";
                    else
                        std::cout << "无解\n";
                }
                else {
                    Frac x = Dx / D;
                    Frac y = Dy / D;

                    std::cout << "x = " << x;
                    if (x.d != 1) std::cout << " ≈ " << x.toDouble();
                    std::cout << "\n";
                    std::cout << "y = " << y;
                    if (y.d != 1) std::cout << " ≈ " << y.toDouble();
                    std::cout << "\n";
                }
            }
            catch (const std::exception& e) {
                std::cout << "错误: " << e.what() << "\n";
            }

            std::cout << "\n";
        }

        std::cout << "退出求解器\n\n";
        return true;
    }

    if (cmdTrimmed.substr(0, 14) == "Jiefangcheng11 " ||
        cmdTrimmed.substr(0, 14) == "jiefangcheng11 ") {
        std::string eq = cmdTrimmed.substr(14);
        size_t s = eq.find_first_not_of(" \t");
        if (s != std::string::npos) eq = eq.substr(s);
        size_t e = eq.find_last_not_of(" \t");
        if (e != std::string::npos) eq = eq.substr(0, e + 1);

        if (!eq.empty()) {
            try {
                Frac a, c;
                if (parseLinearEq(eq, a, c)) {
                    if (a.n == 0) {
                        std::cout << (c.n == 0 ? "无穷多解\n" : "无解\n");
                    }
                    else {
                        Frac x = c / a;
                        std::cout << "x = " << x;
                        if (x.d != 1) std::cout << " ≈ " << x.toDouble();
                        std::cout << "\n\n";
                    }
                }
                else {
                    std::cout << "格式错误！\n\n";
                }
            }
            catch (const std::exception& e) {
                std::cout << "错误: " << e.what() << "\n\n";
            }
            return true;
        }
    }

    if (cmdTrimmed.find("Jiefangcheng11") != std::string::npos || cmdTrimmed.find("jiefangcheng11") != std::string::npos || cmdTrimmed.find("SOLVE1") != std::string::npos || cmdTrimmed.find("solve1") != std::string::npos) {
        std::cout << "\n===== 一元一次方程求解器=====\n";
        std::cout << "示例: 2/3*x + 1/2 = 5/6\n";
        std::cout << "      x/2 + 3 = 7\n";
        std::cout << "      1.5x - 0.5 = 2\n";
        std::cout << "输入 'exit' 退出\n\n";

        std::string eq;
        while (true) {
            std::cout << "input:> ";
            if (!IsRunningAsAdmin()) {
                SetConsoleTitleA("一元一次方程求解");
            }
            else {
                SetConsoleTitleA("管理员: 一元一次方程求解");
            }
            if (!getline(std::cin, eq)) break;
            if (eq == "exit" || eq == "quit") break;
            if (eq.empty()) continue;

            try {
                Frac a, c;
                if (!parseLinearEq(eq, a, c)) {
                    std::cout << "格式错误！\n\n";
                    continue;
                }

                std::cout << a << "x = " << c << "  →  ";

                if (a.n == 0) {
                    if (c.n == 0) std::cout << "无穷多解";
                    else std::cout << "无解";
                }
                else {
                    Frac x = c / a;
                    std::cout << "x = " << x;
                    if (x.d != 1) std::cout << " ≈ " << x.toDouble();
                }
                std::cout << "\n\n";
            }
            catch (const std::exception& e) {
                std::cout << "错误: " << e.what() << "\n\n";
            }
        }
        std::cout << "退出求解器\n\n";
        return true;
    }

    if (cmdTrimmed.find("Jiefangcheng31") != std::string::npos || cmdTrimmed.find("jiefangcheng31") != std::string::npos || cmdTrimmed.find("solve3var") != std::string::npos || cmdTrimmed.find("SOLVE3VAR") != std::string::npos) {
        std::string s1, s2, s3;

        std::cout << "\n===== 三元一次方程组求解器 =====\n";
        std::cout << "支持格式：2x+3y-z=5 或 2x+3/4y-1/2z=5/2\n";
        std::cout << "支持括号、小数、分数、系数省略\n";
        std::cout << "输入 'exit' 退出求解器\n\n";

        while (true) {
            if (!IsRunningAsAdmin()) {
                SetConsoleTitleA("三元一次方程组求解");
            }
            else {
                SetConsoleTitleA("管理员: 三元一次方程组求解");
            }
            std::cout << "方程1: ";
            std::getline(std::cin, s1);
            if (s1 == "exit" || s1 == "quit") break;

            std::cout << "方程2: ";
            std::getline(std::cin, s2);
            if (s2 == "exit" || s2 == "quit") break;

            std::cout << "方程3: ";
            std::getline(std::cin, s3);
            if (s3 == "exit" || s3 == "quit") break;

            if (s1.empty() || s2.empty() || s3.empty()) {
                std::cout << "方程不能为空！\n\n";
                continue;
            }

            auto hasOtherLetter = [](const std::string& s) -> bool {
                for (char c : s) {
                    if (isalpha(c) && c != 'x' && c != 'y' && c != 'z') return true;
                }
                return false;
                };

            if (hasOtherLetter(s1) || hasOtherLetter(s2) || hasOtherLetter(s3)) {
                std::cout << "错误：方程只能包含变量 x、y、z！\n\n";
                continue;
            }

            try {
                Frac a1, b1, c1, d1, a2, b2, c2, d2, a3, b3, c3, d3;
                if (!parseEq3(s1, a1, b1, c1, d1) ||
                    !parseEq3(s2, a2, b2, c2, d2) ||
                    !parseEq3(s3, a3, b3, c3, d3)) {
                    std::cout << "格式错误！请使用格式如：2x+3y-z=5\n\n";
                    continue;
                }

                std::cout << "\n方程1: " << a1 << "x + " << b1 << "y + " << c1 << "z = " << d1 << std::endl;
                std::cout << "方程2: " << a2 << "x + " << b2 << "y + " << c2 << "z = " << d2 << std::endl;
                std::cout << "方程3: " << a3 << "x + " << b3 << "y + " << c3 << "z = " << d3 << std::endl;
                std::cout << "\n计算结果：\n";

                Frac D = a1 * (b2 * c3 - b3 * c2) -
                    b1 * (a2 * c3 - a3 * c2) +
                    c1 * (a2 * b3 - a3 * b2);

                Frac Dx = d1 * (b2 * c3 - b3 * c2) -
                    b1 * (d2 * c3 - d3 * c2) +
                    c1 * (d2 * b3 - d3 * b2);

                Frac Dy = a1 * (d2 * c3 - d3 * c2) -
                    d1 * (a2 * c3 - a3 * c2) +
                    c1 * (a2 * d3 - a3 * d2);

                Frac Dz = a1 * (b2 * d3 - b3 * d2) -
                    b1 * (a2 * d3 - a3 * d2) +
                    d1 * (a2 * b3 - a3 * b2);

                if (D.n == 0) {
                    if (Dx.n == 0 && Dy.n == 0 && Dz.n == 0)
                        std::cout << "无穷多解\n";
                    else
                        std::cout << "无解\n";
                }
                else {
                    Frac x = Dx / D;
                    Frac y = Dy / D;
                    Frac z = Dz / D;

                    std::cout << "x = " << x;
                    if (x.d != 1) std::cout << " ≈ " << x.toDouble();
                    std::cout << "\ny = " << y;
                    if (y.d != 1) std::cout << " ≈ " << y.toDouble();
                    std::cout << "\nz = " << z;
                    if (z.d != 1) std::cout << " ≈ " << z.toDouble();
                    std::cout << std::endl;
                }
            }
            catch (const std::exception& e) {
                std::cout << "错误: " << e.what() << "\n";
            }
            std::cout << "\n";
        }
        std::cout << "退出求解器\n\n";
        return true;
    }

    if (cmdTrimmed.substr(0, 7) == "JHname " || cmdTrimmed == "JHname" || cmdTrimmed.substr(0, 7) == "JHNAME " || cmdTrimmed == "JHNAME" || cmdTrimmed.substr(0, 7) == "jhname " || cmdTrimmed == "jhname") {
        std::cout << "┌───────────────────────┐\n";
        std::cout << "│ ┌───────────────────┐ │\n";
        std::cout << "│ │                   │ │\n";
        std::cout << "│ │      ██╗ ██╗  ██╗ │ │\n";
        std::cout << "│ │      ██║ ██║  ██║ │ │\n";
        std::cout << "│ │      ██║ ███████║ │ │\n";
        std::cout << "│ │ ██╗  ██║ ██╔══██║ │ │\n";
        std::cout << "│ │ ███████║ ██║  ██║ │ │\n";
        std::cout << "│ │ ╚══════╝ ╚═╝  ╚═╝ │ │\n";
        std::cout << "│ │                   │ │\n";
        std::cout << "│ └───────────────────┘ │\n";
        std::cout << "└───────────────────────┘\n\n";
        std::cout << "  JH·编程·数学·科技\n";
        return true;
    }

    if (cmdTrimmed.substr(0, 12) == "JH_Shutdown " ||
        cmdTrimmed.substr(0, 12) == "jh_shutdown " ||
        cmdTrimmed.substr(0, 12) == "JH_shutdown ") {

        std::string param = cmdTrimmed.substr(12);

        int delaySeconds = 0;
        bool hasDelay = false;

        size_t underscore1 = param.find('_');
        if (underscore1 != std::string::npos) {
            size_t underscore2 = param.find('_', underscore1 + 1);
            if (underscore2 != std::string::npos) {
                std::string numStr = param.substr(underscore1 + 1, underscore2 - underscore1 - 1);
                bool isAllDigit = true;
                for (char c : numStr) {
                    if (!isdigit(c)) { isAllDigit = false; break; }
                }
                if (isAllDigit && !numStr.empty()) {
                    long long val = std::stoll(numStr);
                    if (val > 0 && val <= 31536000) {
                        delaySeconds = (int)val;
                        hasDelay = true;
                        param.erase(underscore1, underscore2 - underscore1 + 1);
                        size_t sp = param.find_first_not_of(" \t");
                        if (sp != std::string::npos) param = param.substr(sp);
                        else param.clear();
                    }
                    else {
                        std::cout << "参数无效！\n";
                        return true;
                    }
                }
            }
        }

        int action = -1;
        if (param.find("0") != std::string::npos) action = 0;
        else if (param.find("1") != std::string::npos) action = 1;
        else if (param.find("2") != std::string::npos) action = 2;
        else if (param.find("3") != std::string::npos) action = 3;
        else if (param.find("4") != std::string::npos) action = 4;
        else if (param.find("5") != std::string::npos) action = 5;

        if (action == -1) {
            std::cout << "参数错误！\n";
            std::cout << "用法: JH_Shutdown [_延时秒数_] 0-5\n";
            std::cout << "  0=关机 1=重启 2=注销 3=强制关机 4=强制重启 5=强制注销\n";
            std::cout << "  延时示例: JH_Shutdown _300_ 0  (等待300秒后关机)\n";
            std::cout << "  最大延时: 31536000 秒\n\n";
            return true;
        }

        HANDLE hToken;
        TOKEN_PRIVILEGES tp;
        LUID luid;
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
            std::cout << "无法获取权限，命令执行失败\n\n";
            return true;
        }
        LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &luid);
        tp.PrivilegeCount = 1;
        tp.Privileges[0].Luid = luid;
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);
        CloseHandle(hToken);

        if (hasDelay && delaySeconds > 0) {
            if (delaySeconds > 0) {
                std::cout << "将在 " << delaySeconds << " 秒后执行操作...\n";
                std::cout << "按 Ctrl+C 取消（需自行处理中断）\n";
            }

            const int CHUNK = 1000;
            for (int i = 0; i < delaySeconds; i += CHUNK) {
                int remain = delaySeconds - i;
                int sleepMs = (remain > CHUNK) ? CHUNK * 1000 : remain * 1000;
                Sleep(sleepMs);
            }
        }

        bool is1 = true;
        if (cmdTrimmed.find("/q") != std::string::npos || cmdTrimmed.find("-q") != std::string::npos ||
            cmdTrimmed.find("/Q") != std::string::npos || cmdTrimmed.find("-Q") != std::string::npos) {
            is1 = false;
        }

        int get;
        switch (action) {
        case 0:
            if (!is1) ExitWindowsEx(EWX_POWEROFF, 0);
            else {
                std::cout << "警告：此操作将关闭系统，是否继续？(Y/N): ";
                get = _getch();
                if (get == 'Y' || get == 'y') ExitWindowsEx(EWX_POWEROFF, 0);
            }
            break;
        case 1:
            if (!is1) ExitWindowsEx(EWX_REBOOT, 0);
            else {
                std::cout << "警告：此操作将重启系统，是否继续？(Y/N): ";
                get = _getch();
                if (get == 'Y' || get == 'y') ExitWindowsEx(EWX_REBOOT, 0);
            }
            break;
        case 2:
            if (!is1) ExitWindowsEx(EWX_LOGOFF, 0);
            else {
                std::cout << "警告：此操作将注销当前用户，是否继续？(Y/N): ";
                get = _getch();
                if (get == 'Y' || get == 'y') ExitWindowsEx(EWX_LOGOFF, 0);
            }
            break;
        case 3:
            if (!is1) ExitWindowsEx(EWX_POWEROFF | EWX_FORCE, 0);
            else {
                std::cout << "警告：此操作将强制关闭系统，是否继续？(Y/N): ";
                get = _getch();
                if (get == 'Y' || get == 'y') ExitWindowsEx(EWX_POWEROFF | EWX_FORCE, 0);
            }
            break;
        case 4:
            if (!is1) ExitWindowsEx(EWX_REBOOT | EWX_FORCE, 0);
            else {
                std::cout << "警告：此操作将强制重启系统，是否继续？(Y/N): ";
                get = _getch();
                if (get == 'Y' || get == 'y') ExitWindowsEx(EWX_REBOOT | EWX_FORCE, 0);
            }
            break;
        case 5:
            if (!is1) ExitWindowsEx(EWX_LOGOFF | EWX_FORCE, 0);
            else {
                std::cout << "警告：此操作将强制注销当前用户，是否继续？(Y/N): ";
                get = _getch();
                if (get == 'Y' || get == 'y') ExitWindowsEx(EWX_LOGOFF | EWX_FORCE, 0);
            }
            break;
        }

        return true;
    }

    if (_strnicmp(cmdTrimmed.c_str(), "title", 5) == 0) {
        _1t = cmdTrimmed.substr(5, cmdTrimmed.size());
        size_t start = _1t.find_first_not_of(" \t");
        if (start != std::string::npos) _1t = _1t.substr(start);
        SetConsoleTitleA(_1t.c_str());
        return true;
    }

    if (cmdTrimmed.substr(0, 4) == "hash" || cmdTrimmed.substr(0, 4) == "HASH") {
        std::string param = cmdTrimmed.size() > 4 ? cmdTrimmed.substr(4) : "";
        std::string result = HandleHashCommand(param);
        std::cout << result << "\n\n";
        return true;
    }

    if (cmdTrimmed.substr(0, 8) == "st timer" || cmdTrimmed.substr(0, 8) == "ST TIMER") {
        std::string param = cmdTrimmed.size() > 8 ? cmdTrimmed.substr(8) : "";
        size_t s = param.find_first_not_of(" \t");
        if (s != std::string::npos) param = param.substr(s);
        int seconds = atoi(param.c_str());
        HandleTimer(seconds);
        return true;
    }

    if (cmdTrimmed.substr(0, 8) == "st alarm" || cmdTrimmed.substr(0, 8) == "ST ALARM") {
        std::string param = cmdTrimmed.size() > 9 ? cmdTrimmed.substr(9) : "";
        size_t s = param.find_first_not_of(" \t");
        if (s != std::string::npos) param = param.substr(s);

        int seconds = 60;
        std::string message = "闹钟响了！";
        if (!param.empty()) {
            size_t spacePos = param.find(' ');
            if (spacePos != std::string::npos) {
                seconds = atoi(param.substr(0, spacePos).c_str());
                message = param.substr(spacePos + 1);
            }
            else {
                seconds = atoi(param.c_str());
            }
        }
        HandleAlarm(seconds, message);
        return true;
    }

    if (cmdTrimmed.substr(0, 10) == "st weather" || cmdTrimmed.substr(0, 10) == "ST WEATHER") {
        std::string param = cmdTrimmed.size() > 10 ? cmdTrimmed.substr(10) : "";
        size_t s = param.find_first_not_of(" \t");
        if (s != std::string::npos) param = param.substr(s);
        HandleWeather(param);
        return true;
    }

    if (cmdTrimmed == "st quote" || cmdTrimmed == "ST QUOTE") {
        HandleQuote();
        return true;
    }

    if (cmdTrimmed == "st fortune" || cmdTrimmed == "ST FORTUNE") {
        HandleFortune();
        return true;
    }

    if (_strnicmp(cmdTrimmed.c_str(), "assoc", 5) == 0) {
        std::string param = cmdTrimmed.size() > 5 ? cmdTrimmed.substr(5) : "";
        size_t s = param.find_first_not_of(" \t");
        if (s != std::string::npos) param = param.substr(s);
        else param = "";

        if (param.empty()) {
            HKEY hKey;
            if (RegOpenKeyExW(HKEY_CLASSES_ROOT, NULL, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
                DWORD index = 0;
                wchar_t name[256];
                DWORD nameSize;
                while (true) {
                    nameSize = sizeof(name) / sizeof(wchar_t);
                    if (RegEnumKeyExW(hKey, index++, name, &nameSize, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)
                        break;
                    if (name[0] == L'.') {
                        wchar_t value[256];
                        DWORD valueSize = sizeof(value);
                        if (RegQueryValueExW(hKey, name, NULL, NULL, (LPBYTE)value, &valueSize) == ERROR_SUCCESS) {
                            std::wcout << name << L"=" << value << L"\n";
                        }
                    }
                }
                RegCloseKey(hKey);
            }
        }
        else {
            size_t eq = param.find('=');
            if (eq != std::string::npos) {
                std::string ext = param.substr(0, eq);
                std::string type = param.substr(eq + 1);
                std::wstring wExt = U82W_Path(ext);
                std::wstring wType = U82W_Path(type);

                HKEY hKey;
                if (RegCreateKeyExW(HKEY_CLASSES_ROOT, wExt.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
                    RegSetValueExW(hKey, NULL, 0, REG_SZ, (const BYTE*)wType.c_str(), (DWORD)((wType.size() + 1) * sizeof(wchar_t)));
                    RegCloseKey(hKey);
                    if (!silentMode) std::cout << "关联已设置\n\n";
                }
            }
            else {
                std::wstring wParam = U82W_Path(param);
                HKEY hKey;
                if (RegOpenKeyExW(HKEY_CLASSES_ROOT, wParam.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
                    wchar_t value[256];
                    DWORD valueSize = sizeof(value);
                    if (RegQueryValueExW(hKey, NULL, NULL, NULL, (LPBYTE)value, &valueSize) == ERROR_SUCCESS) {
                        std::wcout << wParam << L"=" << value << L"\n";
                    }
                    RegCloseKey(hKey);
                }
            }
        }
        std::cout << "\n";
        return true;
    }

    if (_strnicmp(cmdTrimmed.c_str(), "ftype", 5) == 0) {
        std::string param = cmdTrimmed.size() > 5 ? cmdTrimmed.substr(5) : "";
        size_t s = param.find_first_not_of(" \t");
        if (s != std::string::npos) param = param.substr(s);
        else param = "";

        if (param.empty()) {
            HKEY hKey;
            if (RegOpenKeyExA(HKEY_CLASSES_ROOT, NULL, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
                DWORD index = 0;
                char name[256];
                DWORD nameSize;
                while (true) {
                    nameSize = sizeof(name);
                    if (RegEnumKeyExA(hKey, index++, name, &nameSize, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)
                        break;
                    if (name[0] != '.') {
                        HKEY hSubKey;
                        if (RegOpenKeyExA(HKEY_CLASSES_ROOT, name, 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
                            char value[512];
                            DWORD valueSize = sizeof(value);
                            if (RegQueryValueExA(hSubKey, "shell\\open\\command", NULL, NULL, (LPBYTE)value, &valueSize) == ERROR_SUCCESS) {
                                std::cout << name << "=" << value << "\n";
                            }
                            RegCloseKey(hSubKey);
                        }
                    }
                }
                RegCloseKey(hKey);
            }
        }
        else {
            size_t eq = param.find('=');
            if (eq != std::string::npos) {
                std::string type = param.substr(0, eq);
                std::string cmdLine = param.substr(eq + 1);
                HKEY hKey;
                std::string path = type + "\\shell\\open\\command";
                if (RegCreateKeyExA(HKEY_CLASSES_ROOT, path.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
                    RegSetValueExA(hKey, NULL, 0, REG_SZ, (const BYTE*)cmdLine.c_str(), cmdLine.size() + 1);
                    RegCloseKey(hKey);
                    if (!silentMode) std::cout << "文件类型已设置\n\n";
                }
            }
        }
        std::cout << "\n";
        return true;
    }

    if (_strnicmp(cmdTrimmed.c_str(), "find", 4) == 0) {
        std::string args = cmdTrimmed.size() > 4 ? cmdTrimmed.substr(4) : "";
        bool ignoreCase = false;
        bool invert = false;
        bool countOnly = false;
        bool showLineNum = false;
        std::string searchStr;
        std::string fileName;

        size_t pos = 0;
        while (pos < args.size()) {
            while (pos < args.size() && (args[pos] == ' ' || args[pos] == '\t')) pos++;
            if (pos >= args.size()) break;
            if (args[pos] == '/') {
                if (pos + 1 < args.size()) {
                    char flag = (char)tolower(args[pos + 1]);
                    if (flag == 'i') ignoreCase = true;
                    else if (flag == 'v') invert = true;
                    else if (flag == 'c') countOnly = true;
                    else if (flag == 'n') showLineNum = true;
                }
                pos += 2;
            }
            else {
                break;
            }
        }

        while (pos < args.size() && (args[pos] == ' ' || args[pos] == '\t')) pos++;

        if (pos < args.size() && args[pos] == '"') {
            size_t end = args.find('"', pos + 1);
            if (end != std::string::npos) {
                searchStr = args.substr(pos + 1, end - pos - 1);
                pos = end + 1;
            }
            else {
                searchStr = args.substr(pos + 1);
                pos = args.size();
            }
        }
        else {
            size_t end = args.find(' ', pos);
            if (end == std::string::npos) end = args.size();
            searchStr = args.substr(pos, end - pos);
            pos = end;
        }

        while (pos < args.size() && (args[pos] == ' ' || args[pos] == '\t')) pos++;
        if (pos < args.size()) {
            std::string fileArg = args.substr(pos);
            if (fileArg.size() >= 2 && fileArg.front() == '"' && fileArg.back() == '"') {
                fileArg = fileArg.substr(1, fileArg.size() - 2);
            }
            fileName = ExpandEnvironmentVars(fileArg);
        }

        std::ifstream file;
        std::istream* input = &std::cin;
        if (!fileName.empty()) {
            std::wstring wFileName = U82W_Path(fileName);
            file.open(wFileName.c_str());
            if (!file.is_open()) {
                std::cout << "找不到文件 - " << fileName << "\n\n";
                return true;
            }
            input = &file;
        }

        std::string line;
        int lineNum = 0;
        int matchCount = 0;
        std::string searchLower = searchStr;
        if (ignoreCase) {
            std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);
        }

        while (std::getline(*input, line)) {
            lineNum++;
            bool match = false;
            if (ignoreCase) {
                std::string lineLower = line;
                std::transform(lineLower.begin(), lineLower.end(), lineLower.begin(), ::tolower);
                match = lineLower.find(searchLower) != std::string::npos;
            }
            else {
                match = line.find(searchStr) != std::string::npos;
            }
            if (invert) match = !match;
            if (match) {
                matchCount++;
                if (!countOnly) {
                    if (showLineNum) std::cout << "[" << lineNum << "] ";
                    std::cout << line << "\n";
                }
            }
        }

        if (countOnly) {
            std::cout << matchCount << "\n";
        }
        else if (!countOnly && matchCount == 0 && !invert && !silentMode) {
            std::cout << "找不到字符串 - " << searchStr << "\n";
        }
        std::cout << "\n";
        return true;
    }

    if (_strnicmp(cmdTrimmed.c_str(), "dpath", 5) == 0) {
        std::string param = cmdTrimmed.size() > 5 ? cmdTrimmed.substr(5) : "";
        size_t s = param.find_first_not_of(" \t");
        if (s != std::string::npos) param = param.substr(s);
        else param = "";

        if (param.empty()) {
            char buf[4096];
            DWORD len = GetEnvironmentVariableA("DPATH", buf, sizeof(buf));
            if (len > 0) {
                std::cout << "DPATH=" << buf << "\n";
            }
            else {
                std::cout << "DPATH 未设置\n";
            }
            std::cout << "\nDPATH 是数据搜索路径，用于查找数据文件\n";
            std::cout << "当程序打开文件时，会先在DPATH指定的目录中查找\n\n";
            return true;
        }

        if (param == ";") {
            SetEnvironmentVariableA("DPATH", NULL);
            if (!silentMode) std::cout << "DPATH 已清除\n\n";
        }
        else {
            std::string paths = ExpandEnvironmentVars(param);
            SetEnvironmentVariableA("DPATH", paths.c_str());
            if (!silentMode) {
                std::cout << "DPATH 已设置为: " << paths << "\n";
                std::cout << "数据文件搜索路径已更新\n";
            }
        }

        if (!silentMode && !param.empty() && param != ";") {
            std::cout << "\n当前数据搜索路径:\n";
            std::string dpath = ExpandEnvironmentVars(param);
            size_t pos = 0;
            int index = 1;
            while (pos < dpath.size()) {
                size_t semicolon = dpath.find(';', pos);
                if (semicolon == std::string::npos) semicolon = dpath.size();
                std::string dir = dpath.substr(pos, semicolon - pos);
                if (!dir.empty()) {
                    std::cout << "  " << index++ << ". " << dir << "\n";
                }
                pos = semicolon + 1;
            }
            std::cout << "\n";
        }
        return true;
    }

    if (_strnicmp(cmdTrimmed.c_str(), "mklink", 6) == 0) {
        std::string args = cmdTrimmed.size() > 6 ? cmdTrimmed.substr(6) : "";
        size_t s = args.find_first_not_of(" \t");
        if (s != std::string::npos) args = args.substr(s);

        bool isDir = false;
        if (args.find("/d") == 0 || args.find("/D") == 0) {
            isDir = true;
            args = args.substr(2);
            s = args.find_first_not_of(" \t");
            if (s != std::string::npos) args = args.substr(s);
        }

        size_t sp = args.find(' ');
        if (sp != std::string::npos) {
            std::string link = args.substr(0, sp);
            std::string target = args.substr(sp + 1);
            s = link.find_first_not_of(" \t");
            if (s != std::string::npos) link = link.substr(s);
            size_t e = link.find_last_not_of(" \t");
            if (e != std::string::npos) link = link.substr(0, e + 1);
            s = target.find_first_not_of(" \t");
            if (s != std::string::npos) target = target.substr(s);
            e = target.find_last_not_of(" \t");
            if (e != std::string::npos) target = target.substr(0, e + 1);

            link = ExpandEnvironmentVars(link);
            target = ExpandEnvironmentVars(target);

            DWORD flags = isDir ? SYMBOLIC_LINK_FLAG_DIRECTORY : 0;
            if (CreateSymbolicLinkA(link.c_str(), target.c_str(), flags)) {
                if (!silentMode) std::cout << "符号链接已创建\n\n";
            }
            else {
                std::cout << "创建符号链接失败\n\n";
            }
        }
        else {
            std::cout << "用法: mklink [/d] <链接路径> <目标路径>\n\n";
        }
        return true;
    }

    if (_strnicmp(cmdTrimmed.c_str(), "del", 3) == 0 ||
        _strnicmp(cmdTrimmed.c_str(), "erase", 5) == 0) {

        std::string param;
        if (cmdTrimmed.substr(0, 3) == "del") {
            param = cmdTrimmed.size() > 3 ? cmdTrimmed.substr(3) : "";
        }
        else {
            param = cmdTrimmed.size() > 5 ? cmdTrimmed.substr(5) : "";
        }

        size_t s = param.find_first_not_of(" \t");
        if (s != std::string::npos) param = param.substr(s);

        bool quiet = false;
        size_t qPos = param.find("/q");
        if (qPos == std::string::npos) qPos = param.find("/Q");
        if (qPos != std::string::npos) {
            quiet = true;
            param.erase(qPos, 2);
            size_t s2 = param.find_first_not_of(" \t");
            if (s2 != std::string::npos) param = param.substr(s2);
        }

        if (param.empty()) {
            std::cout << "用法: del <文件名>\n";
            std::cout << "      erase <文件名>\n";
            std::cout << "      del /q <文件名>  (静默删除)\n\n";
            return true;
        }

        if (param.size() >= 2 && param.front() == '"' && param.back() == '"') {
            param = param.substr(1, param.size() - 2);
        }

        param = ExpandEnvironmentVars(param);

        std::wstring wParam = U82W_Path(param);
        WIN32_FIND_DATAW findData;
        HANDLE hFind = FindFirstFileW(wParam.c_str(), &findData);

        if (hFind == INVALID_HANDLE_VALUE) {
            if (!quiet) {
                std::cout << "找不到文件: " << param << "\n\n";
            }
            return true;
        }

        std::wstring wDirPath;
        size_t lastSlash = param.find_last_of("\\/");
        if (lastSlash != std::string::npos) {
            wDirPath = U82W_Path(param.substr(0, lastSlash + 1));
        }
        else {
            wchar_t currentDirW[MAX_PATH];
            GetCurrentDirectoryW(MAX_PATH, currentDirW);
            wDirPath = std::wstring(currentDirW) + L"\\";
        }

        int deletedCount = 0;
        do {
            if (wcscmp(findData.cFileName, L".") != 0 &&
                wcscmp(findData.cFileName, L"..") != 0) {

                if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    std::wstring wFullPath = wDirPath + findData.cFileName;
                    if (DeleteFileW(wFullPath.c_str())) {
                        deletedCount++;
                        if (!quiet) {
                            std::wcout << L"已删除: " << findData.cFileName << L"\n";
                        }
                    }
                    else if (!quiet) {
                        std::wcout << L"删除失败: " << findData.cFileName << L"\n";
                    }
                }
            }
        } while (FindNextFileW(hFind, &findData));

        FindClose(hFind);

        if (!quiet) {
            std::cout << "已删除 " << deletedCount << " 个文件\n\n";
        }
        return true;
    }

    if (_strnicmp(cmdTrimmed.c_str(), "tree", 4) == 0) {
        std::string param = cmdTrimmed.size() > 4 ? cmdTrimmed.substr(4) : "";
        size_t s = param.find_first_not_of(" \t");
        if (s != std::string::npos) param = param.substr(s);
        else param = "";

        bool showFiles = true;
        if (param.find("/f") != std::string::npos || param.find("/F") != std::string::npos) {
            showFiles = true;
            size_t p = param.find("/f");
            if (p == std::string::npos) p = param.find("/F");
            param.erase(p, 2);
            s = param.find_first_not_of(" \t");
            if (s != std::string::npos) param = param.substr(s);
            else param = "";
        }

        std::string targetDir;
        if (param.empty()) {
            char buf[MAX_PATH];
            GetCurrentDirectoryA(MAX_PATH, buf);
            targetDir = buf;
        }
        else {
            if (param.front() == '"' && param.back() == '"') {
                param = param.substr(1, param.size() - 2);
            }
            targetDir = ExpandEnvironmentVars(param);
        }

        std::wstring wTargetDir = U82W_Path(targetDir);
        std::wcout << wTargetDir << L"\n";

        std::function<void(const std::wstring&, const std::wstring&, bool, bool)> TreeFunc;
        TreeFunc = [&](const std::wstring& wPath, const std::wstring& wPrefix, bool isLast, bool showFiles) {
            std::wcout << wPrefix;
            if (isLast) std::wcout << L"└── ";
            else std::wcout << L"├── ";

            size_t pos = wPath.find_last_of(L"\\/");
            std::wstring wName = (pos != std::wstring::npos) ? wPath.substr(pos + 1) : wPath;
            std::wcout << wName << L"\n";

            std::wstring newPrefix = wPrefix;
            if (isLast) newPrefix += L"    ";
            else newPrefix += L"│   ";

            WIN32_FIND_DATAW fd;
            HANDLE hFind = FindFirstFileW((wPath + L"\\*").c_str(), &fd);
            if (hFind == INVALID_HANDLE_VALUE) return;

            std::vector<std::wstring> dirs;
            std::vector<std::wstring> files;

            do {
                if (wcscmp(fd.cFileName, L".") == 0 || wcscmp(fd.cFileName, L"..") == 0) continue;
                if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    dirs.push_back(fd.cFileName);
                }
                else if (showFiles) {
                    files.push_back(fd.cFileName);
                }
            } while (FindNextFileW(hFind, &fd));
            FindClose(hFind);

            for (size_t i = 0; i < dirs.size(); i++) {
                bool last = (i == dirs.size() - 1 && files.empty());
                TreeFunc(wPath + L"\\" + dirs[i], newPrefix, last, showFiles);
            }

            for (size_t i = 0; i < files.size(); i++) {
                bool last = (i == files.size() - 1);
                std::wcout << newPrefix;
                if (last) std::wcout << L"└── ";
                else std::wcout << L"├── ";
                std::wcout << files[i] << L"\n";
            }
            };

        TreeFunc(wTargetDir, L"", true, showFiles);
        std::cout << "\n";
        return true;
    }

    if (_strnicmp(cmdTrimmed.c_str(), "attrib", 6) == 0) {
        std::string args = cmdTrimmed.size() > 6 ? cmdTrimmed.substr(6) : "";
        size_t s = args.find_first_not_of(" \t");
        if (s != std::string::npos) args = args.substr(s);

        std::string operation;
        std::string target;

        size_t pos = 0;
        while (pos < args.size()) {
            if (args[pos] == ' ' || args[pos] == '\t') { pos++; continue; }
            if (args[pos] == '/') {
                char opt = (char)toupper(args[pos + 1]);
                if (opt == '?') {
                    std::cout << "用法: attrib [+R|-R] [+A|-A] [+S|-S] [+H|-H] [文件]\n";
                    std::cout << "  R: 只读属性  A: 存档属性  S: 系统属性  H: 隐藏属性\n\n";
                    return true;
                }
                pos += 2;
            }
            else if (args[pos] == '+' || args[pos] == '-') {
                operation = args.substr(pos, 2);
                pos += 2;
            }
            else {
                target = args.substr(pos);
                break;
            }
        }

        if (target.empty()) {
            target = "*.*";
        }
        target = ExpandEnvironmentVars(target);

        std::wstring wTarget = U82W_Path(target);
        WIN32_FIND_DATAW fd;
        HANDLE hFind = FindFirstFileW(wTarget.c_str(), &fd);
        if (hFind == INVALID_HANDLE_VALUE) {
            std::cout << "找不到文件\n\n";
            return true;
        }

        std::wstring wDir;
        {
            std::string path = target;
            size_t lastSlash = path.find_last_of("\\/");
            std::string dirStr = (lastSlash != std::string::npos) ? path.substr(0, lastSlash + 1) : "";
            wDir = U82W_Path(dirStr);
        }

        do {
            if (wcscmp(fd.cFileName, L".") == 0 || wcscmp(fd.cFileName, L"..") == 0) continue;

            std::wstring wFullPath = wDir + fd.cFileName;
            DWORD attrs = GetFileAttributesW(wFullPath.c_str());

            if (!operation.empty()) {
                char op = operation[0];
                char attrFlag = (char)toupper(operation[1]);
                DWORD flag = 0;
                if (attrFlag == 'R') flag = FILE_ATTRIBUTE_READONLY;
                else if (attrFlag == 'H') flag = FILE_ATTRIBUTE_HIDDEN;
                else if (attrFlag == 'S') flag = FILE_ATTRIBUTE_SYSTEM;
                else if (attrFlag == 'A') flag = FILE_ATTRIBUTE_ARCHIVE;

                if (op == '+') attrs |= flag;
                else attrs &= ~flag;
                SetFileAttributesW(wFullPath.c_str(), attrs);
            }

            if (!operation.empty() && !silentMode) {
                std::wcout << L"已修改: " << fd.cFileName << L"\n";
            }
            else if (operation.empty()) {
                std::cout << (attrs & FILE_ATTRIBUTE_READONLY ? "R" : "-");
                std::cout << (attrs & FILE_ATTRIBUTE_ARCHIVE ? "A" : "-");
                std::cout << (attrs & FILE_ATTRIBUTE_SYSTEM ? "S" : "-");
                std::cout << (attrs & FILE_ATTRIBUTE_HIDDEN ? "H" : "-");
                std::cout << "  ";
                std::wcout << fd.cFileName << L"\n";
            }
        } while (FindNextFileW(hFind, &fd));
        FindClose(hFind);

        std::cout << "\n";
        return true;
    }

    if (_strnicmp(cmdTrimmed.c_str(), "tasklist", 8) == 0) {
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot == INVALID_HANDLE_VALUE) {
            std::cout << "无法获取进程列表\n\n";
            return true;
        }

        PROCESSENTRY32 pe;
        pe.dwSize = sizeof(PROCESSENTRY32);

        std::cout << "映像名称\t\t\t PID\t\t会话名\t\t内存使用\n";
        std::cout << "=========================================================\n";

        if (Process32First(hSnapshot, &pe)) {
            do {
                HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe.th32ProcessID);
                SIZE_T memUsage = 0;
                if (hProcess) {
                    PROCESS_MEMORY_COUNTERS pmc;
                    if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
                        memUsage = pmc.WorkingSetSize / 1024;
                    }
                    CloseHandle(hProcess);
                }

                printf("%-25s\t%6d\t\tConsole\t\t%6d K\n", pe.szExeFile, pe.th32ProcessID, memUsage);
            } while (Process32Next(hSnapshot, &pe));
        }

        CloseHandle(hSnapshot);
        std::cout << "\n";
        return true;
    }

    if (_strnicmp(cmdTrimmed.c_str(), "ipconfig", 8) == 0) {
        PIP_ADAPTER_INFO pAdapterInfo = NULL;
        ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
        pAdapterInfo = (IP_ADAPTER_INFO*)malloc(ulOutBufLen);

        DWORD dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen);
        if (dwRetVal == ERROR_BUFFER_OVERFLOW) {
            free(pAdapterInfo);
            pAdapterInfo = (IP_ADAPTER_INFO*)malloc(ulOutBufLen);
            dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen);
        }

        if (dwRetVal == NO_ERROR) {
            PIP_ADAPTER_INFO pAdapter = pAdapterInfo;
            while (pAdapter) {
                std::cout << "\n网卡: " << pAdapter->Description << "\n";
                std::cout << "   MAC地址: ";
                for (UINT i = 0; i < pAdapter->AddressLength; i++) {
                    printf("%02X%s", pAdapter->Address[i], (i == pAdapter->AddressLength - 1) ? "" : "-");
                }
                std::cout << "\n   IPv4地址: " << pAdapter->IpAddressList.IpAddress.String << "\n";
                std::cout << "   子网掩码: " << pAdapter->IpAddressList.IpMask.String << "\n";
                std::cout << "   默认网关: " << pAdapter->GatewayList.IpAddress.String << "\n";
                pAdapter = pAdapter->Next;
            }
        }
        else {
            std::cout << "获取适配器信息失败\n";
        }

        free(pAdapterInfo);
        std::cout << "\n";
        return true;
    }

    if (cmdTrimmed.substr(0, 4) == "date" || cmdTrimmed.substr(0, 4) == "DATE") {
        SYSTEMTIME st;
        GetSystemTime(&st);

        std::string param = cmdTrimmed.size() > 4 ? cmdTrimmed.substr(4) : "";
        size_t s = param.find_first_not_of(" \t");
        if (s != std::string::npos) param = param.substr(s);

        if (param.empty()) {
            printf("当前日期: %d/%02d/%02d\n", st.wYear, st.wMonth, st.wDay);
            std::cout << "输入新日期 (格式: YYYY-MM-DD) 或直接按回车跳过: ";
            std::string newDate;
            std::getline(std::cin, newDate);
            if (!newDate.empty()) {
                std::cout << "设置日期需要管理员权限\n";
            }
        }
        std::cout << std::endl;
        return true;
    }

    if (cmdTrimmed.substr(0, 4) == "time" || cmdTrimmed.substr(0, 4) == "TIME") {
        SYSTEMTIME st;
        GetSystemTime(&st);

        std::string param = cmdTrimmed.size() > 4 ? cmdTrimmed.substr(4) : "";
        size_t s = param.find_first_not_of(" \t");
        if (s != std::string::npos) param = param.substr(s);

        if (param.empty()) {
            printf("当前时间: %02d:%02d:%02d.%02d\n",
                st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
            std::cout << "输入新时间 (格式: HH:MM:SS) 或直接按回车跳过: ";
            std::string newTime;
            std::getline(std::cin, newTime);
            if (!newTime.empty()) {
                std::cout << "设置时间需要管理员权限\n";
            }
        }
        std::cout << std::endl;
        return true;
    }

    if (_strnicmp(cmdTrimmed.c_str(), "call", 4) == 0) {
        std::string param = cmdTrimmed.size() > 4 ? cmdTrimmed.substr(4) : "";
        size_t s = param.find_first_not_of(" \t");
        if (s != std::string::npos) param = param.substr(s);
        s = param.find_last_not_of(" \t");
        if (s != std::string::npos) param = param.substr(0, s + 1);

        if (param.empty()) {
            std::cout << "用法: call <批处理文件>\n";
            std::cout << "       call :标签名\n\n";
            return true;
        }

        if (!param.empty() && param[0] == ':') {
            std::string label = param.substr(1);
            size_t pos = FindLabel(label);
            if (pos != (size_t)-1) {
                g_currentLine = pos;
                if (!silentMode) std::cout << "跳转到标签: " << label << "\n";
            }
            else {
                std::cout << "标签不存在: " << label << "\n";
            }
            return true;
        }

        std::string batchFile = ExpandEnvironmentVars(param);
        std::ifstream file(batchFile);

        if (!file.is_open()) {
            std::cout << "找不到批处理文件: " << batchFile << "\n\n";
            return true;
        }

        auto savedLines = g_scriptLines;
        auto savedLine = g_currentLine;

        g_scriptLines.clear();
        g_currentLine = 0;

        std::string line;
        while (std::getline(file, line)) {
            while (!line.empty() && (line.back() == '\r' || line.back() == '\n')) {
                line.pop_back();
            }
            g_scriptLines.push_back(line);
        }
        file.close();

        bool echoOn = true;
        while (g_currentLine < g_scriptLines.size()) {
            std::string currentLine = g_scriptLines[g_currentLine];
            g_currentLine++;

            size_t start = currentLine.find_first_not_of(" \t");
            if (start == std::string::npos) continue;
            size_t end = currentLine.find_last_not_of(" \t");
            currentLine = currentLine.substr(start, end - start + 1);

            if (currentLine.substr(0, 2) == "::") continue;

            if (currentLine == "@echo off" || currentLine == "@ECHO OFF") {
                echoOn = false;
                continue;
            }
            if (currentLine == "@echo on" || currentLine == "@ECHO ON") {
                echoOn = true;
                continue;
            }

            if (echoOn && currentLine[0] != '@') {
                std::cout << currentLine << "\n";
            }

            if (!currentLine.empty() && currentLine[0] == '@') {
                currentLine = currentLine.substr(1);
            }

            if (currentLine.substr(0, 4) == "echo" || currentLine.substr(0, 4) == "ECHO") {
                std::string msg = currentLine.size() > 5 ? currentLine.substr(5) : "";
                if (msg == "off" || msg == "OFF") {
                    echoOn = false;
                    g_echoState = false;
                }
                else if (msg == "on" || msg == "ON") {
                    echoOn = true;
                    g_echoState = true;
                }
                else {
                    msg = ExpandEnvironmentVars(msg);
                    std::cout << msg << "\n";
                }
                continue;
            }

            if (currentLine == "pause" || currentLine == "PAUSE") {
                std::cout << "按任意键继续...";
                (void)_getch();
                std::cout << "\n";
                continue;
            }

            if (currentLine == "cls" || currentLine == "CLS") {
                Wclear();
                continue;
            }

            if (cmdTrimmed == "exit" || cmdTrimmed == "EXIT") {
                bool exitBatch = false;
                int exitCode = 0;

                size_t spacePos = cmdTrimmed.find(' ');
                if (spacePos != std::string::npos) {
                    std::string param = cmdTrimmed.substr(spacePos + 1);
                    if (param == "/b" || param == "/B") {
                        exitBatch = true;
                        size_t codePos = param.find(' ');
                        if (codePos != std::string::npos) {
                            std::string codeStr = param.substr(codePos + 1);
                            exitCode = atoi(codeStr.c_str());
                        }
                    }
                }

                if (exitBatch) {
                    if (!g_scriptLines.empty()) {
                        g_currentLine = g_scriptLines.size();
                        if (!silentMode) {
                            std::cout << "退出批处理 (exit code: " << exitCode << ")\n";
                        }
                    }
                }
                else {
                    exit(exitCode);
                }
                return true;
            }

            if (currentLine.substr(0, 4) == "goto" || currentLine.substr(0, 4) == "GOTO") {
                std::string label = currentLine.size() > 5 ? currentLine.substr(5) : "";
                size_t ls = label.find_first_not_of(" \t");
                if (ls != std::string::npos) label = label.substr(ls);
                size_t le = label.find_last_not_of(" \t");
                if (le != std::string::npos) label = label.substr(0, le + 1);

                size_t pos = FindLabel(label);
                if (pos != (size_t)-1) {
                    g_currentLine = pos;
                }
                continue;
            }

            std::string expandedCmd = ExpandEnvironmentVars(currentLine);

            if (HandleBuiltinCommand(expandedCmd, silentMode)) {
                continue;
            }

            char* cmdBuf = _strdup(expandedCmd.c_str());
            STARTUPINFOA si = { sizeof(si) };
            PROCESS_INFORMATION pi = { 0 };
            si.dwFlags = STARTF_USESTDHANDLES;
            si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
            si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
            si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

            if (CreateProcessA(NULL, cmdBuf, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
                WaitForSingleObject(pi.hProcess, INFINITE);
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
            }
            else {
                if (currentLine.find(")") == std::string::npos) {
                    std::cout << "无法执行: " << currentLine << "\n";
                }

            }
            free(cmdBuf);
        }

        g_scriptLines = savedLines;
        g_currentLine = savedLine;

        return true;
    }

    if (_strnicmp(cmdTrimmed.c_str(), "goto", 4) == 0) {
        std::string label = cmdTrimmed.size() > 4 ? cmdTrimmed.substr(4) : "";
        size_t s = label.find_first_not_of(" \t");
        if (s != std::string::npos) label = label.substr(s);
        size_t e = label.find_last_not_of(" \t");
        if (e != std::string::npos) label = label.substr(0, e + 1);

        if (label.empty()) {
            std::cout << "用法: goto <标签名>\n";
            std::cout << "标签定义: :标签名\n\n";
            return true;
        }

        size_t pos = FindLabel(label);
        if (pos != (size_t)-1) {
            g_currentLine = pos;
            if (!silentMode) std::cout << "跳转到标签: " << label << "\n";
        }
        else {
            std::cout << "找不到标签: " << label << "\n";
        }
        return true;
    }

    if (_strnicmp(cmdTrimmed.c_str(), "if", 2) == 0) {
        std::string rest = cmdTrimmed.size() > 2 ? cmdTrimmed.substr(2) : "";
        size_t s = rest.find_first_not_of(" \t");
        if (s != std::string::npos) rest = rest.substr(s);

        bool condition = false;
        size_t cmdStart = 0;
        bool hasElse = false;
        std::string elseCommand;

        size_t elsePos = rest.find(" else ");
        if (elsePos != std::string::npos) {
            hasElse = true;
            elseCommand = rest.substr(elsePos + 6);
            rest = rest.substr(0, elsePos);
        }

        if (rest.find("exist ") == 0) {
            std::string path = rest.substr(6);
            size_t sp = path.find(' ');
            if (sp != std::string::npos) {
                cmdStart = sp;
                path = path.substr(0, sp);
            }
            path = ExpandEnvironmentVars(path);
            DWORD attrs = GetFileAttributesA(path.c_str());
            condition = (attrs != INVALID_FILE_ATTRIBUTES);
            if (!silentMode && cmdStart == 0) {
                std::cout << (condition ? "文件存在" : "文件不存在") << "\n";
            }
        }
        else if (rest.find("errorlevel ") == 0) {
            int level = 0;
            std::string num = rest.substr(11);
            size_t sp = num.find(' ');
            if (sp != std::string::npos) {
                cmdStart = sp;
                num = num.substr(0, sp);
            }
            level = atoi(num.c_str());
            condition = (GetLastError() >= (DWORD)level);
        }
        else if (rest.find("defined ") == 0) {
            std::string varName = rest.substr(8);
            size_t sp = varName.find(' ');
            if (sp != std::string::npos) {
                cmdStart = sp;
                varName = varName.substr(0, sp);
            }
            char buf[4096];
            DWORD len = GetEnvironmentVariableA(varName.c_str(), buf, sizeof(buf));
            condition = (len > 0);
        }
        else if (rest.find("==") != std::string::npos) {
            size_t eqPos = rest.find("==");
            std::string left = rest.substr(0, eqPos);
            std::string right = rest.substr(eqPos + 2);

            size_t sp = right.find(' ');
            if (sp != std::string::npos) {
                cmdStart = sp;
                right = right.substr(0, sp);
            }

            if (left.size() >= 2 && left.front() == '"' && left.back() == '"')
                left = left.substr(1, left.size() - 2);
            if (right.size() >= 2 && right.front() == '"' && right.back() == '"')
                right = right.substr(1, right.size() - 2);

            left = ExpandEnvironmentVars(left);
            right = ExpandEnvironmentVars(right);

            bool ignoreCase = false;
            if (left.find("/i") == 0) {
                ignoreCase = true;
                left = left.substr(2);
                size_t ts = left.find_first_not_of(" \t");
                if (ts != std::string::npos) left = left.substr(ts);
            }

            if (ignoreCase) {
                std::transform(left.begin(), left.end(), left.begin(), ::tolower);
                std::transform(right.begin(), right.end(), right.begin(), ::tolower);
            }
            condition = (left == right);
        }
        else if (rest.find("not ") == 0) {
            std::string subCond = rest.substr(4);
            if (subCond.find("exist ") == 0) {
                std::string path = subCond.substr(6);
                path = ExpandEnvironmentVars(path);
                condition = (GetFileAttributesA(path.c_str()) == INVALID_FILE_ATTRIBUTES);
                cmdStart = subCond.find(' ', 6);
                if (cmdStart != std::string::npos) cmdStart = cmdStart - 4;
            }
        }

        if (condition && cmdStart > 0) {
            std::string command = rest.substr(cmdStart);
            s = command.find_first_not_of(" \t");
            if (s != std::string::npos) command = command.substr(s);
            if (command.front() == '(') {
                command = command.substr(1);
                size_t e = command.find_last_not_of(" \t");
                if (e != std::string::npos) command = command.substr(0, e + 1);
                if (command.back() == ')') command.pop_back();
            }
            HandleBuiltinCommand(command, silentMode);
        }
        else if (!condition && hasElse) {
            HandleBuiltinCommand(elseCommand, silentMode);
        }

        return true;
    }

    if (_strnicmp(cmdTrimmed.c_str(), "for", 3) == 0) {
        std::string rest = cmdTrimmed.size() > 3 ? cmdTrimmed.substr(3) : "";
        size_t s = rest.find_first_not_of(" \t");
        if (s != std::string::npos) rest = rest.substr(s);

        if (rest.find("/r") == 0 || rest.find("/R") == 0) {
            HandleForRecursive(rest);
            return true;
        }
        rest = cmdTrimmed.size() > 3 ? cmdTrimmed.substr(3) : "";
        s = rest.find_first_not_of(" \t");
        if (s != std::string::npos) rest = rest.substr(s);

        bool parseFile = false;
        bool skipEmpty = false;
        std::string delims = " \t";
        std::string fileToParse;

        if (rest.find("/f") == 0) {
            parseFile = true;
            rest = rest.substr(2);
            s = rest.find_first_not_of(" \t");
            if (s != std::string::npos) rest = rest.substr(s);

            if (rest.find("skip=") == 0) {
                rest = rest.substr(5);
            }
            if (rest.find("delims=") == 0) {
                delims = rest.substr(7);
                size_t spacePos = delims.find(' ');
                if (spacePos != std::string::npos) {
                    rest = delims.substr(spacePos);
                    delims = delims.substr(0, spacePos);
                }
                else {
                    rest = "";
                }
            }
        }

        size_t p1 = rest.find('%');
        if (p1 != std::string::npos && p1 + 1 < rest.size()) {
            char var = rest[p1 + 1];
            size_t p2 = rest.find("in", p1 + 2);
            if (p2 != std::string::npos) {
                size_t p3 = rest.find('(', p2 + 2);
                if (p3 != std::string::npos) {
                    size_t p4 = rest.find(')', p3 + 1);
                    if (p4 != std::string::npos) {
                        std::string setStr = rest.substr(p3 + 1, p4 - p3 - 1);
                        size_t p5 = rest.find("do", p4 + 1);
                        if (p5 != std::string::npos) {
                            std::string command = rest.substr(p5 + 2);
                            s = command.find_first_not_of(" \t");
                            if (s != std::string::npos) command = command.substr(s);

                            std::vector<std::string> items;

                            if (parseFile && !setStr.empty()) {
                                std::string filename = ExpandEnvironmentVars(setStr);
                                std::ifstream file(filename);
                                if (file.is_open()) {
                                    std::string line;
                                    while (std::getline(file, line)) {
                                        size_t start = 0;
                                        while (start < line.size()) {
                                            size_t end = line.find_first_of(delims, start);
                                            if (end == std::string::npos) end = line.size();
                                            std::string token = line.substr(start, end - start);
                                            if (!token.empty() || !skipEmpty) {
                                                items.push_back(token);
                                            }
                                            start = end + 1;
                                        }
                                    }
                                    file.close();
                                }
                            }
                            else {
                                size_t pos = 0;
                                bool inQuote = false;
                                std::string current;

                                while (pos < setStr.size()) {
                                    char c = setStr[pos];
                                    if (c == '"') {
                                        inQuote = !inQuote;
                                        current += c;
                                    }
                                    else if (!inQuote && (c == ' ' || c == ',')) {
                                        if (!current.empty()) {
                                            if (current.front() == '"' && current.back() == '"') {
                                                current = current.substr(1, current.size() - 2);
                                            }
                                            WIN32_FIND_DATAA fd;
                                            HANDLE hFind = FindFirstFileA(current.c_str(), &fd);
                                            if (hFind != INVALID_HANDLE_VALUE) {
                                                do {
                                                    if (strcmp(fd.cFileName, ".") != 0 &&
                                                        strcmp(fd.cFileName, "..") != 0) {
                                                        items.push_back(fd.cFileName);
                                                    }
                                                } while (FindNextFileA(hFind, &fd));
                                                FindClose(hFind);
                                            }
                                            else {
                                                items.push_back(current);
                                            }
                                            current.clear();
                                        }
                                    }
                                    else {
                                        current += c;
                                    }
                                    pos++;
                                }
                                if (!current.empty()) {
                                    if (current.front() == '"' && current.back() == '"') {
                                        current = current.substr(1, current.size() - 2);
                                    }
                                    items.push_back(current);
                                }
                            }

                            std::string envVarName(1, var);
                            int loopCount = 0;
                            for (const auto& item : items) {
                                SetEnvironmentVariableA(envVarName.c_str(), item.c_str());

                                std::string cmdCopy = command;
                                size_t vpos = 0;
                                while ((vpos = cmdCopy.find('%' + envVarName, vpos)) != std::string::npos) {
                                    cmdCopy.replace(vpos, 2, item);
                                    vpos += item.size();
                                }

                                if (!silentMode) {
                                    std::cout << "循环 " << ++loopCount << ": ";
                                }
                                HandleBuiltinCommand(cmdCopy, silentMode);
                            }

                            if (!silentMode && !items.empty()) {
                                std::cout << "共执行 " << loopCount << " 次循环\n";
                            }
                            return true;
                        }
                    }
                }
            }
        }

        size_t lPos = rest.find("/l");
        if (lPos != std::string::npos) {
            size_t startP = rest.find('(', lPos);
            if (startP != std::string::npos) {
                size_t endP = rest.find(')', startP);
                if (endP != std::string::npos) {
                    std::string range = rest.substr(startP + 1, endP - startP - 1);
                    int start, step, end;
                    if (sscanf_s(range.c_str(), "%d,%d,%d", &start, &step, &end) == 3) {
                        size_t doPos = rest.find("do", endP);
                        if (doPos != std::string::npos) {
                            std::string command = rest.substr(doPos + 2);
                            size_t pctPos = rest.find('%', lPos);
                            if (pctPos != std::string::npos && pctPos + 1 < rest.size()) {
                                char var = rest[pctPos + 1];
                                std::string envVarName(1, var);

                                for (int i = start; (step > 0 ? i <= end : i >= end); i += step) {
                                    char numStr[16];
                                    sprintf_s(numStr, "%d", i);
                                    SetEnvironmentVariableA(envVarName.c_str(), numStr);
                                    HandleBuiltinCommand(command, silentMode);
                                }
                                return true;
                            }
                        }
                    }
                }
            }
        }

        if (rest.find("/r") == 0) {
            // 先提取循环变量名（如 %%i 中的 i）
            char varName = 'i';
            size_t pctPos = rest.find('%');
            if (pctPos != std::string::npos && pctPos + 1 < rest.size()) {
                varName = rest[pctPos + 1];
            }

            std::string rootDir = extractRootDir(rest);
            std::string command = extractCommand(rest);


            std::function<void(const std::string&)> walkDir;

            walkDir = [&walkDir, &command, varName](const std::string& dir) {
                WIN32_FIND_DATAA fd;
                HANDLE hFind = FindFirstFileA((dir + "\\*").c_str(), &fd);
                if (hFind != INVALID_HANDLE_VALUE) {
                    do {
                        if (strcmp(fd.cFileName, ".") != 0 && strcmp(fd.cFileName, "..") != 0) {
                            std::string fullPath = dir + "\\" + fd.cFileName;
                            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                                walkDir(fullPath);
                            }
                            executeForCommand(command, fullPath, varName);
                        }
                    } while (FindNextFileA(hFind, &fd));
                    FindClose(hFind);
                }
                };

            walkDir(rootDir);
            return true;
        }

        if (!silentMode) {
            std::cout << "for 命令格式错误\n";
            std::cout << "用法:\n";
            std::cout << "  for %%i in (set) do command\n";
            std::cout << "  for /l %%i in (start,step,end) do command\n";
            std::cout << "  for /f \"delims=,\" %%i in (file.txt) do command\n";
        }
        return true;
    }



    if (_strnicmp(cmdTrimmed.c_str(), "prompt", 6) == 0) {
        std::string newPrompt = cmdTrimmed.size() > 6 ? cmdTrimmed.substr(6) : "";
        size_t s = newPrompt.find_first_not_of(" \t");
        if (s != std::string::npos) newPrompt = newPrompt.substr(s);
        else newPrompt = "$P$G";

        SetEnvironmentVariableA("PROMPT", newPrompt.c_str());
        if (!silentMode) std::cout << "命令提示符已修改\n\n";
        return true;
    }

    if (_strnicmp(cmdTrimmed.c_str(), "verify", 6) == 0) {
        std::string param = cmdTrimmed.size() > 6 ? cmdTrimmed.substr(6) : "";
        size_t s = param.find_first_not_of(" \t");
        if (s != std::string::npos) param = param.substr(s);

        if (param == "on") {
            SetEnvironmentVariableA("VERIFY", "on");
            if (!silentMode) std::cout << "VERIFY ON\n\n";
        }
        else if (param == "off") {
            SetEnvironmentVariableA("VERIFY", "off");
            if (!silentMode) std::cout << "VERIFY OFF\n\n";
        }
        else {
            char buf[16];
            GetEnvironmentVariableA("VERIFY", buf, sizeof(buf));
            std::cout << "VERIFY " << (strcmp(buf, "off") == 0 ? "OFF" : "ON") << " 状态\n\n";
        }
        return true;
    }

    if (cmdTrimmed.substr(0, 5) == "shift" || cmdTrimmed.substr(0, 5) == "SHIFT") {
        if (g_scriptLines.empty() || g_batchArgs.empty()) {
            if (!silentMode) {
                std::cout << "shift 命令只能在批处理脚本中使用\n\n";
            }
        }
        else {
            std::vector<std::string>& args = g_batchArgs.back();
            if (!args.empty()) {
                args.erase(args.begin());

                for (size_t i = 0; i < args.size() && i < 10; i++) {
                    char varName[8];
                    sprintf_s(varName, "%zu", i);
                    SetEnvironmentVariableA(varName, args[i].c_str());
                }

                if (!silentMode) {
                    std::cout << "参数已左移，剩余 " << args.size() << " 个参数\n";
                }
            }
        }
        std::cout << std::endl;
        return true;
    }

    if (_strnicmp(cmdTrimmed.c_str(), "copy", 4) == 0) {
        std::string args = cmdTrimmed.size() > 4 ? cmdTrimmed.substr(4) : "";
        size_t s = args.find_first_not_of(" \t");
        if (s != std::string::npos) args = args.substr(s);

        if (args.empty()) {
            std::cout << "用法: copy <源文件> <目标文件>\n";
            std::cout << "示例: copy a.txt b.txt\n";
            std::cout << "      copy C:\\source.txt D:\\dest.txt\n\n";
            return true;
        }

        std::string source, dest;
        bool inQuote = false;
        std::string current;

        for (char c : args) {
            if (c == '"') {
                inQuote = !inQuote;
                if (!current.empty() && !inQuote) {
                    if (source.empty()) source = current;
                    else dest = current;
                    current.clear();
                }
            }
            else if (c == ' ' && !inQuote) {
                if (!current.empty()) {
                    if (source.empty()) source = current;
                    else dest = current;
                    current.clear();
                }
            }
            else {
                current += c;
            }
        }
        if (!current.empty()) {
            if (source.empty()) source = current;
            else dest = current;
        }

        if (source.empty() || dest.empty()) {
            std::cout << "错误: 请指定源文件和目标文件\n\n";
            return true;
        }

        source = ExpandEnvironmentVars(source);
        dest = ExpandEnvironmentVars(dest);

        BOOL result = CopyFileW(U82W_Path(source).c_str(), U82W_Path(dest).c_str(), FALSE);

        if (result) {
            std::cout << "已复制 1 个文件: " << source << " -> " << dest << "\n\n";
        }
        else {
            DWORD err = GetLastError();
            switch (err) {
            case ERROR_FILE_NOT_FOUND:
                std::cout << "错误: 找不到源文件 \"" << source << "\"\n\n";
                break;
            case ERROR_ACCESS_DENIED:
                std::cout << "错误: 访问被拒绝\n\n";
                break;
            case ERROR_ALREADY_EXISTS:
                std::cout << "错误: 目标文件已存在\n\n";
                break;
            default:
                std::cout << "复制失败，错误码: " << err << "\n\n";
            }
        }
        return true;
    }

    if (_strnicmp(cmdTrimmed.c_str(), "move", 4) == 0) {
        std::string args = cmdTrimmed.size() > 4 ? cmdTrimmed.substr(4) : "";
        size_t s = args.find_first_not_of(" \t");
        if (s != std::string::npos) args = args.substr(s);

        if (args.empty()) {
            std::cout << "用法: move <源文件> <目标文件>\n";
            std::cout << "示例: move a.txt b.txt\n";
            std::cout << "      move C:\\source.txt D:\\dest.txt\n\n";
            return true;
        }

        std::string source, dest;
        bool inQuote = false;
        std::string current;

        for (char c : args) {
            if (c == '"') {
                inQuote = !inQuote;
                if (!current.empty() && !inQuote) {
                    if (source.empty()) source = current;
                    else dest = current;
                    current.clear();
                }
            }
            else if (c == ' ' && !inQuote) {
                if (!current.empty()) {
                    if (source.empty()) source = current;
                    else dest = current;
                    current.clear();
                }
            }
            else {
                current += c;
            }
        }
        if (!current.empty()) {
            if (source.empty()) source = current;
            else dest = current;
        }

        if (source.empty() || dest.empty()) {
            std::cout << "错误: 请指定源文件和目标文件\n\n";
            return true;
        }

        source = ExpandEnvironmentVars(source);
        dest = ExpandEnvironmentVars(dest);

        BOOL result = MoveFileW(U82W_Path(source).c_str(), U82W_Path(dest).c_str());

        if (result) {
            std::cout << "已移动/重命名: " << source << " -> " << dest << "\n\n";
        }
        else {
            DWORD err = GetLastError();
            switch (err) {
            case ERROR_FILE_NOT_FOUND:
                std::cout << "错误: 找不到源文件 \"" << source << "\"\n\n";
                break;
            case ERROR_ACCESS_DENIED:
                std::cout << "错误: 访问被拒绝\n\n";
                break;
            case ERROR_ALREADY_EXISTS:
                std::cout << "错误: 目标文件已存在\n\n";
                break;
            default:
                std::cout << "移动失败，错误码: " << err << "\n\n";
            }
        }
        return true;
    }

    if (_strnicmp(cmdTrimmed.c_str(), "mkdir", 5) == 0 ||
        _strnicmp(cmdTrimmed.c_str(), "md", 2) == 0) {

        std::string args = cmdTrimmed;
        size_t spacePos = args.find(' ');
        std::string dirPath;

        if (spacePos != std::string::npos) {
            dirPath = args.substr(spacePos + 1);
        }
        else {
            dirPath = "";
        }

        size_t s = dirPath.find_first_not_of(" \t");
        if (s != std::string::npos) dirPath = dirPath.substr(s);
        size_t e = dirPath.find_last_not_of(" \t\"");
        if (e != std::string::npos) dirPath = dirPath.substr(0, e + 1);

        if (dirPath.size() >= 2 && dirPath.front() == '"' && dirPath.back() == '"') {
            dirPath = dirPath.substr(1, dirPath.size() - 2);
        }

        if (dirPath.empty()) {
            std::cout << "用法: mkdir <目录名>\n";
            std::cout << "      md <目录名>\n";
            std::cout << "示例: mkdir NewFolder\n";
            std::cout << "      md \"C:\\My Folder\"\n\n";
            return true;
        }

        dirPath = ExpandEnvironmentVars(dirPath);

        BOOL result = CreateDirectoryW(U82W_Path(dirPath).c_str(), NULL);

        if (result) {
            std::cout << "目录已创建: " << dirPath << "\n\n";
        }
        else {
            DWORD err = GetLastError();
            if (err == ERROR_ALREADY_EXISTS) {
                std::cout << "错误: 目录已存在\n\n";
            }
            else if (err == ERROR_PATH_NOT_FOUND) {
                std::cout << "错误: 路径不存在\n\n";
            }
            else {
                std::cout << "创建失败，错误码: " << err << "\n\n";
            }
        }
        return true;
    }

    if (_strnicmp(cmdTrimmed.c_str(), "rmdir", 5) == 0 ||
        _strnicmp(cmdTrimmed.c_str(), "rd", 2) == 0) {

        std::string args = cmdTrimmed;
        size_t spacePos = args.find(' ');
        std::string dirPath;
        bool removeRecursively = false;

        if (spacePos != std::string::npos) {
            dirPath = args.substr(spacePos + 1);
        }
        else {
            dirPath = "";
        }

        if (dirPath.find("/s") != std::string::npos || dirPath.find("/S") != std::string::npos) {
            removeRecursively = true;
            size_t pos = dirPath.find("/s");
            if (pos == std::string::npos) pos = dirPath.find("/S");
            dirPath.erase(pos, 2);
        }

        size_t s = dirPath.find_first_not_of(" \t");
        if (s != std::string::npos) dirPath = dirPath.substr(s);
        size_t e = dirPath.find_last_not_of(" \t\"");
        if (e != std::string::npos) dirPath = dirPath.substr(0, e + 1);

        if (dirPath.size() >= 2 && dirPath.front() == '"' && dirPath.back() == '"') {
            dirPath = dirPath.substr(1, dirPath.size() - 2);
        }

        if (dirPath.empty()) {
            std::cout << "用法: rmdir <目录名> [/s]\n";
            std::cout << "      rd <目录名> [/s]\n";
            std::cout << "      /s  - 递归删除目录及其所有内容\n";
            std::cout << "示例: rmdir EmptyFolder\n";
            std::cout << "      rd /s NonEmptyFolder\n\n";
            return true;
        }

        dirPath = ExpandEnvironmentVars(dirPath);

        BOOL result;

        if (removeRecursively) {
            result = DeleteDirectoryRecursive(U82W_Path(dirPath));
        }
        else {
            result = RemoveDirectoryW(U82W_Path(dirPath).c_str());
        }

        if (result) {
            std::cout << "目录已删除: " << dirPath << "\n\n";
        }
        else {
            DWORD err = GetLastError();
            if (err == ERROR_FILE_NOT_FOUND || err == ERROR_PATH_NOT_FOUND) {
                std::cout << "错误: 目录不存在\n\n";
            }
            else if (err == ERROR_DIR_NOT_EMPTY) {
                std::cout << "错误: 目录非空，请使用 /s 参数\n\n";
            }
            else {
                std::cout << "删除失败，错误码: " << err << "\n\n";
            }
        }
        return true;
    }

    if (_strnicmp(cmdTrimmed.c_str(), "type", 4) == 0) {
        std::string args = cmdTrimmed.size() > 4 ? cmdTrimmed.substr(4) : "";
        size_t s = args.find_first_not_of(" \t");
        if (s != std::string::npos) args = args.substr(s);

        if (args.size() >= 2 && args.front() == '"' && args.back() == '"') {
            args = args.substr(1, args.size() - 2);
        }

        if (args.empty()) {
            std::cout << "用法: type <文件名>\n";
            std::cout << "示例: type readme.txt\n";
            std::cout << "      type \"C:\\my file.txt\"\n\n";
            return true;
        }

        std::string filePath = ExpandEnvironmentVars(args);
        std::wstring wFilePath = U82W_Path(filePath);

        HANDLE hFile = CreateFileW(wFilePath.c_str(), GENERIC_READ, FILE_SHARE_READ,
            NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        if (hFile == INVALID_HANDLE_VALUE) {
            DWORD err = GetLastError();
            if (err == ERROR_FILE_NOT_FOUND) {
                std::cout << "错误: 找不到文件 \"" << filePath << "\"\n\n";
            }
            else {
                std::cout << "无法打开文件，错误码: " << err << "\n\n";
            }
            return true;
        }

        DWORD fileSize = GetFileSize(hFile, NULL);

        if (fileSize == 0 || fileSize > 1024 * 1024 * 10) {
            CloseHandle(hFile);
            if (fileSize == 0) {
                std::cout << "(空文件)\n\n";
            }
            else {
                std::cout << "文件过大 (" << fileSize / 1024 / 1024 << "MB)，不显示内容\n\n";
            }
            return true;
        }

        char* buffer = new char[fileSize + 1];
        DWORD bytesRead;
        BOOL readResult = ReadFile(hFile, buffer, fileSize, &bytesRead, NULL);

        if (readResult && bytesRead > 0) {
            buffer[bytesRead] = '\0';

            bool isText = true;
            for (DWORD i = 0; i < bytesRead && i < 1000; i++) {
                unsigned char c = buffer[i];
                if (c != '\r' && c != '\n' && c != '\t' && (c < 32 || c > 126)) {
                    if (c != 0) {
                        isText = false;
                        break;
                    }
                }
            }

            if (isText) {
                std::cout << buffer;
                if (bytesRead > 0 && buffer[bytesRead - 1] != '\n') {
                    std::cout << "\n";
                }
            }
            else {
                std::cout << "(二进制文件，无法显示文本内容)\n";
            }
        }

        delete[] buffer;
        CloseHandle(hFile);
        std::cout << "\n";
        return true;
    }

    if (cmdTrimmed.substr(0, 5) == "sqrt " || cmdTrimmed == "sqrt") {
        std::string param = cmdTrimmed.size() > 5 ? cmdTrimmed.substr(5) : "";
        size_t s = param.find_first_not_of(" \t");
        if (s != std::string::npos) param = param.substr(s);

        int n = 0, digits = 0;
        size_t spacePos = param.find(' ');
        if (spacePos != std::string::npos) {
            n = atoi(param.substr(0, spacePos).c_str());
            digits = atoi(param.substr(spacePos + 1).c_str());
        }
        else {
            n = atoi(param.c_str());
        }

        if (n < 0) {
            std::cout << "sqrt: 负数开平方结果为虚数\n\n";
            return true;
        }
        if (digits <= 0) {
            double result = std::sqrt((double)n);
            std::stringstream ss;
            ss << std::fixed << std::setprecision(10) << result;
            std::string res = ss.str();
            res.erase(res.find_last_not_of('0') + 1, std::string::npos);
            if (res.back() == '.') res.pop_back();
            std::cout << "√" << n << " = " << res << "\n\n";
        }
        else {
            std::cout << "√" << n << " = " << sqrt_precision(n, digits) << "\n\n";
        }
        return true;
    }

    if (cmdTrimmed.substr(0, 5) == "cbrt " || cmdTrimmed == "cbrt") {
        std::string param = cmdTrimmed.size() > 5 ? cmdTrimmed.substr(5) : "";
        size_t s = param.find_first_not_of(" \t");
        if (s != std::string::npos) param = param.substr(s);

        int n = 0, digits = 0;
        size_t spacePos = param.find(' ');
        if (spacePos != std::string::npos) {
            n = atoi(param.substr(0, spacePos).c_str());
            digits = atoi(param.substr(spacePos + 1).c_str());
        }
        else {
            n = atoi(param.c_str());
        }

        if (digits <= 0) {
            double result = std::cbrt((double)n);
            std::stringstream ss;
            ss << std::fixed << std::setprecision(10) << result;
            std::string res = ss.str();
            res.erase(res.find_last_not_of('0') + 1, std::string::npos);
            if (res.back() == '.') res.pop_back();
            std::cout << "3√" << n << " = " << res << "\n\n";
        }
        else {
            std::cout << "3√" << n << " = " << cbrt_precision(n, digits) << "\n\n";
        }
        return true;
    }

    if (cmdTrimmed.find("sin ") == 0 || cmdTrimmed.find("cos ") == 0 ||
        cmdTrimmed.find("tan ") == 0 || cmdTrimmed.find("asin ") == 0 ||
        cmdTrimmed.find("acos ") == 0 || cmdTrimmed.find("atan ") == 0 ||
        cmdTrimmed.find("exp ") == 0 || cmdTrimmed.find("log ") == 0 ||
        cmdTrimmed.find("log10 ") == 0 || cmdTrimmed.find("log2 ") == 0 ||
        cmdTrimmed.find("abs ") == 0 || cmdTrimmed.find("fmod ") == 0 ||
        cmdTrimmed.find("ceil ") == 0 || cmdTrimmed.find("floor ") == 0 ||
        cmdTrimmed.find("round ") == 0) {

        std::string result = ProcessMathCommand(cmdTrimmed);
        std::cout << result << "\n\n";
        return true;
    }

    if (cmdTrimmed.substr(0, 4) == "gen2") {
        std::string numStr = cmdTrimmed.substr(4);
        size_t s = numStr.find_first_not_of(" \t");
        if (s != std::string::npos) numStr = numStr.substr(s);
        else numStr.clear();

        if (numStr.empty()) {
            std::cout << "错误: 请输入数字\n\n";
            return true;
        }

        try {
            size_t slashPos = numStr.find('/');
            if (slashPos != std::string::npos) {
                std::string numPart = numStr.substr(0, slashPos);
                std::string denPart = numStr.substr(slashPos + 1);
                double numerator = std::stod(numPart);
                double denominator = std::stod(denPart);
                if (denominator == 0) {
                    std::cout << "错误: 分母不能为0\n\n";
                }
                else {
                    double value = numerator / denominator;
                    std::cout << "√(" << numStr << ") = " << CalculateSquareRoot(value) << "\n\n";
                }
            }
            else {
                double num = std::stod(numStr);
                std::cout << "√" << numStr << " = " << CalculateSquareRoot(num) << "\n\n";
            }
        }
        catch (const std::exception& e) {
            std::cout << "错误: 无效的数字格式\n\n";
        }
        return true;
    }

    if (cmdTrimmed.substr(0, 4) == "gen3") {
        std::string numStr = cmdTrimmed.substr(4);
        size_t s = numStr.find_first_not_of(" \t");
        if (s != std::string::npos) numStr = numStr.substr(s);
        else numStr.clear();

        if (numStr.empty()) {
            std::cout << "错误: 请输入数字\n\n";
            return true;
        }

        try {
            size_t slashPos = numStr.find('/');
            if (slashPos != std::string::npos) {
                std::string numPart = numStr.substr(0, slashPos);
                std::string denPart = numStr.substr(slashPos + 1);
                double numerator = std::stod(numPart);
                double denominator = std::stod(denPart);
                if (denominator == 0) {
                    std::cout << "错误: 分母不能为0\n\n";
                }
                else {
                    double value = numerator / denominator;
                    std::cout << "3√(" << numStr << ") = " << CalculateCubeRoot(value) << "\n\n";
                }
            }
            else {
                double num = std::stod(numStr);
                std::cout << "3√" << numStr << " = " << CalculateCubeRoot(num) << "\n\n";
            }
        }
        catch (const std::exception& e) {
            std::cout << "错误: 无效的数字格式\n\n";
        }
        return true;
    }

    if (cmdTrimmed.substr(0, 6) == "solvei" || cmdTrimmed.substr(0, 6) == "SOLVEI") {
        std::string ineq = cmdTrimmed.size() > 6 ? cmdTrimmed.substr(6) : "";
        size_t s = ineq.find_first_not_of(" \t");
        if (s != std::string::npos) ineq = ineq.substr(s);
        else ineq.clear();

        size_t e = ineq.find_last_not_of(" \t");
        if (e != std::string::npos) ineq = ineq.substr(0, e + 1);

        if (ineq.empty()) {
            std::cout << "用法: solvei <不等式>\n";
            std::cout << "示例: solvei 2x+3>7\n";
            std::cout << "      solvei 3x-5<=10\n";
            std::cout << "      solvei x/2+1>=3\n\n";
            return true;
        }

        try {
            auto result = SolveLinearInequality(ineq);
            std::cout << "不等式: " << ineq << "\n\n";

            if (result.isSpecialCase) {
                std::cout << "解: " << result.specialMessage << "\n\n";
            }
            else {

                std::cout << "解: " << result.solution << "\n\n";
            }
        }
        catch (const std::exception& e) {
            std::cout << "错误: " << e.what() << "\n\n";
        }
        return true;
    }

    if (cmdTrimmed == "st matrix" || cmdTrimmed == "ST MATRIX" || cmdTrimmed == "st Matrix") {
        std::string param = cmdTrimmed.size() > 9 ? cmdTrimmed.substr(9) : "";
        size_t s = param.find_first_not_of(" \t");
        if (s != std::string::npos) param = param.substr(s);

        int duration = 10;
        bool colored = true;

        if (!param.empty()) {
            if (param == "-b" || param == "/b") {
                colored = false;
            }
            else {
                duration = atoi(param.c_str());
                if (duration <= 0) duration = 10;
                if (duration > 3600) duration = 3600;
            }
        }

        std::cout << "\n  启动 Matrix 特效 (按 Ctrl+C 提前退出)...\n\n";

        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(hConsole, &csbi);
        WORD originalAttrs = csbi.wAttributes;

        int cols = csbi.dwSize.X;
        int rows = csbi.dwSize.Y;

        struct ColumnState {
            int y;
            int speed;
            int length;
            bool active;
            int brightness;
        };

        std::vector<ColumnState> columns(cols);
        for (int i = 0; i < cols; i++) {
            columns[i].y = rand() % rows;
            columns[i].speed = 1 + (rand() % 3);
            columns[i].length = 3 + (rand() % 10);
            columns[i].active = (rand() % 3) != 0;
            columns[i].brightness = 0;
        }

        COORD coord = { 0, 0 };
        DWORD written;
        FillConsoleOutputCharacter(hConsole, ' ', cols * rows, coord, &written);
        FillConsoleOutputAttribute(hConsole, 0x0A, cols * rows, coord, &written);
        SetConsoleCursorPosition(hConsole, coord);

        DWORD startTick = GetTickCount();
        DWORD endTick = startTick + (duration * 1000);
        int frame = 0;

        while (GetTickCount() < endTick) {
            if (_kbhit()) {
                int key = _getch();
                if (key == 27 || key == 'q' || key == 'Q') { // ESC 或 Q
                    std::cout << "\n  [已退出 Matrix 特效]\n";
                    break;
                }
            }

            frame++;

            for (int x = 0; x < cols; x++) {
                ColumnState& col = columns[x];

                if (!col.active && (rand() % 100) < 2) {
                    col.active = true;
                    col.y = 0;
                    col.length = 3 + (rand() % 12);
                    col.speed = 1 + (rand() % 3);
                    col.brightness = 0;
                }

                if (col.active) {
                    if (frame % col.speed == 0) {
                        col.y++;

                        if (col.y - col.length > rows) {
                            col.active = false;
                            col.y = 0;
                        }
                    }

                    for (int dy = 0; dy < col.length && col.y - dy >= 0; dy++) {
                        int drawY = col.y - dy;
                        if (drawY < rows) {
                            int charCode;
                            if (dy == 0) {
                                charCode = 33 + (rand() % 94);
                                WORD attr = colored ? 0x0F : 0x07; // 亮白
                                SetConsoleTextAttribute(hConsole, attr);
                            }
                            else if (dy < 3) {

                                charCode = 33 + (rand() % 94);
                                WORD attr = colored ? 0x0A : 0x07; // 亮绿
                                SetConsoleTextAttribute(hConsole, attr);
                            }
                            else {
                                charCode = 33 + (rand() % 94);
                                int brightness = 2 + (dy * 2 / col.length);
                                WORD attr = colored ? 0x02 : 0x07;
                                if (brightness > 2) attr = colored ? 0x0A : 0x07;
                                SetConsoleTextAttribute(hConsole, attr);
                            }

                            coord.X = x;
                            coord.Y = drawY;
                            SetConsoleCursorPosition(hConsole, coord);
                            std::cout << (char)charCode;
                        }
                    }
                }
            }

            for (int x = 0; x < cols; x++) {
                ColumnState& col = columns[x];
                int clearY = col.y - col.length - 1;
                if (clearY >= 0 && clearY < rows && col.active) {
                    coord.X = x;
                    coord.Y = clearY;
                    SetConsoleCursorPosition(hConsole, coord);
                    std::cout << ' ';
                }
            }

            Sleep(30);
        }

        SetConsoleTextAttribute(hConsole, originalAttrs);
        std::cout << "\n  [Matrix 特效结束，按任意键继续...]";
        _getch();
        std::cout << "\n\n";
        return true;
    }

    if (cmdTrimmed.substr(0, 11) == "st compress" || cmdTrimmed.substr(0, 11) == "ST COMPRESS") {
        std::string param = cmdTrimmed.size() > 11 ? cmdTrimmed.substr(11) : "";
        size_t s = param.find_first_not_of(" \t");
        if (s == std::string::npos || param.empty()) {
            std::cout << "用法: st compress <文件路径> [-o 输出路径] [/u]\n";
            std::cout << "  使用 Windows makecab 真正压缩\n";
            std::cout << "  注意: 路径包含空格时请用引号括起来\n";
            std::cout << "示例:\n";
            std::cout << "  st compress data.txt                    - 压缩为 data.txt.cab\n";
            std::cout << "  st compress data.txt -o backup.cab      - 指定输出文件名\n";
            std::cout << "  st compress \"G:\\Programs\\test.exe\"     - 带空格的路径\n";
            std::cout << "  st compress \"G:\\Programs\\test.exe\" -o \"T:\\backup.cab\"\n\n";
            return true;
        }

        std::string sourceFile;
        std::string outputFile;
        bool decompress = false;
        bool hasOutput = false;

        std::vector<std::string> parts;
        std::string current;
        bool inQuote = false;

        for (char c : param) {
            if (c == '"') {
                inQuote = !inQuote;
                if (!inQuote && !current.empty()) {
                    parts.push_back(current);
                    current.clear();
                }
            }
            else if (c == ' ' && !inQuote) {
                if (!current.empty()) {
                    parts.push_back(current);
                    current.clear();
                }
            }
            else {
                current += c;
            }
        }
        if (!current.empty()) {
            parts.push_back(current);
        }

        for (size_t i = 0; i < parts.size(); i++) {
            std::string p = parts[i];
            std::string pLower = p;
            std::transform(pLower.begin(), pLower.end(), pLower.begin(), ::tolower);

            if (pLower == "-o" || pLower == "/o") {
                if (i + 1 < parts.size()) {
                    outputFile = parts[i + 1];
                    hasOutput = true;
                    i++;
                }
                else {
                    std::cout << "错误: -o 参数缺少输出文件名\n\n";
                    return true;
                }
            }
            else if (pLower == "/u" || pLower == "-u") {
                decompress = true;
            }
            else if (pLower == "/c" || pLower == "-c") {

            }
            else if (sourceFile.empty()) {
                sourceFile = p;
            }
            else {
                if (!hasOutput && outputFile.empty()) {
                    outputFile = p;
                    hasOutput = true;
                }
                else {
                    std::cout << "警告: 忽略多余参数 '" << p << "'\n";
                }
            }
        }

        if (sourceFile.empty()) {
            std::cout << "错误: 请指定要操作的文件\n\n";
            return true;
        }

        sourceFile = ExpandEnvironmentVars(sourceFile);

        DWORD attrs = GetFileAttributesA(sourceFile.c_str());
        if (attrs == INVALID_FILE_ATTRIBUTES) {
            std::cout << "错误: 文件不存在 - " << sourceFile << "\n\n";
            return true;
        }
        if (attrs & FILE_ATTRIBUTE_DIRECTORY) {
            std::cout << "错误: 不能操作目录 - " << sourceFile << "\n\n";
            return true;
        }

        if (!hasOutput || outputFile.empty()) {
            if (decompress) {
                std::string lowerSrc = sourceFile;
                std::transform(lowerSrc.begin(), lowerSrc.end(), lowerSrc.begin(), ::tolower);
                if (lowerSrc.size() > 4 && lowerSrc.substr(lowerSrc.size() - 4) == ".cab") {
                    outputFile = sourceFile.substr(0, sourceFile.size() - 4);
                }
                else {
                    outputFile = sourceFile + ".out";
                }
            }
            else {
                outputFile = sourceFile + ".cab";
            }
        }
        else {
            outputFile = ExpandEnvironmentVars(outputFile);
        }

        if (sourceFile == outputFile) {
            std::cout << "错误: 源文件和输出文件相同\n\n";
            return true;
        }

        if (decompress) {
            std::string cmdLine = "expand \"" + sourceFile + "\" \"" + outputFile + "\"";
            std::cout << "  解压中... " << sourceFile << " -> " << outputFile << "\n";

            STARTUPINFOA si = { sizeof(si) };
            PROCESS_INFORMATION pi = { 0 };
            si.dwFlags = STARTF_USESTDHANDLES;
            si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
            si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
            si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

            char* cmdBuf = _strdup(cmdLine.c_str());
            if (CreateProcessA(NULL, cmdBuf, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
                WaitForSingleObject(pi.hProcess, INFINITE);
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
                free(cmdBuf);
                std::cout << "\n  解压完成! 输出: " << outputFile << "\n\n";
            }
            else {
                free(cmdBuf);
                std::cout << "错误: 解压失败\n\n";
            }
            return true;
        }

        std::string cmdLine = "makecab \"" + sourceFile + "\" \"" + outputFile + "\"";
        std::cout << "  压缩中... " << sourceFile << " -> " << outputFile << "\n";

        STARTUPINFOA si = { sizeof(si) };
        PROCESS_INFORMATION pi = { 0 };
        si.dwFlags = STARTF_USESTDHANDLES;
        si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
        si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
        si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

        char* cmdBuf = _strdup(cmdLine.c_str());
        if (!cmdBuf) {
            std::cout << "错误: 内存分配失败\n\n";
            return true;
        }

        if (CreateProcessA(NULL, cmdBuf, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
            WaitForSingleObject(pi.hProcess, INFINITE);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            free(cmdBuf);

            HANDLE hFile = CreateFileA(outputFile.c_str(), GENERIC_READ, FILE_SHARE_READ,
                NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFile != INVALID_HANDLE_VALUE) {
                LARGE_INTEGER size;
                GetFileSizeEx(hFile, &size);
                CloseHandle(hFile);

                HANDLE hSrcFile = CreateFileA(sourceFile.c_str(), GENERIC_READ, FILE_SHARE_READ,
                    NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                LARGE_INTEGER srcSize;
                if (hSrcFile != INVALID_HANDLE_VALUE) {
                    GetFileSizeEx(hSrcFile, &srcSize);
                    CloseHandle(hSrcFile);
                    double ratio = (double)size.QuadPart / (double)srcSize.QuadPart * 100.0;
                    std::cout << "\n  压缩完成!\n";
                    std::cout << "  原始: " << std::fixed << std::setprecision(1) << srcSize.QuadPart / 1024.0 << " KB\n";
                    std::cout << "  压缩: " << std::fixed << std::setprecision(1) << size.QuadPart / 1024.0 << " KB\n";
                    std::cout << "  压缩率: " << std::fixed << std::setprecision(1) << ratio << "%\n";
                }
                else {
                    std::cout << "\n  压缩完成! 大小: " << std::fixed << std::setprecision(1) << size.QuadPart / 1024.0 << " KB\n";
                }
                std::cout << "  输出: " << outputFile << "\n\n";
            }
            else {
                std::cout << "\n  压缩完成! 输出: " << outputFile << "\n\n";
            }
        }
        else {
            free(cmdBuf);
            DWORD err = GetLastError();
            if (err == ERROR_FILE_NOT_FOUND) {
                std::cout << "错误: makecab.exe 未找到\n";
                std::cout << "请确保 Windows 系统文件完整\n\n";
            }
            else {
                std::cout << "错误: 压缩失败 (错误码: " << err << ")\n\n";
            }
        }
        return true;
    }

    if (cmdTrimmed.substr(0, 3) == "st " || cmdTrimmed.substr(0, 3) == "ST " || cmdTrimmed == "st") {
        if (cmdTrimmed.find("st help") == std::string::npos && cmdTrimmed.find("ST HELP") == std::string::npos) {
            std::cout << "st模块的命令语法不正确\n";
            std::cout << "参见ZJHCMD的st更多命令帮助\n\n";
        }

        std::cout << "\n";
        std::cout << "╔══════════════════════════════════════════════════════════════════╗\n";
        std::cout << "║                    st 命令模块化帮助中心                         ║\n";
        std::cout << "║              (Super Tool - 超级工具命令集)                       ║\n";
        std::cout << "╚══════════════════════════════════════════════════════════════════╝\n";
        std::cout << "\n";
        std::cout << "  st 是 ZJHCMD 内置的超级工具命令，格式为: st <模块> <参数>\n";
        std::cout << "  共 7 大模块，覆盖系统控制、硬件信息、网络通信、AI 等\n";
        std::cout << "\n";
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        std::cout << "  【模块一：系统控制】\n";
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        std::cout << "  st theme=0/1       切换 Windows 深色/浅色模式\n";
        std::cout << "  st wallpaper=路径  设置桌面壁纸（支持环境变量）\n";
        std::cout << "  st lock [/f]       锁定电脑（/f 强制不提示）\n";
        std::cout << "  st volume          显示当前音量和静音状态\n";
        std::cout << "  st volume 0~100    设置绝对音量\n";
        std::cout << "  st volume up [n]   增加音量（默认 10%）\n";
        std::cout << "  st volume down [n] 减少音量（默认 10%）\n";
        std::cout << "  st volume mute     静音\n";
        std::cout << "  st volume unmute   取消静音\n";
        std::cout << "  st volume toggle   切换静音状态\n";
        std::cout << "  st notify \"标题\" \"内容\" [秒]  发送 Windows 通知弹窗\n";
        std::cout << "  st toast \"标题\" \"内容\" [秒]    notify 的别名\n";
        std::cout << "  st popup \"标题\" \"内容\" [秒]    notify 的别名\n";
        std::cout << "  st speak \"文本\"     语音朗读（支持中英文）\n";
        std::cout << "  st clean [/f]       清空回收站（/f 强制不提示）\n";
        std::cout << "  st compress <文件> [-o 输出] [/u] - 压缩/解压文件 (makecab)\n";
        std::cout << "  st matrix [秒数]        - 黑客帝国数字雨特效（默认10秒）\n";
        std::cout << "\n";
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        std::cout << "  【模块二：硬件，系统信息及数学】\n";
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        std::cout << "  st battery          查看电池状态（电量/充电/剩余时间）\n";
        std::cout << "  st uptime           显示系统运行时间（精确到秒）\n";
        std::cout << "  st drives           列出所有盘符及剩余空间\n";
        std::cout << "  st size <路径> [-h] 显示文件/目录大小（-h 人类可读格式）\n";
        std::cout << "  st adapter          显示网卡详细信息（IP/网关/MAC/DHCP）\n";
        std::cout << "  st mac              显示所有网卡 MAC 地址\n";
        std::cout << "  st num /1 <表达式>   展开化简\n";
        std::cout << "  st num /2 <表达式>   因式分解\n";
        std::cout << "\n";
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        std::cout << "  【模块三：鼠标与输入】\n";
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        std::cout << "  st mouse=X Y        将鼠标移动到屏幕坐标 (X, Y)\n";
        std::cout << "\n";
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        std::cout << "  【模块四：剪贴板操作】\n";
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        std::cout << "  st clipboard             显示剪贴板内容\n";
        std::cout << "  st clipboard set \"文本\"  复制文本到剪贴板\n";
        std::cout << "  st clipboard clear       清空剪贴板\n";
        std::cout << "  st clip                  同 st clipboard（简写）\n";
        std::cout << "\n";
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        std::cout << "  【模块五：环境与路径】\n";
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        std::cout << "  st path                显示当前 PATH 环境变量\n";
        std::cout << "  st path add <目录>     添加目录到 PATH\n";
        std::cout << "  st path clear          清空 PATH\n";
        std::cout << "\n";
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        std::cout << "  【模块六：计时与提醒】\n";
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        std::cout << "  st timer <秒数>        倒计时（蜂鸣提醒，最大 3600 秒）\n";
        std::cout << "  st alarm <秒数> [消息] 设置闹钟（蜂鸣 + 通知弹窗，最大 86400 秒）\n";
        std::cout << "\n";
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        std::cout << "  【模块七：网络与在线】\n";
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        std::cout << "  st weather <城市名>    查询天气（支持英文城市名）\n";
        std::cout << "  st translate \"文本\" [目标语言]  多语言翻译\n";
        std::cout << "  st news                获取今日头条新闻\n";
        std::cout << "  st quote               随机显示名人名言（中英双语）\n";
        std::cout << "  st fortune             随机显示科技小知识\n";
        std::cout << "\n";
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        std::cout << "  【翻译支持的语言代码】\n";
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        std::cout << "  zh-CN  简体中文    zh-TW  繁体中文    en     英语\n";
        std::cout << "  ja     日语        ko     韩语        fr     法语\n";
        std::cout << "  de     德语        es     西班牙语    ru     俄语\n";
        std::cout << "  it     意大利语    pt     葡萄牙语    ar     阿拉伯语\n";
        std::cout << "\n";
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        std::cout << "  【常用示例】\n";
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        std::cout << "  st volume 50               设置音量为 50%\n";
        std::cout << "  st volume up 15            音量增加 15%\n";
        std::cout << "  st theme=0                 切换到深色模式\n";
        std::cout << "  st wallpaper=%USERPROFILE%\\Pictures\\bg.jpg  设置壁纸\n";
        std::cout << "  st notify \"完成\" \"备份成功\" 10  发送通知，显示 10 秒\n";
        std::cout << "  st speak \"你好世界\"        语音朗读\n";
        std::cout << "  st battery                 查看电池状态\n";
        std::cout << "  st size C:\\Windows -h      显示 Windows 目录大小（人类可读）\n";
        std::cout << "  st clipboard set \"Hello\"   复制 Hello 到剪贴板\n";
        std::cout << "  st weather Beijing         查询北京天气\n";
        std::cout << "  st translate \"Hello\" zh-CN  翻译英文到中文\n";
        std::cout << "  st timer 60                倒计时 60 秒\n";
        std::cout << "  st alarm 300 \"会议开始\"     5 分钟后提醒\n";
        std::cout << "  st news                    看今日头条\n";
        std::cout << "  st quote                   来一句名言\n";
        std::cout << "\n";

        return true;
    }

    for (size_t i = 0; i < files.size(); i++) {
        if (cmdTrimmed == files[i].first && files[i].second == true) {
            std::string scriptPath = GetZJHCMDConfigPath() + files[i].first + ".zjhcmd";
            std::ifstream file(scriptPath);
            if (!file.is_open()) return true;

            std::string line;
            while (std::getline(file, line)) {
                while (!line.empty() && (line.back() == '\r' || line.back() == '\n')) {
                    line.pop_back();
                }

                size_t start = line.find_first_not_of(" \t");
                if (start == std::string::npos) continue;
                line = line.substr(start);

                if (line.empty() || line.substr(0, 2) == "::") continue;

                std::string expanded = ExpandEnvironmentVars(line);

                if (HandleBuiltinCommand(expanded, false)) {
                    continue;
                }

                char* cmdBuf = _strdup(expanded.c_str());
                if (!cmdBuf) continue;

                STARTUPINFOA si = { sizeof(si) };
                PROCESS_INFORMATION pi = { 0 };
                si.dwFlags = STARTF_USESTDHANDLES;
                si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
                si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
                si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

                if (CreateProcessA(NULL, cmdBuf, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
                    WaitForSingleObject(pi.hProcess, INFINITE);
                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                }
                free(cmdBuf);
            }

            file.close();
            return true;
        }
    }

    return false;

}

bool ExecuteBatchFileDirectly(const std::string& batchPath) {
    std::ifstream file(batchPath);
    if (!file.is_open()) {
        std::cout << "无法打开批处理文件: " << batchPath << "\n";
        return false;
    }

    std::vector<std::string> lines;
    std::string line;

    while (std::getline(file, line)) {
        while (!line.empty() && (line.back() == '\r' || line.back() == '\n')) {
            line.pop_back();
        }
        lines.push_back(line);
    }
    file.close();

    for (size_t i = 0; i < lines.size(); i++) {
        std::string cmd = lines[i];

        size_t start = cmd.find_first_not_of(" \t");
        if (start == std::string::npos) continue;
        size_t end = cmd.find_last_not_of(" \t");
        cmd = cmd.substr(start, end - start + 1);

        if (cmd.empty() || cmd.substr(0, 2) == "::" || cmd.substr(0, 1) == "@") {
            if (cmd == "@echo off") {
                continue;
            }
            continue;
        }

        if (cmd.substr(0, 4) == "echo" || cmd.substr(0, 4) == "ECHO") {
            std::string msg = cmd.size() > 5 ? cmd.substr(5) : "";
            if (msg == "off" || msg == "OFF") {
                continue;
            }
            else if (msg == "on" || msg == "ON") {
                continue;
            }
            else {
                msg = ExpandEnvironmentVars(msg);
                std::cout << msg << "\n";
            }
            continue;
        }

        if (cmd == "pause" || cmd == "PAUSE") {
            std::cout << "按任意键继续...";
            _getch();
            std::cout << "\n";
            continue;
        }

        if (cmd.substr(0, 3) == "set" || cmd.substr(0, 3) == "SET") {
            if (cmd.size() > 4) {
                std::string setCmd = cmd.substr(4);
                size_t eqPos = setCmd.find('=');
                if (eqPos != std::string::npos) {
                    std::string varName = setCmd.substr(0, eqPos);
                    std::string varValue = setCmd.substr(eqPos + 1);
                    varName.erase(0, varName.find_first_not_of(" \t"));
                    varName.erase(varName.find_last_not_of(" \t") + 1);
                    varValue.erase(0, varValue.find_first_not_of(" \t"));
                    varValue.erase(varValue.find_last_not_of(" \t") + 1);
                    SetEnvironmentVariableA(varName.c_str(), varValue.c_str());
                }
            }
            continue;
        }

        if (cmd.substr(0, 2) == "cd" || cmd.substr(0, 2) == "CD") {
            std::string path = cmd.size() > 3 ? cmd.substr(3) : "";
            if (path.empty()) {
                char buf[MAX_PATH];
                GetCurrentDirectoryA(MAX_PATH, buf);
                std::cout << buf << "\n";
            }
            else {
                path = ExpandEnvironmentVars(path);
                SetCurrentDirectoryA(path.c_str());
            }
            continue;
        }

        if (cmd.substr(0, 3) == "dir" || cmd.substr(0, 3) == "DIR") {
            ListDirectory("", false);
            continue;
        }

        std::string expandedCmd = ExpandEnvironmentVars(cmd);

        char* cmdBuf = _strdup(expandedCmd.c_str());
        STARTUPINFOA si = { sizeof(si) };
        PROCESS_INFORMATION pi = { 0 };
        si.dwFlags = STARTF_USESTDHANDLES;
        si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
        si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
        si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

        BOOL ok = CreateProcessA(NULL, cmdBuf, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
        free(cmdBuf);

        if (ok) {
            WaitForSingleObject(pi.hProcess, INFINITE);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
        else {
            CommandInfo info;
            info.command = cmd;
            if (!HandleBuiltinCommand(cmd, false)) {
                if (cmd.find(")") == std::string::npos) {
                    std::cout << "无法执行: " << cmd << "\n";
                }

            }
        }
    }
    return true;
}//st notify