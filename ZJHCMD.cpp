#pragma warning(disable : 4996)

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "lz32.lib")

#include "JHCOMMAND2.h"

const int rainbownumbers[10] = { 4, 12, 6, 14, 10, 11, 9, 1, 5, 13 };

extern "C" int SafeExecuteExternalCommandOnly(const char* cmd);

int ExecuteNonInteractive(const std::string& command, bool isKeepMode, std::string& keepModeMessage, bool& shouldEnterInteractive) {
    if (HandleBuiltinCommand(command, false)) {
        if (isKeepMode) {
            shouldEnterInteractive = true;
            keepModeMessage = "\n";
        }
        return 0;
    }

    CommandInfo info = ParseCommand(command);

    if (info.hasRedirect) {
        if (ExecuteWithRedirection(command, info)) {
            if (isKeepMode) {
                shouldEnterInteractive = true;
                keepModeMessage = "\n";
            }
            return 0;
        }
    }

    std::wstring wCmd = U82W_Path(command);
    std::vector<wchar_t> cmdBuf(wCmd.begin(), wCmd.end());
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
        if (isKeepMode) {
            shouldEnterInteractive = true;
            keepModeMessage = "\n";
        }
        return 0;
    }

    if (ExecuteSmart(command, info)) {
        if (isKeepMode) {
            shouldEnterInteractive = true;
            keepModeMessage = "\n";
        }
        return 0;
    }

    std::string cmdName = command;
    size_t spacePos = cmdName.find(' ');
    if (spacePos != std::string::npos) {
        cmdName = cmdName.substr(0, spacePos);
    }

    std::cout << FormatErrorMessage(cmdName) << "\n";
    return 1;
}

void printJH() {
    std::cout << "┌─────────────────────────────────────────────────────────────┐ \n";
    std::cout << "│                                                             │ \n";
    std::cout << "│";
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 3);
    std::cout << "    ███████╗     ██╗██╗  ██╗ ██████╗███╗   ███╗██████╗       ";
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
    std::cout << "│ \n";
    std::cout << "│";
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 3);
    std::cout << "    ╚══███╔╝     ██║██║  ██║██╔════╝████╗ ████║██╔══██╗      ";
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
    std::cout << "│ \n";
    std::cout << "│      ███╔╝      ██║███████║██║     ██╔████╔██║██║  ██║      │ \n";
    std::cout << "│";
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 11);
    std::cout << "     ███╔╝  ██╗  ██║██╔══██║██║     ██║╚██╔╝██║██║  ██║      ";
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
    std::cout << "│ \n";
    std::cout << "│";
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 11);
    std::cout << "    ███████╗███████║██║  ██║╚██████╗██║ ╚═╝ ██║██████╔╝      ";
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
    std::cout << "│ \n";
    std::cout << "│";
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 3);
    std::cout << "    ╚══════╝╚══════╝╚═╝  ╚═╝ ╚═════╝╚═╝     ╚═╝╚═════╝       ";
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
    std::cout << "│ \n";
    std::cout << "│                                                             │ \n";
    std::cout << "│             欢迎使用新版赵瑨娢开发者命令提示符              │ \n";
    std::cout << "│               JH Developer Command Prompt                   │ \n";
    std::cout << "│         Copyright (c) 2026 新版 赵瑨娢\"JH\" Corporation      │ \n";
    std::cout << "│                 输入 help 获取帮助                          │ \n";
    std::cout << "│";
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 14);
    std::cout << "                海内存知己，天涯若比邻。                     ";
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
    std::cout << "│ \n";
    std::cout << "│                                                             │ \n";
    std::cout << "└─────────────────────────────────────────────────────────────┘ \n\n";

    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
    std::cout << "  声明：本产品不属于微软，不应该与微软产品混淆\n";
    std::cout << "注：X:\\盘是临时系统盘，请不要将重要数据存储到X:\\盘，否则重启后会丢失！\n\n";
}

int RunMain(int argc, char* argv[]) {
    std::ifstream file("ZJHCMD_TITLE");
    bool title = true;

    std::ios::sync_with_stdio(false);
    srand(time(NULL));
    OptInitialize();
    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

    SYSTEMTIME st;

    LoadAllEnvironmentVariables();

    bool shouldEnterInteractive = false;
    bool isKeepMode = false;
    std::string keepModeMessage;

    if (argc > 1) {
        if ((strcmp(argv[1], "/c") == 0 || strcmp(argv[1], "/k") == 0) && argc >= 3) {
            std::string command;
            for (int i = 2; i < argc; i++) {
                if (i > 2) command += " ";
                command += argv[i];
            }

            isKeepMode = (strcmp(argv[1], "/k") == 0);

            int result = ExecuteNonInteractive(command, isKeepMode, keepModeMessage, shouldEnterInteractive);

            if (result == 0) {
                if (isKeepMode) {
                }
                else {
                    return 0;
                }
            }
            else {
                if (isKeepMode) {
                    shouldEnterInteractive = true;
                    keepModeMessage = "\n";
                }
                else {
                    return 1;
                }
            }
        }
        else if (argc >= 2) {
            std::string command;

            if (argc >= 2) {
                std::string arg = argv[1];
                if (arg.size() > 7 && (arg.substr(arg.size() - 7) == ".zjhcmd" || arg.substr(arg.size() - 7) == ".ZJHCMD")) {
                    std::string batchPath = ExpandEnvironmentVars(arg);
                    DWORD attrs = GetFileAttributesA(batchPath.c_str());
                    if (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY)) {
                        ExecuteBatchLikeCMD(batchPath, {});
                        return 0;
                    }
                }
                else if (arg.size() > 4 && (arg.substr(arg.size() - 4) == ".bat" ||
                    arg.substr(arg.size() - 4) == ".cmd")) {
                    std::string batchPath = ExpandEnvironmentVars(arg);
                    ExecuteBatchLikeCMD(batchPath, {});
                    return 0;
                }
            }

            for (int i = 1; i < argc; i++) {
                if (i > 1) command += " ";
                command += argv[i];
            }

            bool dummyBool = false;
            std::string dummyMsg;
            int result = ExecuteNonInteractive(command, false, dummyMsg, dummyBool);
            return result;
        }
        else {
            std::cout << "用法: " << argv[0] << " /c <command>" << std::endl;
            std::cout << "示例: " << argv[0] << " /c dir" << std::endl;
            std::cout << "      " << argv[0] << " /c \"echo Hello World\"" << std::endl;
            return 1;
        }
    }

    if (shouldEnterInteractive || argc == 1) {
        if (!file.is_open()) {
            title = false;
        }
        else {
            std::stringstream buffer;
            buffer << file.rdbuf();
            if (isKeepMode) {

            }
            else {
                std::string content = buffer.str();
                std::cout << content;
            }
            file.close();
        }

        if (isKeepMode && !keepModeMessage.empty()) {
            std::cout << keepModeMessage << std::endl;
        }

        if (!title && !isKeepMode) {
            printJH();
        }

        _1t = GetWindowTitle();

        if (!IsRunningAsAdmin()) {
            SetConsoleTitleA(_1t.c_str());
        }
        else {
            SetConsoleTitleA(("管理员: " + _1t).c_str());
        }
        int t = 0;

        char exePath[MAX_PATH];
        GetModuleFileNameA(NULL, exePath, MAX_PATH);
        std::string fullPath = exePath;
        size_t lastSlash = fullPath.find_last_of("\\/");
        std::string fileName = fullPath.substr(lastSlash + 1);

        while (true) {
            if (!IsRunningAsAdmin()) {
                SetConsoleTitleA(_1t.c_str());
            }
            else {
                SetConsoleTitleA(("管理员: " + _1t).c_str());
            }
            if (rainbow) {
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), rainbownumbers[t]);
                t++;
                t %= 10;
            }
            GetSystemTime(&st);
            ShowPrompt();

            std::string cmd;
            std::getline(std::cin, cmd);
            if (!cmd.empty() && cmd != "exit") {
                extern std::vector<std::string> g_commandHistory;
                extern const size_t MAX_HISTORY;
                g_commandHistory.push_back(cmd);
                if (g_commandHistory.size() > MAX_HISTORY) {
                    g_commandHistory.erase(g_commandHistory.begin());
                }
            }

            if (cmd.empty()) continue;
            if (cmd == "exit") {
                std::cout << std::endl;
                return 0;
            }

            if (cmd.find("&&") != std::string::npos ||
                cmd.find("||") != std::string::npos ||
                (cmd.find('(') != std::string::npos && cmd.find(')') != std::string::npos)) {
                CommandSequence seq = ParseCommandSequence(cmd);
                if (seq.commands.size() > 1 || seq.hasParen) {
                    ExecuteSequence(seq);
                    std::cout << std::endl;
                    continue;
                }
            }

            if (cmd.find(">") != std::string::npos && (cmd.find("solvei ") != 0 || cmd.find("SOLVEI ") != 0)) {
                size_t pos = cmd.find(">");
                std::string left = cmd.substr(0, pos);
                std::string right = cmd.substr(pos + 1);

                while (!left.empty() && left.back() == ' ') left.pop_back();
                while (!right.empty() && right.front() == ' ') right.erase(0, 1);

                bool append = false;
                if (!left.empty() && left.back() == '>') {
                    append = true;
                    left.pop_back();
                    while (!left.empty() && left.back() == ' ') left.pop_back();
                }

                fflush(stdout);
                if (right == "nul" || right == "NUL") {
                    freopen("nul", "w", stdout);
                }
                else {
                    freopen(right.c_str(), append ? "a" : "w", stdout);
                }

                HandleBuiltinCommand(left, false);

                fflush(stdout);
                freopen("CON", "w", stdout);
                std::cout << std::endl;
                continue;
            }

            if (cmd.size() > 4 && (cmd.substr(cmd.size() - 4) == ".bat" ||
                cmd.substr(cmd.size() - 4) == ".cmd")) {
                std::string batchPath = ExpandEnvironmentVars(cmd);
                DWORD attrs = GetFileAttributesA(batchPath.c_str());
                if (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY)) {
                    ExecuteBatchLikeCMD(batchPath, {});
                    std::cout << std::endl;
                    continue;
                }
            }

            if (HandleBuiltinCommand(cmd, false)) {
                std::cout << std::endl;
                continue;
            }

            bool processCreated = false;

            std::string cmdName = cmd;
            std::string args;
            size_t spacePos = cmd.find(' ');
            if (spacePos != std::string::npos) {
                cmdName = cmd.substr(0, spacePos);
                args = cmd.substr(spacePos);
            }

            STARTUPINFOW si2 = { sizeof(si2) };
            PROCESS_INFORMATION pi2 = { 0 };
            si2.dwFlags = STARTF_USESTDHANDLES;
            si2.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
            si2.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
            si2.hStdError = GetStdHandle(STD_ERROR_HANDLE);

            {
                std::wstring wCmd = U82W_Path(cmd);
                std::vector<wchar_t> cmdBuf(wCmd.begin(), wCmd.end());
                cmdBuf.push_back(L'\0');

                if (CreateProcessW(NULL, cmdBuf.data(), NULL, NULL, TRUE, 0, NULL, NULL, &si2, &pi2)) {
                    WaitForSingleObject(pi2.hProcess, INFINITE);
                    CloseHandle(pi2.hProcess);
                    CloseHandle(pi2.hThread);
                    processCreated = true;
                }
            }

            if (!processCreated) {
                std::string zjhCmdDir = GetZJHCMDConfigPath();

                struct Candidate {
                    std::string path;
                    bool isScript;
                };

                std::vector<Candidate> candidates = {
                    { zjhCmdDir + cmdName + ".exe", false },
                    { zjhCmdDir + cmdName + ".bat", false },
                    { zjhCmdDir + cmdName + ".cmd", false },
                    { zjhCmdDir + cmdName + ".zjhcmd", true },
                };

                for (const auto& cand : candidates) {
                    DWORD attrs = GetFileAttributesA(cand.path.c_str());
                    if (attrs == INVALID_FILE_ATTRIBUTES || (attrs & FILE_ATTRIBUTE_DIRECTORY)) {
                        continue;
                    }

                    std::string cmdLine;
                    if (cand.isScript) {
                        cmdLine = "\"" + fullPath + "\" /c \"" + cand.path + "\"" + args;
                    }
                    else {
                        cmdLine = "\"" + cand.path + "\"" + args;
                    }

                    std::wstring wCmdLine = U82W_Path(cmdLine);
                    std::vector<wchar_t> cmdBuf(wCmdLine.begin(), wCmdLine.end());
                    cmdBuf.push_back(L'\0');

                    if (CreateProcessW(NULL, cmdBuf.data(), NULL, NULL, TRUE, 0, NULL, NULL, &si2, &pi2)) {
                        WaitForSingleObject(pi2.hProcess, INFINITE);
                        CloseHandle(pi2.hProcess);
                        CloseHandle(pi2.hThread);
                        processCreated = true;
                    }

                    if (processCreated) break;
                }
            }

            if (!processCreated) {
                std::cout << FormatErrorMessage(cmdName) << "\n";
            }

            std::cout << std::endl;
        }
    }

    return 0;
}

int main(int argc, char* argv[]) {
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);
    EnsureConfigDirectory();

    if (argc > 1 && strcmp(argv[1], "/c") == 0) {
        return RunMain(argc, argv);
    }

    int restartCount = 0;
    const int MAX_RESTARTS = 10;

    while (restartCount < MAX_RESTARTS) {
        int result = 0;
        bool crashed = false;

        __try {
            if (restartCount > 0) {
                std::cout << "\n[程序已自动重启 (第 " << restartCount << " 次)]\n\n";
                fflush(stdout);
                freopen("CON", "w", stdout);
                freopen("CON", "r", stdin);
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
                SetConsoleTitleA(_1t.c_str());
            }

            result = RunMain(argc, argv);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            crashed = true;
            restartCount++;
            std::cout << "\n[程序发生严重错误，正在自动恢复... (第 " << restartCount << " 次)]\n";
            fflush(stdout);
            freopen("CON", "w", stdout);
            freopen("CON", "r", stdin);
            SetLastErrorCode(1);
            Sleep(1000);
        }

        if (!crashed) {
            return result;
        }
    }

    std::cout << "\n[程序多次崩溃（超过 " << MAX_RESTARTS << " 次），已停止自动重启]\n";
    return 1;
}
