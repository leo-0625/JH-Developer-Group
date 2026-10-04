#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <cstring>
#include <Windows.h>
#include <shlobj.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <conio.h>
#include <functional>
#include <sstream>
#include <set>
#include <strsafe.h>
#include <filesystem>
std::string ExpandEnvironmentVars(const std::string& input);

#pragma warning(disable : 4996)

extern bool g_echoState;
namespace fs = std::filesystem;
int JHcolor = 7;

std::string version = "Beta 26.1";

std::string W2U8(const std::wstring& w);
std::wstring U82W(const std::string& s);
std::string FormatNumber(unsigned long long n);
bool IsSystemDir(const std::wstring& name);
bool IsRvaValid(LPVOID pBase, PIMAGE_NT_HEADERS pNtHeaders, DWORD rva);
void* GetPtrFromRva(LPVOID pBase, PIMAGE_NT_HEADERS pNtHeaders, DWORD rva);

static const char* g_commandWhitelist[] = {
    "dir", "cd", "type", "whoami", "hostname", "ver",
    "echo", "help", "allhelp", "history", "date", "time",
    "vol", "tree", "find", "where", "more", "dumpbin",
    "hex", "dec", "bin", "hash",
    "sin", "cos", "tan", "asin", "acos", "atan",
    "log", "log10", "log2", "exp", "abs", "ceil",
    "floor", "round", "sqrt", "cbrt", "fmod",
    "gen2", "gen3", "solve1", "solve2", "solvei",
    "solve2var", "solve3var", "st num", "_calc",
    "st battery", "st uptime", "st drives", "st size",
    "st adapter", "st mac", "st path", "st clipboard",
    "st quote", "st fortune", "st weather", "st translate",
    "st news", "st timer", "st alarm", "st matrix",
    "ipconfig", "tasklist", "ping", "tracert", "nslookup",
    "netstat", "systeminfo", "ver", "set",
    "JHversion", "JHname", "zjhhelp",

    nullptr
};

bool GetDataDirectory(PIMAGE_NT_HEADERS pNtHeaders, int index,
    DWORD& outRVA, DWORD& outSize) {
    outRVA = 0;
    outSize = 0;
    if (!pNtHeaders) return false;
    if (index < 0 || index >= 16) return false;

    WORD magic = pNtHeaders->OptionalHeader.Magic;

    if (magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
        PIMAGE_OPTIONAL_HEADER32 pOpt =
            (PIMAGE_OPTIONAL_HEADER32)&pNtHeaders->OptionalHeader;
        if (pOpt->NumberOfRvaAndSizes > 16) return false;
        if ((DWORD)index >= pOpt->NumberOfRvaAndSizes) return false;
        outRVA = pOpt->DataDirectory[index].VirtualAddress;
        outSize = pOpt->DataDirectory[index].Size;
        return true;
    }
    else if (magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
        PIMAGE_OPTIONAL_HEADER64 pOpt =
            (PIMAGE_OPTIONAL_HEADER64)&pNtHeaders->OptionalHeader;
        if (pOpt->NumberOfRvaAndSizes > 16) return false;
        if ((DWORD)index >= pOpt->NumberOfRvaAndSizes) return false;
        outRVA = pOpt->DataDirectory[index].VirtualAddress;
        outSize = pOpt->DataDirectory[index].Size;
        return true;
    }

    return false;
}

bool SafeReadString(LPVOID pBase, PIMAGE_NT_HEADERS pNtHeaders,
    DWORD rva, std::string& out, size_t maxLen = 512) {
    out.clear();
    if (!IsRvaValid(pBase, pNtHeaders, rva)) return false;

    char* p = (char*)GetPtrFromRva(pBase, pNtHeaders, rva);
    if (!p) return false;

    size_t len = 0;
    while (len < maxLen && p[len] != '\0') len++;
    if (len == 0 || len >= maxLen) return false;

    out.assign(p, len);
    return true;
}

static std::wstring U82W_Path(const std::string& s) {
    if (s.empty()) return L"";
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, NULL, 0);
    if (len <= 0) return L"";
    std::wstring out(len - 1, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &out[0], len);
    return out;
}

static const char* helpcommand[] = {
    "",
    "================================================================================",
    "                    ZJHCMD 全部命令列表",
    "================================================================================",
    "",
    "【基础命令】",
    "  cd <路径>                    - 切换当前目录（无参数时显示当前目录）",
    "  dir [路径] [/s] [/b]         - 列出目录内容（/s 递归，/b 裸输出）",
    "  echo <文本>                  - 输出文本，支持 %VAR% 环境变量",
    "  cls / clear                  - 清屏",
    "  exit                         - 退出程序",
    "  help / ?                     - 显示系统 CMD 命令帮助",
    "  allhelp                      - 显示所有 ZJHCMD 指令",
    "  history                      - 显示命令历史",
    "  ver                          - 显示版本信息",
    "  date                         - 显示当前日期",
    "  time                         - 显示当前时间",
    "  whoami                       - 显示当前用户名",
    "  hostname                     - 显示计算机名",
    "  vol [驱动器:]                - 显示磁盘卷标和序列号",
    "  chcp [代码页]                - 显示/设置代码页（65001=UTF-8）",
    "  color <颜色值>               - 设置控制台颜色（0-15）",
    "  title <标题>                 - 设置窗口标题",
    "  prompt <提示符>              - 设置命令提示符",
    "  sleep <秒数>                 - 延时（最大 3600 秒）",
    "  beep [频率] [时长]           - 蜂鸣（默认 800Hz 500ms）",
    "  home [/e]                    - 切换到用户目录（/e 在资源管理器中打开）",
    "  ~ / cd ~                     - 快速切换到用户目录",
    "",
    "【文件操作】",
    "  type <文件>                  - 查看文件内容（支持 10MB 以内）",
    "  copy <源> <目标>             - 复制文件",
    "  move <源> <目标>             - 移动/重命名文件",
    "  del / erase <文件> [/q]      - 删除文件（/q 静默模式）",
    "  mkdir / md <目录>            - 创建目录",
    "  rmdir / rd <目录> [/s]       - 删除目录（/s 递归删除）",
    "  ren / rename <旧> <新>       - 重命名（支持通配符）",
    "  find \"字符串\" [文件] [/i] [/v] [/c] [/n] - 查找字符串（/i 忽略大小写，/v 反向，/c 计数，/n 行号）",
    "  tree [路径] [/f]             - 显示目录树（/f 显示文件）",
    "  attrib [+R|-R] [文件]        - 修改文件属性（R 只读，H 隐藏，S 系统，A 存档）",
    "  mklink [/d] <链接> <目标>    - 创建符号链接（/d 目录链接）",
    "  xcopy <源> <目标> [/S] [/E] [/Y] [/D] [/C] [/H] [/R] - 高级复制",
    "  robocopy <源> <目标> [/S] [/E] [/MIR] [/R:n] [/W:n] - 健壮的文件复制",
    "  replace <源> <目标> [/A] [/R] [/S] [/U] - 替换文件",
    "  more <文件>                  - 分页显示文件",
    "",
    "【数学计算】",
    "  _calc <表达式>               - 高精度计算（支持 + - * / ^ 括号）",
    "  solve1 <方程>                - 一元一次方程（如 2x+3=7）",
    "  solve2 <方程>                - 一元二次方程（如 x^2-4=0）",
    "  solve2var                    - 二元一次方程组（交互式）",
    "  solve3var                    - 三元一次方程组（交互式）",
    "  solvei <不等式>              - 一元一次不等式（如 2x+3>7）",
    "  sin <角度>                   - 正弦（同时显示弧度和角度）",
    "  cos <角度>                   - 余弦",
    "  tan <角度>                   - 正切",
    "  asin <值>                    - 反正弦（-1 到 1）",
    "  acos <值>                    - 反余弦（-1 到 1）",
    "  atan <值>                    - 反正切",
    "  log <数字>                   - 自然对数（ln，需 > 0）",
    "  log10 <数字>                 - 常用对数（需 > 0）",
    "  log2 <数字>                  - 以 2 为底的对数（需 > 0）",
    "  sqrt <数字> [小数位数]       - 平方根（如 sqrt 2 100）",
    "  cbrt <数字> [小数位数]       - 立方根（如 cbrt 2 100）",
    "  gen2 <数字>                  - 平方根（支持有理数和虚数，如 gen2 12）",
    "  gen3 <数字>                  - 立方根（支持有理数，如 gen3 27）",
    "  fmod <a> <b>                 - 浮点数取余（b 不能为 0）",
    "  abs <数字>                   - 绝对值",
    "  ceil <数字>                  - 向上取整",
    "  floor <数字>                 - 向下取整",
    "  round <数字>                 - 四舍五入",
    "  exp <数字>                   - e 的幂次",
    "",
    "【进制与编码】",
    "  hex <数字>                   - 十进制转十六进制（如 hex 255 → 0xFF）",
    "  dec <十六进制>               - 十六进制转十进制（如 dec 0xFF → 255）",
    "  bin <数字>                   - 十进制转二进制（如 bin 255 → 0b11111111）",
    "  hash <文件> [算法]           - 计算哈希（算法：MD5/SHA1/SHA256/SHA384/SHA512）",
    "",
    "【系统控制 st 模块】",
    "  st theme=0/1                 - 切换主题（0=深色，1=浅色）",
    "  st wallpaper=<路径>          - 设置桌面壁纸（支持环境变量）",
    "  st lock [/f]                 - 锁定电脑（/f 强制不提示）",
    "  st mouse=X Y                 - 移动鼠标到屏幕坐标 (X, Y)",
    "  st volume                    - 显示当前音量和静音状态",
    "  st volume <0-100>            - 设置绝对音量（如 st volume 50）",
    "  st volume up [值]            - 增加音量（默认 10，如 st volume up 15）",
    "  st volume down [值]          - 减少音量（默认 10）",
    "  st volume mute               - 静音",
    "  st volume unmute             - 取消静音",
    "  st volume toggle             - 切换静音状态",
    "  st notify \"标题\" \"内容\" [秒] - 通知弹窗（默认 5 秒，最大 30 秒）",
    "  st toast \"标题\" \"内容\" [秒]  - notify 的别名",
    "  st popup \"标题\" \"内容\" [秒]  - notify 的别名",
    "  st speak \"文本\"              - 语音朗读（支持中英文）",
    "  st battery                   - 电池状态（电量/充电/剩余时间）",
    "  st uptime                    - 系统运行时间（精确到秒）",
    "  st drives                    - 盘符列表（含剩余空间）",
    "  st size <路径> [-h]          - 文件/目录大小（-h 人类可读格式）",
    "  st adapter                   - 网卡信息（IP/网关/MAC/DHCP）",
    "  st mac                       - 所有网卡 MAC 地址",
    "  st clean [/f]                - 清空回收站（/f 强制不提示）",
    "  st path                      - 显示当前 PATH",
    "  st path add <目录>           - 添加目录到 PATH",
    "  st path clear                - 清空 PATH",
    "  st clipboard                 - 显示剪贴板内容",
    "  st clipboard set \"文本\"      - 复制文本到剪贴板",
    "  st clipboard clear           - 清空剪贴板",
    "  st timer <秒数>              - 倒计时（最大 3600 秒）",
    "  st alarm <秒数> [消息]       - 闹钟（最大 86400 秒）",
    "  st matrix [秒数]             - 数字雨特效（默认 10 秒）",
    "  st compress <文件> [-o 输出] [/u] - 压缩/解压（/u 解压）",
    "",
    "【网络与在线】",
    "  st weather <城市>            - 天气查询（如 st weather Beijing）",
    "  st translate \"文本\" [语言]   - 多语言翻译（zh-CN/en/ja/ko/fr/de/es/ru/it/pt/ar）",
    "  st news                      - 今日头条新闻",
    "  st quote                     - 随机名人名言",
    "  st fortune                   - 随机科技小知识",
    "  _Internet_                   - 打开 Edge 浏览器",
    "",
    "【进程与系统】",
    "  tasklist                     - 进程列表（含内存占用）",
    "  taskkill /PID <PID>          - 终止指定 PID 的进程",
    "  taskkill /IM <进程名.exe>    - 终止指定名称的进程",
    "  taskkill /F /PID <PID>       - 强制终止",
    "  taskkill /F /IM <进程名>     - 强制终止",
    "  ipconfig                     - IP 配置信息",
    "  start <程序>                 - 启动程序",
    "  start /b <程序>              - 后台启动",
    "  start /wait <程序>           - 等待程序结束",
    "  start /min <程序>            - 最小化启动",
    "  start /max <程序>            - 最大化启动",
    "",
    "【脚本与流程】",
    "  set                          - 显示所有环境变量",
    "  set <变量>=<值>              - 设置环境变量",
    "  set /a <表达式>              - 算术计算",
    "  if <条件> <命令>             - 条件判断（exist/errorlevel/defined/==/not）",
    "  for %%i in (<集合>) do <命令> - 循环（支持 /f /l /r）",
    "  goto <标签>                  - 跳转（标签定义: :label）",
    "  call <批处理>                - 调用批处理",
    "  call :<标签>                 - 调用标签",
    "  shift                        - 参数左移",
    "  pause                        - 暂停",
    "  setlocal [enabledelayedexpansion] - 环境变量本地化",
    "  endlocal                     - 结束环境变量本地化",
    "  pushd <目录>                 - 保存当前目录并切换",
    "  popd                         - 恢复保存的目录",
    "  verify [on|off]              - 验证开关",
    "  break [on|off]               - 中断开关",
    "",
    "【文件关联】",
    "  assoc                        - 显示所有文件关联",
    "  assoc <扩展名>=<类型>        - 设置文件关联",
    "  ftype                        - 显示所有文件类型",
    "  ftype <类型>=<命令>          - 设置文件类型",
    "  path                         - 显示 PATH 环境变量",
    "  path <目录>                  - 添加目录到 PATH",
    "  path =<新PATH>               - 设置 PATH",
    "  dpath                        - 显示 DPATH",
    "  dpath <目录>                 - 设置 DPATH",
    "  dpath ;                      - 清除 DPATH",
    "",
    "【PE 分析】",
    "  dumpbin /dependents <文件>   - 显示依赖的 DLL",
    "  dumpbin /exports <文件>      - 显示导出表",
    "  dumpbin /imports <文件>      - 显示导入表",
    "  dumpbin /sections <文件>     - 显示节区信息",
    "  dumpbin /headers <文件>      - 显示 PE 头信息",
    "  dumpbin /resources <文件>    - 显示资源目录",
    "  dumpbin /all <文件>          - 显示全部信息",
    "",
    "【品牌与彩蛋】",
    "  JHname                       - 显示 JH Logo",
    "  friend                       - 友情日记",
    "  rainbow [-on|-off]           - 彩虹模式（-on 开启，-off 关闭）",
    "",
    "【系统操作】",
    "  jh_shutdown [_延时_] <0-5> [/q] - 关机/重启（0=关机 1=重启 2=注销 3=强制关机 4=强制重启 5=强制注销，/q 静默）",
    "  _admin_                      - 以管理员身份重启",
    "  _solveproblems_              - 系统优化（内存整理 + 僵尸进程清理）",
    "",
    "【包管理】",
    "  apt install \"文件.zip\"       - 安装 JH 认证模块（v46）",
    "  apt remove <模块名>          - 卸载模块（v48 计划中）",
    "  apt list                     - 列出已安装模块（v48 计划中）",
    "",
    "【自定义命令】",
    "  在 .zjhcmd\\ 目录下的 .zjhcmd 文件，文件名即命令名",
    "  输入文件名即可执行",
    "",
    "================================================================================",
    "  输入 'zjhhelp <命令名>' 查看详细帮助",
    "  输入 'help' 查看系统命令帮助",
    "================================================================================",
    "",
    nullptr
};

void ShowExports(LPVOID pBase, PIMAGE_NT_HEADERS pNtHeaders);
void ShowSections(PIMAGE_NT_HEADERS pNtHeaders);
void ShowResources(LPVOID pBase, PIMAGE_NT_HEADERS pNtHeaders);
void ShowImports(LPVOID pBase, PIMAGE_NT_HEADERS pNtHeaders);
void ShowHeaders(PIMAGE_NT_HEADERS pNtHeaders);

static void ParsePEFileSafe(LPVOID pBase, PIMAGE_NT_HEADERS pNtHeaders,
    const char* option) {
   
    __try {
        bool all = (option != nullptr && strcmp(option, "/all") == 0);

        if (strcmp(option, "/dependents") == 0 || all) {
            DWORD importRVA = 0, importSize = 0;
            if (!GetDataDirectory(pNtHeaders, IMAGE_DIRECTORY_ENTRY_IMPORT,
                importRVA, importSize) || importRVA == 0) {
                printf("  无依赖的 DLL\n");
            }
            else if (!IsRvaValid(pBase, pNtHeaders, importRVA)) {
                printf("  警告：导入表 RVA 无效\n");
            }
            else {
                PIMAGE_IMPORT_DESCRIPTOR pImportDesc =
                    (PIMAGE_IMPORT_DESCRIPTOR)GetPtrFromRva(pBase, pNtHeaders, importRVA);
                if (pImportDesc) {
                    printf("\n  依赖的 DLL:\n");
                    printf("  ----------------------------------------\n");
                    int dllCount = 0;
                    const int MAX_DLLS = 4096;
                    while (pImportDesc->Name != 0 && dllCount < MAX_DLLS) {
                        
                        if (IsRvaValid(pBase, pNtHeaders, pImportDesc->Name)) {
                            char* name = (char*)GetPtrFromRva(pBase, pNtHeaders, pImportDesc->Name);
                            if (name) {
                                char buf[512];
                                int len = 0;
                                while (len < 511 && name[len] != '\0') {
                                    buf[len] = name[len];
                                    len++;
                                }
                                buf[len] = '\0';
                                printf("    %s\n", buf);
                                dllCount++;
                            }
                        }
                        pImportDesc++;
                    }
                    printf("  ----------------------------------------\n");
                    printf("  共 %d 个依赖\n", dllCount);
                }
            }
        }

        if (strcmp(option, "/exports") == 0 || all) {
            ShowExports(pBase, pNtHeaders);
        }
        if (strcmp(option, "/imports") == 0 || all) {
            ShowImports(pBase, pNtHeaders);
        }
        if (strcmp(option, "/sections") == 0 || all) {
            ShowSections(pNtHeaders);
        }
        if (strcmp(option, "/headers") == 0 || all) {
            ShowHeaders(pNtHeaders);
        }
        if (strcmp(option, "/resources") == 0 || all) {
            ShowResources(pBase, pNtHeaders);
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        DWORD code = GetExceptionCode();
        printf("\n");
        printf("  ⚠ 解析过程中发生异常\n");
        printf("  ----------------------------------------\n");
        printf("  异常代码: 0x%08X\n", code);
        switch (code) {
        case EXCEPTION_ACCESS_VIOLATION:
            printf("  类型: 访问冲突（内存越界或空指针）\n");
            break;
        case EXCEPTION_INT_DIVIDE_BY_ZERO:
            printf("  类型: 整数除以零\n");
            break;
        case EXCEPTION_STACK_OVERFLOW:
            printf("  类型: 栈溢出\n");
            break;
        default:
            printf("  类型: 未知异常\n");
            break;
        }
        printf("  可能原因：文件损坏、加壳、格式异常或恶意构造\n");
        printf("  已安全终止本次解析，ZJHCMD 将继续运行\n");
        printf("  ----------------------------------------\n");
    }
}

std::vector<std::pair<std::string, bool>> GetZjhCmdFiles() {
    std::vector<std::pair<std::string, bool>> result;

    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    fs::path exeDir = fs::path(exePath).parent_path();

    fs::path zjhCmdDir = exeDir / ".zjhcmd";
    if (!fs::exists(zjhCmdDir) || !fs::is_directory(zjhCmdDir)) {
        return result;
    }

    for (const auto& entry : fs::directory_iterator(zjhCmdDir)) {
        if (entry.is_regular_file()) {
            fs::path filePath = entry.path();
            std::string filename = filePath.filename().string();
            std::string stem = filePath.stem().string();
            std::string extension = filePath.extension().string();

            bool isZjhCmd = false;
            std::string extLower = extension;
            std::transform(extLower.begin(), extLower.end(), extLower.begin(), ::tolower);
            if (extLower == ".zjhcmd") {
                isZjhCmd = true;
            }

            result.push_back(std::make_pair(stem, isZjhCmd));
        }
    }

    std::sort(result.begin(), result.end(),
        [](const std::pair<std::string, bool>& a, const std::pair<std::string, bool>& b) {
            std::string aLower = a.first;
            std::string bLower = b.first;
            std::transform(aLower.begin(), aLower.end(), aLower.begin(), ::tolower);
            std::transform(bLower.begin(), bLower.end(), bLower.begin(), ::tolower);
            return aLower < bLower;
        });

    return result;
}

bool IsCommandBlocked(const std::string& line) {
    std::string lowerLine = line;
    std::transform(lowerLine.begin(), lowerLine.end(), lowerLine.begin(), ::tolower);

    if (lowerLine.substr(0, 4) == "call" && lowerLine[4] == ' ') {
        std::cout << "错误：ZJHCMD自定义命令中禁止使用 'call' 指令。" << std::endl;
        return true;
    }

    if (lowerLine.find("%0") != std::string::npos) {
        std::cout << "错误：检测到高危递归指令 '%0'，已拦截。" << std::endl;
        return true;
    }

    return false;
}

void MorePager(const std::string& fileName) {
    std::wstring wFileName;
    {
        int len = MultiByteToWideChar(CP_UTF8, 0, fileName.c_str(), -1, NULL, 0);
        if (len <= 0) {
            std::cout << "无法打开文件: " << fileName << "\n";
            return;
        }
        wFileName.resize(len - 1);
        MultiByteToWideChar(CP_UTF8, 0, fileName.c_str(), -1, &wFileName[0], len);
    }

    std::ifstream file(wFileName.c_str());
    if (!file.is_open()) {
        std::wcout << L"无法打开文件: " << wFileName << L"\n";
        return;
    }

    std::string line;
    int lineCount = 0;
    const int LINES_PER_PAGE = 24;

    while (std::getline(file, line)) {
        std::cout << line << "\n";
        lineCount++;

        if (lineCount >= LINES_PER_PAGE) {
            std::cout << "-- 按任意键继续，按 Q 退出 --";
            int ch = _getch();
            if (ch == 'q' || ch == 'Q') {
                std::cout << "\n";
                break;
            }
            std::cout << "\n";
            lineCount = 0;
        }
    }
    file.close();
}

std::string ExtractReplaceSpec(const std::string& varValue, const std::string& replaceSpec) {
    if (replaceSpec.empty()) return varValue;

    size_t equalPos = replaceSpec.find('=');
    if (equalPos == std::string::npos) return varValue;

    std::string oldStr = replaceSpec.substr(0, equalPos);
    std::string newStr = replaceSpec.substr(equalPos + 1);

    if (oldStr.empty()) return varValue;

    std::string result;
    size_t pos = 0;
    size_t lastPos = 0;

    while ((pos = varValue.find(oldStr, lastPos)) != std::string::npos) {
        result += varValue.substr(lastPos, pos - lastPos);
        result += newStr;
        lastPos = pos + oldStr.length();
    }
    result += varValue.substr(lastPos);

    return result;
}

std::string ExtractSubstring(const std::string& varValue, const std::string& subSpec) {
    if (subSpec.empty()) return varValue;

    std::string spec = subSpec;
    if (spec.size() >= 2 && spec[0] == ':' && spec[1] == '~') {
        spec = spec.substr(2);
    }
    else if (spec.size() >= 1 && spec[0] == '~') {
        spec = spec.substr(1);
    }
    else {
        return varValue;
    }

    if (spec.empty()) return varValue;

    int start = 0;
    int length = -1;

    size_t commaPos = spec.find(',');
    if (commaPos == std::string::npos) {
        start = std::stoi(spec);
    }
    else {
        std::string startStr = spec.substr(0, commaPos);
        std::string lenStr = spec.substr(commaPos + 1);
        start = std::stoi(startStr);
        if (!lenStr.empty()) {
            length = std::stoi(lenStr);
        }
    }

    int len = (int)varValue.length();

    if (start < 0) {
        start = len + start;
    }
    if (start < 0) start = 0;
    if (start > len) return "";

    int actualLength;
    if (length < 0) {
        if (length == -1) {
            actualLength = len - start;
        }
        else {
            actualLength = len + length - start;
            if (actualLength < 0) actualLength = 0;
        }
    }
    else if (length == 0) {
        return "";
    }
    else {
        actualLength = length;
        if (start + actualLength > len) {
            actualLength = len - start;
        }
    }

    if (actualLength <= 0) return "";

    return varValue.substr(start, actualLength);
}

std::string ExpandEnvironmentVarsEnhanced(const std::string& input) {
    static int depth = 0;
    const int MAX_DEPTH = 10;
    const size_t MAX_EXPAND_SIZE = 65536;

    if (++depth > MAX_DEPTH) {
        depth--;
        return input;
    }

    std::string result = input;
    size_t pos = 0;
    int iterations = 0;
    const int MAX_ITERATIONS = 50;

    while (iterations++ < MAX_ITERATIONS && (pos = result.find('%', pos)) != std::string::npos) {
        size_t endPos = result.find('%', pos + 1);
        if (endPos == std::string::npos) {
            pos++;
            continue;
        }

        if (result.size() > MAX_EXPAND_SIZE) {
            depth--;
            return result.substr(0, MAX_EXPAND_SIZE);
        }

        std::string varName = result.substr(pos + 1, endPos - pos - 1);
        if (varName.empty()) {
            result.replace(pos, 2, "%");
            pos++;
            continue;
        }

        char buffer[4096] = { 0 };
        DWORD len = GetEnvironmentVariableA(varName.c_str(), buffer, sizeof(buffer) - 1);
        std::string varValue = (len > 0) ? buffer : "";

        if (varValue.find('%') != std::string::npos) {
            char temp[32767] = { 0 };
            ExpandEnvironmentStringsA(varValue.c_str(), temp, sizeof(temp) - 1);
            varValue = temp;
        }

        size_t newSize = result.size() - (endPos - pos + 1) + varValue.size();
        if (newSize > MAX_EXPAND_SIZE) {
            varValue = varValue.substr(0, MAX_EXPAND_SIZE - result.size() + (endPos - pos + 1) - 1);
        }

        result.replace(pos, endPos - pos + 1, varValue);
        pos += varValue.length();

        if (pos == 0) break;
        if (pos >= result.size()) break;
    }

    depth--;
    return result;
}

std::string ExpandEnvironmentVarsBasic(const std::string& input) {
    char buffer[32767];
    DWORD result = ExpandEnvironmentStringsA(input.c_str(), buffer, sizeof(buffer));
    if (result > 0 && result <= sizeof(buffer)) {
        return std::string(buffer);
    }
    return input;
}

struct BatchContext {
    std::vector<std::string> lines;
    size_t currentLine;
    bool echoOn;
    bool enabledelayedexpansion;
    std::map<std::string, std::string> localVars;
    std::map<std::string, std::string> savedEnv;
    std::vector<size_t> callStack;

    BatchContext() : currentLine(0), echoOn(true), enabledelayedexpansion(false) {}
};

void SetLastErrorCode(DWORD code);
DWORD GetLastErrorCode();
bool IsLastCommandSuccessful();

struct CommandInfo {
    std::string command;
    std::string args;
    bool hasPipe;
    bool hasRedirect;
    std::string pipeCommand;
    std::string redirectFile;
    char redirectType;
};

bool HandleSetCommand(const std::string& cmd, bool silentMode);
bool HandleIfCommand(const std::string& cmd, BatchContext& ctx);
bool HandleForCommand(const std::string& cmd, BatchContext& ctx);
bool ExecuteBatchLine(const std::string& line, BatchContext& ctx);
long long EvaluateArithmeticExpression(const std::string& expr);

CommandInfo ParseCommand(const std::string& cmd);
extern DWORD g_lastErrorCode;

DWORD RvaToFileOffset(PIMAGE_NT_HEADERS pNtHeaders, DWORD rva) {
    PIMAGE_SECTION_HEADER pSectionHeader = IMAGE_FIRST_SECTION(pNtHeaders);
    WORD numSections = pNtHeaders->FileHeader.NumberOfSections;

    for (WORD i = 0; i < numSections; i++) {
        DWORD sectionStart = pSectionHeader[i].VirtualAddress;
        DWORD sectionEnd = sectionStart + pSectionHeader[i].Misc.VirtualSize;

        if (rva >= sectionStart && rva < sectionEnd) {
            return (rva - sectionStart) + pSectionHeader[i].PointerToRawData;
        }
    }
    return 0;
}

void* GetPtrFromRva(LPVOID pBase, PIMAGE_NT_HEADERS pNtHeaders, DWORD rva) {
    DWORD offset = RvaToFileOffset(pNtHeaders, rva);
    if (offset == 0) return NULL;
    return (BYTE*)pBase + offset;
}

bool HandleBuiltinCommand(const std::string& cmd, bool silentMode);

static BatchContext g_batchCtx;
static std::stack<BatchContext> g_batchStack;

bool MatchPattern(const std::string& filename, const std::string& pattern);

volatile BOOL g_dirInterrupted = FALSE;

long long EvaluateArithmeticExpression(const std::string& expr);

BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType) {
    if (dwCtrlType == CTRL_C_EVENT || dwCtrlType == CTRL_BREAK_EVENT) {
        g_dirInterrupted = TRUE;
        return TRUE;
    }
    return FALSE;
}

struct CommandSequence {
    std::vector<std::string> commands;
    std::vector<char> operators;
    bool hasParen;
    std::string groupedCommand;
};

CommandSequence ParseCommandSequence(const std::string& cmdLine);
bool ExecuteSequence(const CommandSequence& seq);

void chunWclear() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE) return;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    DWORD dwConsoleSize = csbi.dwSize.X * csbi.dwSize.Y;
    COORD coordScreen = { 0, 0 };
    DWORD dwCharsWritten;
    FillConsoleOutputCharacter(hConsole, L' ', dwConsoleSize, coordScreen, &dwCharsWritten);
    SetConsoleCursorPosition(hConsole, coordScreen);
}

void Wclear() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE) return;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    DWORD dwConsoleSize = csbi.dwSize.X * csbi.dwSize.Y;
    COORD coordScreen = { 0, 0 };
    DWORD dwCharsWritten;
    FillConsoleOutputCharacter(hConsole, L' ', dwConsoleSize, coordScreen, &dwCharsWritten);
    SetConsoleCursorPosition(hConsole, coordScreen);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
    for (int i = 1; i <= 1024; i++) {
        std::cout << " ";
        if (i % 64 == 0) {
            std::cout << "\n";
        }
    }
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), JHcolor);
    chunWclear();
}

std::string ExpandEnvironmentVarsDelayed(const std::string& input, bool delayedExpansion) {
    std::string result = input;

    result = ExpandEnvironmentVarsEnhanced(result);

    if (delayedExpansion) {
        size_t pos = 0;
        while ((pos = result.find('!', pos)) != std::string::npos) {
            size_t end = result.find('!', pos + 1);
            if (end != std::string::npos) {
                std::string varExpr = result.substr(pos + 1, end - pos - 1);

                std::string varName;
                std::string subSpec;
                size_t tildePos = varExpr.find('~');
                if (tildePos != std::string::npos) {
                    varName = varExpr.substr(0, tildePos);
                    subSpec = varExpr.substr(tildePos);
                }
                else {
                    varName = varExpr;
                }

                std::string varValue;
                char* envValue = nullptr;
                size_t envLen = 0;
                if (_dupenv_s(&envValue, &envLen, varName.c_str()) == 0 && envValue != nullptr) {
                    varValue = envValue;
                    free(envValue);
                }

                if (!subSpec.empty()) {
                    varValue = ExtractSubstring(varValue, subSpec);
                }

                result.replace(pos, end - pos + 1, varValue);
                pos += varValue.length();
            }
            else {
                pos++;
            }
        }
    }

    return result;
}

std::string ParseQuotedCommand(const std::string& cmd, size_t& pos) {
    std::string result;
    bool inQuote = false;

    while (pos < cmd.size()) {
        char c = cmd[pos];
        if (c == '"') {
            inQuote = !inQuote;
            pos++;
        }
        else if (c == ' ' && !inQuote) {
            break;
        }
        else {
            result += c;
            pos++;
        }
    }
    return result;
}

bool HandleSetCommand(const std::string& cmd, bool silentMode) {
    std::string setCmd = cmd.size() > 3 ? cmd.substr(3) : "";

    size_t s = setCmd.find_first_not_of(" \t");
    if (s != std::string::npos) setCmd = setCmd.substr(s);

    bool arithmetic = false;
    if (_strnicmp(setCmd.c_str(), "/a", 2) == 0) {
        arithmetic = true;
        setCmd = setCmd.substr(2);
        s = setCmd.find_first_not_of(" \t");
        if (s != std::string::npos) setCmd = setCmd.substr(s);
    }

    size_t eqPos = setCmd.find('=');
    if (eqPos == std::string::npos) {
        std::string varName = setCmd;
        varName.erase(0, varName.find_first_not_of(" \t"));
        varName.erase(varName.find_last_not_of(" \t") + 1);

        if (!varName.empty() && varName.back() == ':') {
            varName.pop_back();
        }

        std::string subSpec;
        size_t tildePos = varName.find('~');
        if (tildePos != std::string::npos) {
            subSpec = varName.substr(tildePos);
            varName = varName.substr(0, tildePos);
        }

        char buffer[4096];
        DWORD len = GetEnvironmentVariableA(varName.c_str(), buffer, sizeof(buffer));
        if (len > 0) {
            std::string valStr = buffer;
            if (!subSpec.empty()) {
                valStr = ExtractSubstring(valStr, subSpec);
            }
            std::cout << varName << "=" << valStr << "\n\n";
        }
        else {
            std::cout << "环境变量 " << varName << " 未定义\n\n";
        }
        return true;
    }

    std::string varName = setCmd.substr(0, eqPos);
    std::string varValue = setCmd.substr(eqPos + 1);

    varName.erase(0, varName.find_first_not_of(" \t"));
    varName.erase(varName.find_last_not_of(" \t") + 1);

    while (!varName.empty() && varName.back() == ':') {
        varName.pop_back();
    }

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
        if (!silentMode) {
            char checkBuffer[4096];
            DWORD checkLen = GetEnvironmentVariableA(varName.c_str(), checkBuffer, sizeof(checkBuffer));
            if (checkLen > 0) {
                std::cout << "设置成功 (" << varName << "=" << checkBuffer << ")\n\n";
            }
            else {
                std::cout << "设置成功\n\n";
            }
        }
    }
    else {
        if (!silentMode) {
            std::cout << "设置失败，错误码: " << GetLastError() << "\n\n";
        }
    }

    return true;
}

bool HandleIfCommand(const std::string& cmd, BatchContext& ctx) {
    std::string rest = cmd.size() > 2 ? cmd.substr(2) : "";
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

    bool isNot = false;
    if (rest.find("not ") == 0) {
        isNot = true;
        rest = rest.substr(4);
        s = rest.find_first_not_of(" \t");
        if (s != std::string::npos) rest = rest.substr(s);
    }

    if (rest.find("exist ") == 0) {
        std::string path = rest.substr(6);
        size_t sp = path.find(' ');
        if (sp != std::string::npos) {
            cmdStart = sp;
            path = path.substr(0, sp);
        }
        if (path.size() >= 2 && path.front() == '"' && path.back() == '"') {
            path = path.substr(1, path.size() - 2);
        }
        path = ExpandEnvironmentVarsDelayed(path, ctx.enabledelayedexpansion);
        DWORD attrs = GetFileAttributesA(path.c_str());
        condition = (attrs != INVALID_FILE_ATTRIBUTES);
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
        condition = (g_lastErrorCode >= (DWORD)level);
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

        if (left.size() >= 2 && left.front() == '"' && left.back() == '"') {
            left = left.substr(1, left.size() - 2);
        }
        if (right.size() >= 2 && right.front() == '"' && right.back() == '"') {
            right = right.substr(1, right.size() - 2);
        }

        left = ExpandEnvironmentVarsDelayed(left, ctx.enabledelayedexpansion);
        right = ExpandEnvironmentVarsDelayed(right, ctx.enabledelayedexpansion);

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

    if (isNot) condition = !condition;

    if (condition && cmdStart > 0) {
        std::string command = rest.substr(cmdStart);
        s = command.find_first_not_of(" \t");
        if (s != std::string::npos) command = command.substr(s);

        if (command.front() == '(') {
            command = command.substr(1);
            size_t e = command.find_last_not_of(" \t");
            if (e != std::string::npos) command = command.substr(0, e + 1);
            if (command.back() == ')') command.pop_back();

            std::string block = command;
            size_t pos = 0;
            while (pos < block.size()) {
                size_t end = block.find('\n', pos);
                if (end == std::string::npos) end = block.size();
                std::string line = block.substr(pos, end - pos);
                if (!line.empty()) {
                    ExecuteBatchLine(line, ctx);
                }
                pos = end + 1;
            }
        }
        else {
            ExecuteBatchLine(command, ctx);
        }
    }
    else if (!condition && hasElse) {
        if (elseCommand.front() == '(') {
            elseCommand = elseCommand.substr(1);
            size_t e = elseCommand.find_last_not_of(" \t");
            if (e != std::string::npos) elseCommand = elseCommand.substr(0, e + 1);
            if (elseCommand.back() == ')') elseCommand.pop_back();

            std::string block = elseCommand;
            size_t pos = 0;
            while (pos < block.size()) {
                size_t end = block.find('\n', pos);
                if (end == std::string::npos) end = block.size();
                std::string line = block.substr(pos, end - pos);
                if (!line.empty()) {
                    ExecuteBatchLine(line, ctx);
                }
                pos = end + 1;
            }
        }
        else {
            ExecuteBatchLine(elseCommand, ctx);
        }
    }

    return true;
}

bool HandleForCommand(const std::string& cmd, BatchContext& ctx) {
    std::string rest = cmd.size() > 3 ? cmd.substr(3) : "";
    size_t s = rest.find_first_not_of(" \t");
    if (s != std::string::npos) rest = rest.substr(s);

    size_t inPos = rest.find("in");
    if (inPos == std::string::npos) return false;

    size_t parenStart = rest.find('(', inPos);
    if (parenStart == std::string::npos) return false;
    size_t parenEnd = rest.find(')', parenStart);
    if (parenEnd == std::string::npos) return false;

    std::string setStr = rest.substr(parenStart + 1, parenEnd - parenStart - 1);

    size_t pctPos = rest.find('%');
    char varName = 'i';
    if (pctPos != std::string::npos && pctPos + 1 < rest.size()) {
        varName = rest[pctPos + 1];
    }

    size_t doPos = rest.find("do", parenEnd);
    if (doPos == std::string::npos) return false;
    std::string command = rest.substr(doPos + 2);
    s = command.find_first_not_of(" \t");
    if (s != std::string::npos) command = command.substr(s);

    std::vector<std::string> items;
    std::string current;
    bool inQuote = false;

    for (size_t i = 0; i < setStr.size(); i++) {
        char c = setStr[i];
        if (c == '"') {
            inQuote = !inQuote;
            if (!inQuote && !current.empty()) {
                items.push_back(current);
                current.clear();
            }
        }
        else if (!inQuote && (c == ' ' || c == ',')) {
            if (!current.empty()) {
                items.push_back(current);
                current.clear();
            }
        }
        else {
            current += c;
        }
    }
    if (!current.empty()) {
        items.push_back(current);
    }

    for (const auto& item : items) {
        std::string cmdCopy = command;
        std::string varStr = "%";
        varStr += varName;
        size_t pos = 0;
        while ((pos = cmdCopy.find(varStr, pos)) != std::string::npos) {
            cmdCopy.replace(pos, 2, item);
            pos += item.size();
        }
        ExecuteBatchLine(cmdCopy, ctx);
    }

    return true;
}

bool ExecuteWithPipe(const std::string& cmd, const CommandInfo& info) {
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
    HANDLE hReadPipe, hWritePipe;

    if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) {
        std::cout << "创建管道失败\n";
        return false;
    }

    SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(hWritePipe, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);

    CommandInfo leftInfo = ParseCommand(info.command);
    CommandInfo rightInfo = ParseCommand(info.pipeCommand);

    STARTUPINFOW siLeft = { sizeof(siLeft) };
    STARTUPINFOW siRight = { sizeof(siRight) };
    PROCESS_INFORMATION piLeft = { 0 };
    PROCESS_INFORMATION piRight = { 0 };

    siLeft.dwFlags = STARTF_USESTDHANDLES;
    siLeft.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    siLeft.hStdOutput = hWritePipe;
    siLeft.hStdError = GetStdHandle(STD_ERROR_HANDLE);

    siRight.dwFlags = STARTF_USESTDHANDLES;
    siRight.hStdInput = hReadPipe;
    siRight.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    siRight.hStdError = GetStdHandle(STD_ERROR_HANDLE);

    std::wstring wLeft = U82W_Path(info.command);
    std::wstring wRight = U82W_Path(info.pipeCommand);
    std::vector<wchar_t> leftBuf(wLeft.begin(), wLeft.end());
    std::vector<wchar_t> rightBuf(wRight.begin(), wRight.end());
    leftBuf.push_back(L'\0');
    rightBuf.push_back(L'\0');

    BOOL leftOk = CreateProcessW(NULL, leftBuf.data(), NULL, NULL, TRUE, 0, NULL, NULL, &siLeft, &piLeft);
    BOOL rightOk = CreateProcessW(NULL, rightBuf.data(), NULL, NULL, TRUE, 0, NULL, NULL, &siRight, &piRight);

    CloseHandle(hWritePipe);
    CloseHandle(hReadPipe);

    if (leftOk && rightOk) {
        WaitForSingleObject(piLeft.hProcess, INFINITE);
        WaitForSingleObject(piRight.hProcess, INFINITE);
        CloseHandle(piLeft.hProcess);
        CloseHandle(piLeft.hThread);
        CloseHandle(piRight.hProcess);
        CloseHandle(piRight.hThread);
        return true;
    }

    if (leftOk) {
        CloseHandle(piLeft.hProcess);
        CloseHandle(piLeft.hThread);
    }
    if (rightOk) {
        CloseHandle(piRight.hProcess);
        CloseHandle(piRight.hThread);
    }

    return false;
}

bool ExecuteBatchLine(const std::string& line, BatchContext& ctx) {
    std::string cmdLine = line;

    size_t start = cmdLine.find_first_not_of(" \t");
    if (start == std::string::npos) return true;
    size_t end = cmdLine.find_last_not_of(" \t\r\n");
    cmdLine = cmdLine.substr(start, end - start + 1);

    if (cmdLine.empty() || cmdLine.substr(0, 2) == "::") return true;

    if (cmdLine == "@echo off" || cmdLine == "@ECHO OFF") {
        ctx.echoOn = false;
        return true;
    }
    if (cmdLine == "@echo on" || cmdLine == "@ECHO ON") {
        ctx.echoOn = true;
        return true;
    }

    if (_strnicmp(cmdLine.c_str(), "echo", 4) == 0) {
        std::string msg = cmdLine.size() > 4 ? cmdLine.substr(4) : "";

        if (!msg.empty() && (msg[0] == '.' || msg[0] == ',' || msg[0] == ';' ||
            msg[0] == '=' || msg[0] == '+' || msg[0] == '/' || msg[0] == ':')) {
            std::cout << "\n";
            return true;
        }

        start = msg.find_first_not_of(" \t");
        if (start != std::string::npos) msg = msg.substr(start);
        end = msg.find_last_not_of(" \t");
        if (end != std::string::npos) msg = msg.substr(0, end + 1);

        if (msg.size() >= 2 && msg.front() == '"' && msg.back() == '"') {
            msg = msg.substr(1, msg.size() - 2);
        }

        msg = ExpandEnvironmentVarsDelayed(msg, ctx.enabledelayedexpansion);
        std::cout << msg << "\n";
        return true;
    }

    if (_strnicmp(cmdLine.c_str(), "setlocal", 8) == 0) {
        std::map<std::string, std::string> backup;
        extern char** _environ;
        for (char** env = _environ; *env != nullptr; ++env) {
            std::string envStr = *env;
            size_t eqPos = envStr.find('=');
            if (eqPos != std::string::npos) {
                backup[envStr.substr(0, eqPos)] = envStr.substr(eqPos + 1);
            }
        }
        ctx.savedEnv = backup;

        if (cmdLine.find("enabledelayedexpansion") != std::string::npos) {
            ctx.enabledelayedexpansion = true;
        }
        return true;
    }

    if (_strnicmp(cmdLine.c_str(), "endlocal", 8) == 0) {
        for (const auto& pair : ctx.savedEnv) {
            SetEnvironmentVariableA(pair.first.c_str(), pair.second.c_str());
        }
        ctx.savedEnv.clear();
        ctx.enabledelayedexpansion = false;
        return true;
    }

    std::string expanded = ExpandEnvironmentVarsDelayed(cmdLine, ctx.enabledelayedexpansion);

    if (_strnicmp(expanded.c_str(), "if", 2) == 0) {
        return HandleIfCommand(expanded, ctx);
    }

    if (_strnicmp(expanded.c_str(), "for", 3) == 0) {
        return HandleForCommand(expanded, ctx);
    }

    if (_strnicmp(expanded.c_str(), "set", 3) == 0) {
        return HandleSetCommand(expanded, !ctx.echoOn);
    }

    if (HandleBuiltinCommand(expanded, !ctx.echoOn)) {
        return true;
    }

    std::wstring wExpanded = U82W_Path(expanded);
    std::vector<wchar_t> cmdBuf(wExpanded.begin(), wExpanded.end());
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
        return true;
    }

    return false;
}

std::vector<std::string> ParseBlock(const std::string& block) {
    std::vector<std::string> lines;
    std::string current;
    int parenDepth = 0;
    bool inQuote = false;

    for (size_t i = 0; i < block.size(); i++) {
        char c = block[i];

        if (c == '"') {
            inQuote = !inQuote;
            current += c;
        }
        else if (!inQuote && c == '(') {
            parenDepth++;
            current += c;
        }
        else if (!inQuote && c == ')') {
            parenDepth--;
            if (parenDepth < 0) break;
            current += c;
        }
        else if (c == '\n' && parenDepth == 0) {
            if (!current.empty()) {
                lines.push_back(current);
                current.clear();
            }
        }
        else {
            current += c;
        }
    }

    if (!current.empty()) {
        lines.push_back(current);
    }

    return lines;
}

bool ExecuteBatchLikeCMD(const std::string& batchPath, const std::vector<std::string>& args) {
    std::ifstream file(batchPath);
    if (!file.is_open()) {
        std::cout << "系统找不到文件 " << batchPath << "\n";
        return false;
    }

    BatchContext ctx;
    std::string line;
    while (std::getline(file, line)) {
        while (!line.empty() && (line.back() == '\r' || line.back() == '\n')) {
            line.pop_back();
        }
        ctx.lines.push_back(line);
    }
    file.close();

    SetEnvironmentVariableA("0", batchPath.c_str());
    for (size_t i = 0; i < args.size() && i < 10; i++) {
        char varName[8];
        sprintf_s(varName, "%zu", i + 1);
        SetEnvironmentVariableA(varName, args[i].c_str());
    }

    ctx.currentLine = 0;
    while (ctx.currentLine < ctx.lines.size()) {
        std::string currentLine = ctx.lines[ctx.currentLine];
        ctx.currentLine++;

        std::string trimmed = currentLine;
        size_t t = trimmed.find_first_not_of(" \t");
        if (t != std::string::npos) trimmed = trimmed.substr(t);
        if (trimmed == ")") continue;

        size_t start = currentLine.find_first_not_of(" \t");
        if (start != std::string::npos && currentLine[start] == ':') {
            continue;
        }

        if (currentLine.find_first_not_of(" \t") != std::string::npos) {
            size_t firstChar = currentLine.find_first_not_of(" \t");
            if (currentLine[firstChar] == '(') {
                std::string block = currentLine.substr(firstChar + 1);
                int parenDepth = 1;
                size_t i = 0;

                while (ctx.currentLine < ctx.lines.size() && parenDepth > 0) {
                    std::string nextLine = ctx.lines[ctx.currentLine];
                    ctx.currentLine++;

                    std::string trimmedNext = nextLine;
                    size_t tn = trimmedNext.find_first_not_of(" \t");
                    if (tn != std::string::npos) trimmedNext = trimmedNext.substr(tn);

                    if (trimmedNext == ")") {
                        parenDepth--;
                        continue;
                    }

                    for (char c : nextLine) {
                        if (c == '(') parenDepth++;
                        else if (c == ')') parenDepth--;
                        block += c;
                    }
                    if (parenDepth > 0) block += '\n';
                }

                std::vector<std::string> blockLines = ParseBlock(block);
                for (const auto& bline : blockLines) {
                    if (!ExecuteBatchLine(bline, ctx)) {
                        return true;
                    }
                }
                continue;
            }
        }

        if (!ExecuteBatchLine(currentLine, ctx)) {
            break;
        }
    }

    return true;
}

std::string GetEnvVarFromRegistry(const std::string& varName) {
    std::string result;
    HKEY hKey;
    char buffer[32767];
    DWORD bufferSize = sizeof(buffer);
    DWORD type;

    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Environment", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        bufferSize = sizeof(buffer);
        if (RegQueryValueExA(hKey, varName.c_str(), NULL, &type, (LPBYTE)buffer, &bufferSize) == ERROR_SUCCESS) {
            if (type == REG_SZ || type == REG_EXPAND_SZ) {
                result = buffer;

                if (type == REG_EXPAND_SZ) {
                    char expanded[32767];
                    DWORD expandedSize = ExpandEnvironmentStringsA(result.c_str(), expanded, sizeof(expanded));
                    if (expandedSize > 0 && expandedSize <= sizeof(expanded)) {
                        result = expanded;
                    }
                }
            }
        }
        RegCloseKey(hKey);
        if (!result.empty()) return result;
    }

    bufferSize = sizeof(buffer);
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment",
        0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        if (RegQueryValueExA(hKey, varName.c_str(), NULL, &type, (LPBYTE)buffer, &bufferSize) == ERROR_SUCCESS) {
            if (type == REG_SZ || type == REG_EXPAND_SZ) {
                result = buffer;
                if (type == REG_EXPAND_SZ) {
                    char expanded[32767];
                    DWORD expandedSize = ExpandEnvironmentStringsA(result.c_str(), expanded, sizeof(expanded));
                    if (expandedSize > 0 && expandedSize <= sizeof(expanded)) {
                        result = expanded;
                    }
                }
            }
        }
        RegCloseKey(hKey);
    }

    return result;
}

void LoadAllEnvironmentVariables() {
    HKEY hKey;

    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Environment", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD index = 0;
        char name[4096];
        char value[4096];
        DWORD nameSize, valueSize, type;

        while (true) {
            nameSize = sizeof(name);
            valueSize = sizeof(value);
            if (RegEnumValueA(hKey, index++, name, &nameSize, NULL, &type, (LPBYTE)value, &valueSize) != ERROR_SUCCESS)
                break;
            if (type == REG_SZ || type == REG_EXPAND_SZ) {
                std::string val = value;
                if (type == REG_EXPAND_SZ) {
                    char expanded[32767];
                    if (ExpandEnvironmentStringsA(val.c_str(), expanded, sizeof(expanded))) {
                        val = expanded;
                    }
                }
                SetEnvironmentVariableA(name, val.c_str());
            }
        }
        RegCloseKey(hKey);
    }

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment",
        0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD index = 0;
        char name[4096];
        char value[4096];
        DWORD nameSize, valueSize, type;

        while (true) {
            nameSize = sizeof(name);
            valueSize = sizeof(value);
            if (RegEnumValueA(hKey, index++, name, &nameSize, NULL, &type, (LPBYTE)value, &valueSize) != ERROR_SUCCESS)
                break;
            if (type == REG_SZ || type == REG_EXPAND_SZ) {
                char* existing = nullptr;
                size_t existingLen = 0;
                _dupenv_s(&existing, &existingLen, name);
                if (!existing) {
                    std::string val = value;
                    if (type == REG_EXPAND_SZ) {
                        char expanded[32767];
                        if (ExpandEnvironmentStringsA(val.c_str(), expanded, sizeof(expanded))) {
                            val = expanded;
                        }
                    }
                    SetEnvironmentVariableA(name, val.c_str());
                }
                free(existing);
            }
        }
        RegCloseKey(hKey);
    }
}

bool IsQuoted(const std::string& str, size_t pos) {
    int quoteCount = 0;
    for (size_t i = 0; i < pos && i < str.length(); i++) {
        if (str[i] == '"') quoteCount++;
    }
    return (quoteCount % 2) == 1;
}

CommandInfo ParseCommand(const std::string& cmd) {
    CommandInfo info;
    info.hasPipe = false;
    info.hasRedirect = false;
    info.redirectType = 0;

    std::string expandedCmd = ExpandEnvironmentVars(cmd);

    std::string lowerCmd = expandedCmd;
    std::transform(lowerCmd.begin(), lowerCmd.end(), lowerCmd.begin(), ::tolower);

    size_t firstNonSpace = lowerCmd.find_first_not_of(" \t");
    std::string trimmedLower = (firstNonSpace != std::string::npos)
        ? lowerCmd.substr(firstNonSpace)
        : lowerCmd;

    if (trimmedLower.find("solvei") == 0) {
        info.command = expandedCmd;

        size_t start = info.command.find_first_not_of(" \t");
        if (start != std::string::npos) info.command = info.command.substr(start);
        size_t end = info.command.find_last_not_of(" \t");
        if (end != std::string::npos) info.command = info.command.substr(0, end + 1);

        size_t space = info.command.find(' ');
        if (space != std::string::npos) {
            info.args = info.command.substr(space + 1);
            info.command = info.command.substr(0, space);
        }

        return info;
    }

    std::string buffer;
    bool inQuote = false;
    int splitPos = -1;
    char foundOp = 0;

    for (size_t i = 0; i < expandedCmd.length(); i++) {
        char c = expandedCmd[i];
        if (c == '"') {
            inQuote = !inQuote;
            buffer += c;
        }
        else if (!inQuote && c == '|') {
            foundOp = '|';
            splitPos = i;
            break;
        }
        else if (!inQuote && c == '>') {
            foundOp = '>';
            splitPos = i;
            break;
        }
        else if (!inQuote && c == '<') {
            foundOp = '<';
            splitPos = i;
            break;
        }
        else buffer += c;
    }

    if (splitPos >= 0) {
        info.command = expandedCmd.substr(0, splitPos);
        size_t end = info.command.find_last_not_of(" \t");
        if (end != std::string::npos) {
            info.command = info.command.substr(0, end + 1);
        }

        std::string rest = expandedCmd.substr(splitPos + 1);

        if (foundOp == '|') {
            info.hasPipe = true;
            size_t start = rest.find_first_not_of(" \t");
            if (start != std::string::npos) {
                info.pipeCommand = rest.substr(start);
            }
        }
        else if (foundOp == '>') {
            info.hasRedirect = true;

            if (!rest.empty() && rest[0] == '>') {
                info.redirectType = 'A';
                rest = rest.substr(1);
            }
            else {
                info.redirectType = '>';
            }

            size_t start = rest.find_first_not_of(" \t");
            if (start != std::string::npos) {
                std::string filePart = rest.substr(start);

                bool inFileQuote = false;
                std::string fileName;
                for (size_t j = 0; j < filePart.length(); j++) {
                    char fc = filePart[j];
                    if (fc == '"') {
                        inFileQuote = !inFileQuote;
                    }
                    else if (!inFileQuote && (fc == ' ' || fc == '\t')) {
                        break;
                    }
                    else {
                        fileName += fc;
                    }
                }

                if (fileName.size() >= 2 && fileName.front() == '"' && fileName.back() == '"') {
                    fileName = fileName.substr(1, fileName.size() - 2);
                }
                info.redirectFile = fileName;
            }
        }
        else if (foundOp == '<') {
            info.hasRedirect = true;
            info.redirectType = '<';

            size_t start = rest.find_first_not_of(" \t");
            if (start != std::string::npos) {
                std::string filePart = rest.substr(start);

                bool inFileQuote = false;
                std::string fileName;
                for (size_t j = 0; j < filePart.length(); j++) {
                    char fc = filePart[j];
                    if (fc == '"') {
                        inFileQuote = !inFileQuote;
                    }
                    else if (!inFileQuote && (fc == ' ' || fc == '\t')) {
                        break;
                    }
                    else {
                        fileName += fc;
                    }
                }

                if (fileName.size() >= 2 && fileName.front() == '"' && fileName.back() == '"') {
                    fileName = fileName.substr(1, fileName.size() - 2);
                }
                info.redirectFile = fileName;
            }
        }
    }
    else {
        info.command = expandedCmd;
    }

    size_t start = info.command.find_first_not_of(" \t");
    if (start != std::string::npos) info.command = info.command.substr(start);
    size_t end = info.command.find_last_not_of(" \t");
    if (end != std::string::npos) info.command = info.command.substr(0, end + 1);

    size_t space = info.command.find(' ');
    if (space != std::string::npos) {
        info.args = info.command.substr(space + 1);
        info.command = info.command.substr(0, space);
    }

    if (info.hasPipe) {
        size_t ps = info.pipeCommand.find_first_not_of(" \t");
        if (ps != std::string::npos) info.pipeCommand = info.pipeCommand.substr(ps);
    }

    return info;
}

bool ExecuteWithRedirection(const std::string& cmdLine, const CommandInfo& info) {
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi = { 0 };
    SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };

    HANDLE hFile = INVALID_HANDLE_VALUE;
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

    std::string expandedCmdLine = ExpandEnvironmentVars(cmdLine);

    if (info.hasRedirect && !info.redirectFile.empty()) {
        std::string fileName = info.redirectFile;
        if (!fileName.empty() && fileName.front() == '"' && fileName.back() == '"')
            fileName = fileName.substr(1, fileName.size() - 2);

        fileName = ExpandEnvironmentVars(fileName);
        std::wstring wFileName = U82W_Path(fileName);

        if (info.redirectType == '<') {
            hFile = CreateFileW(wFileName.c_str(), GENERIC_READ, FILE_SHARE_READ,
                &sa, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFile != INVALID_HANDLE_VALUE) si.hStdInput = hFile;
        }
        else {
            DWORD create = (info.redirectType == 'A') ? OPEN_ALWAYS : CREATE_ALWAYS;
            if (fileName == "nul") {
                hFile = CreateFileW(L"nul", GENERIC_WRITE, FILE_SHARE_WRITE,
                    &sa, OPEN_EXISTING, 0, NULL);
            }
            else {
                hFile = CreateFileW(wFileName.c_str(), GENERIC_WRITE,
                    FILE_SHARE_READ, &sa, create,
                    FILE_ATTRIBUTE_NORMAL, NULL);
            }
            if (hFile != INVALID_HANDLE_VALUE) {
                si.hStdOutput = hFile;
                si.hStdError = hFile;
            }
        }
    }

    std::wstring wCmdLine = U82W_Path(expandedCmdLine);
    std::vector<wchar_t> cmdBuf(wCmdLine.begin(), wCmdLine.end());
    cmdBuf.push_back(L'\0');

    BOOL ok = CreateProcessW(NULL, cmdBuf.data(), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
    if (ok) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);
    return ok;
}

void HandleCD(const std::string& cmd) {
    if (cmd == "cd" || cmd == "CD" || cmd == "Cd") {
        wchar_t buf[MAX_PATH];
        GetCurrentDirectoryW(MAX_PATH, buf);
        std::cout << W2U8(buf) << "\n\n";
        return;
    }

    std::string rest = cmd.substr(2);
    size_t s = rest.find_first_not_of(" \t\"");
    size_t e = rest.find_last_not_of(" \t\"");
    if (s == std::string::npos) {
        std::cout << "系统找不到指定路径。\n\n";
        return;
    }
    rest = rest.substr(s, e - s + 1);

    rest = ExpandEnvironmentVars(rest);
    std::wstring wPath = U82W(rest);

    if (!SetCurrentDirectoryW(wPath.c_str())) {
        std::cout << "系统找不到指定路径。\n\n";
    }
}

std::string WCharToString(const WCHAR* wstr) {
    if (!wstr) return "";
    int len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
    if (len <= 0) return "";
    char* buffer = new char[len];
    WideCharToMultiByte(CP_ACP, 0, wstr, -1, buffer, len, NULL, NULL);
    std::string result(buffer);
    delete[] buffer;
    return result;
}

void ShowPrompt() {
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);

    std::string exeDir = exePath;
    size_t lastSlash = exeDir.find_last_of("\\/");
    if (lastSlash != std::string::npos) {
        exeDir = exeDir.substr(0, lastSlash + 1);
    }

    std::string configPath = exeDir + ".zjhcmd\\";
    std::string fullPath = configPath + "ZJHCMDTIPS";

    // 用宽字符模式打开，自动识别 UTF-8 / ANSI
    std::ifstream file(fullPath, std::ios::binary);
    if (!file.is_open()) {
        wchar_t curDirW[MAX_PATH];
        GetCurrentDirectoryW(MAX_PATH, curDirW);
        std::cout << W2U8(curDirW) << ">";
        return;
    }

    std::string content((std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());
    file.close();

    if (content.size() >= 3 &&
        (unsigned char)content[0] == 0xEF &&
        (unsigned char)content[1] == 0xBB &&
        (unsigned char)content[2] == 0xBF) {
        content = content.substr(3);
    }

    size_t s = content.find_first_not_of(" \t\r\n");
    size_t e = content.find_last_not_of(" \t\r\n");
    if (s == std::string::npos) {
        std::cout << ">";
        return;
    }
    content = content.substr(s, e - s + 1);

    bool validUtf8 = true;
    for (size_t i = 0; i < content.size(); ) {
        unsigned char c = content[i];
        int n = 0;
        if ((c & 0x80) == 0x00) n = 1;
        else if ((c & 0xE0) == 0xC0) n = 2;
        else if ((c & 0xF0) == 0xE0) n = 3;
        else if ((c & 0xF8) == 0xF0) n = 4;
        else { validUtf8 = false; break; }

        if (i + n > content.size()) { validUtf8 = false; break; }
        for (int k = 1; k < n; ++k) {
            if (((unsigned char)content[i + k] & 0xC0) != 0x80) {
                validUtf8 = false; break;
            }
        }
        if (!validUtf8) break;
        i += n;
    }

    if (!validUtf8) {
        content = W2U8(U82W(content));
    }

    if (content.empty()) std::cout << ">";
    else                 std::cout << content << ">";
}

bool SetWindowsThemeMode(bool lightMode) {
    HKEY hKey;
    DWORD value = lightMode ? 1 : 0;

    if (RegOpenKeyExW(
        HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        0, KEY_SET_VALUE, &hKey) != ERROR_SUCCESS) {
        std::cerr << "RegOpenKeyEx 失败\n";
        return false;
    }

    RegSetValueExW(hKey, L"SystemUsesLightTheme", 0, REG_DWORD,
        (const BYTE*)&value, sizeof(DWORD));
    RegSetValueExW(hKey, L"AppsUseLightTheme", 0, REG_DWORD,
        (const BYTE*)&value, sizeof(DWORD));

    RegCloseKey(hKey);

    SendMessageTimeoutW(
        HWND_BROADCAST,
        WM_SETTINGCHANGE,
        0,
        (LPARAM)L"ImmersiveColorSet",
        SMTO_NORMAL,
        100,
        NULL);

    SendMessageTimeoutW(
        HWND_BROADCAST,
        WM_THEMECHANGED,
        0, 0,
        SMTO_NORMAL,
        100,
        NULL);

    std::cout << (lightMode ? "已切换到浅色模式" : "已切换到深色模式") << "\n";
    return true;
}

std::wstring StringToWString(const std::string& str) {
    if (str.empty()) return L"";
    int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    if (len <= 0) return L"";
    std::wstring wstr(len - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], len);
    return wstr;
}

extern std::vector<std::string> g_scriptLines;

#ifndef SPI_SETDESKWALLPAPERSTYLE
#define SPI_SETDESKWALLPAPERSTYLE 0x005F
#endif

#ifndef SPI_SETDESKTOPWALLPAPER
#define SPI_SETDESKTOPWALLPAPER 0x0014
#endif

size_t FindLabel(const std::string& label) {
    for (size_t i = 0; i < g_scriptLines.size(); i++) {
        std::string line = g_scriptLines[i];
        size_t start = line.find_first_not_of(" \t");
        if (start == std::string::npos) continue;
        size_t end = line.find_last_not_of(" \t");
        line = line.substr(start, end - start + 1);

        if (!line.empty() && line[0] == ':') {
            std::string lbl = line.substr(1);
            size_t lblEnd = lbl.find_first_of(" \t");
            if (lblEnd != std::string::npos) {
                lbl = lbl.substr(0, lblEnd);
            }
            if (_stricmp(lbl.c_str(), label.c_str()) == 0) {
                return i;
            }
        }
    }
    return (size_t)-1;
}

bool SetDesktopWallpaper(const std::string& imageAbsolutePath) {
    std::string expandedPath = ExpandEnvironmentVars(imageAbsolutePath);

    // 1. UTF-8 -> 宽字符
    std::wstring wSrcPath;
    {
        int len = MultiByteToWideChar(CP_UTF8, 0, expandedPath.c_str(), -1, NULL, 0);
        if (len <= 0) {
            std::cout << "壁纸设置失败：路径转宽字符失败" << std::endl;
            return false;
        }
        wSrcPath.resize(len - 1);
        MultiByteToWideChar(CP_UTF8, 0, expandedPath.c_str(), -1, &wSrcPath[0], len);
    }

    // 2. 用 W 版 API 检查文件
    DWORD attrs = GetFileAttributesW(wSrcPath.c_str());
    if (attrs == INVALID_FILE_ATTRIBUTES) {
        DWORD err = GetLastError();
        std::wcout << L"壁纸设置失败：找不到文件 " << wSrcPath << std::endl;
        std::cout << "  错误码: " << err << std::endl;
        return false;
    }
    if (attrs & FILE_ATTRIBUTE_DIRECTORY) {
        std::cout << "壁纸设置失败：路径是目录" << std::endl;
        return false;
    }

    // 3. 复制到本地
    wchar_t userProfileW[MAX_PATH] = { 0 };
    GetEnvironmentVariableW(L"USERPROFILE", userProfileW, MAX_PATH);
    std::wstring cacheDirW = std::wstring(userProfileW) + L"\\Pictures";
    CreateDirectoryW(cacheDirW.c_str(), NULL);

    std::wstring extW = L".jpg";
    size_t dotPos = wSrcPath.find_last_of(L'.');
    if (dotPos != std::wstring::npos) {
        extW = wSrcPath.substr(dotPos);
    }
    std::wstring wCachePath = cacheDirW + L"\\_zjhcmd_wallpaper" + extW;

    if (!CopyFileW(wSrcPath.c_str(), wCachePath.c_str(), FALSE)) {
        DWORD err = GetLastError();
        std::wcout << L"壁纸设置失败：无法复制图片到本地" << std::endl;
        std::wcout << L"  源文件: " << wSrcPath << std::endl;
        std::wcout << L"  目标: " << wCachePath << std::endl;
        std::cout << "  错误码: " << err << std::endl;
        return false;
    }

    // 4. 设置壁纸样式
    SystemParametersInfoW(SPI_SETDESKWALLPAPERSTYLE, 10, NULL,
        SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);

    // 5. 设置壁纸
    BOOL ret = SystemParametersInfoW(
        SPI_SETDESKTOPWALLPAPER,
        0,
        (PVOID)wCachePath.c_str(),
        SPIF_UPDATEINIFILE | SPIF_SENDCHANGE
    );

    if (ret) {
        std::cout << "壁纸设置成功" << std::endl;
        std::wcout << L"  " << wSrcPath << std::endl;
        return true;
    }
    else {
        DWORD err = GetLastError();
        std::cout << "壁纸设置失败，错误码: " << err << std::endl;
        return false;
    }
}

void ListDirectoryRecursive(const std::string& directory, const std::string& pattern,
    int& fileCount, int& dirCount, long long& totalSize, int depth) {
    if (g_dirInterrupted) return;

    WIN32_FIND_DATAA findData;
    HANDLE hFind;
    std::string searchPath = directory + "\\" + pattern;

    hFind = FindFirstFileA(searchPath.c_str(), &findData);
    if (hFind == INVALID_HANDLE_VALUE) {
        return;
    }

    std::string indent(depth * 2, ' ');

    do {
        if (g_dirInterrupted) break;

        if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0) {
            continue;
        }

        SYSTEMTIME sysTime;
        FileTimeToSystemTime(&findData.ftLastWriteTime, &sysTime);

        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            std::cout << indent << "  ";
            std::cout.width(10);
            std::cout << "<DIR>";
            std::cout << "  " << sysTime.wMonth << "/" << sysTime.wDay << "/" << sysTime.wYear
                << "  " << sysTime.wHour << ":" << std::setw(2) << std::setfill('0') << sysTime.wMinute
                << "  ";
            std::cout << findData.cFileName << "\n";
            dirCount++;

            std::string subDir = directory + "\\" + findData.cFileName;
            ListDirectoryRecursive(subDir, pattern, fileCount, dirCount, totalSize, depth + 1);
        }
        else {
            std::cout << indent << "  ";
            std::cout.width(10);
            std::cout << findData.nFileSizeLow;
            totalSize += findData.nFileSizeLow;
            fileCount++;

            std::cout << "  " << sysTime.wMonth << "/" << sysTime.wDay << "/" << sysTime.wYear
                << "  " << sysTime.wHour << ":" << std::setw(2) << std::setfill('0') << sysTime.wMinute
                << "  ";
            std::cout << findData.cFileName << "\n";
        }

    } while (FindNextFileA(hFind, &findData) != 0 && !g_dirInterrupted);

    FindClose(hFind);
}

void ListDirectory(const std::string& path = "", bool recursive = false) {
    std::wstring targetDir;
    std::wstring pattern = L"*";

    if (path.empty()) {
        wchar_t buf[MAX_PATH];
        GetCurrentDirectoryW(MAX_PATH, buf);
        targetDir = buf;
    }
    else {
        std::wstring wPath = U82W(ExpandEnvironmentVars(path));
        DWORD attrs = GetFileAttributesW(wPath.c_str());

        if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY)) {
            targetDir = wPath;
        }
        else {
            size_t slash = wPath.find_last_of(L"\\/");
            if (slash != std::wstring::npos) {
                targetDir = wPath.substr(0, slash);
                pattern = wPath.substr(slash + 1);
                if (pattern.empty()) pattern = L"*";
            }
            else {
                wchar_t buf[MAX_PATH];
                GetCurrentDirectoryW(MAX_PATH, buf);
                targetDir = buf;
                pattern = wPath;
            }
        }
    }

    wchar_t volumeName[MAX_PATH + 1] = { 0 };
    DWORD   serial = 0;
    wchar_t rootPath[MAX_PATH] = { 0 };
    if (targetDir.size() >= 2 && targetDir[1] == L':') {
        rootPath[0] = targetDir[0];
        rootPath[1] = L':';
        rootPath[2] = L'\\';
    }
    if (rootPath[0] != 0) {
        GetVolumeInformationW(rootPath, volumeName, MAX_PATH,
            &serial, nullptr, nullptr, nullptr, 0);
    }

    std::cout << "\n 驱动器 " << (char)towupper(targetDir[0]) << ": 中的卷是 ";
    if (volumeName[0] == 0) std::cout << "没有卷标";
    else std::cout << W2U8(volumeName);
    std::cout << "\n";
    if (serial != 0) {
        char buf[32];
        sprintf_s(buf, "%04X-%04X", (serial >> 16) & 0xFFFF, serial & 0xFFFF);
        std::cout << " 卷的序列号是 " << buf << "\n";
    }

    std::cout << "\n 目录 " << W2U8(targetDir) << "\n\n";

    int fileCount = 0, dirCount = 0;
    unsigned long long totalSize = 0;

    std::function<void(const std::wstring&)> walk = [&](const std::wstring& dir) {
        std::wstring search = dir + L"\\" + pattern;
        WIN32_FIND_DATAW fd;
        HANDLE h = FindFirstFileW(search.c_str(), &fd);
        if (h == INVALID_HANDLE_VALUE) return;

        do {
            if (IsSystemDir(fd.cFileName)) continue;

            FILETIME ftLocal;
            FileTimeToLocalFileTime(&fd.ftLastWriteTime, &ftLocal);
            SYSTEMTIME st;
            FileTimeToSystemTime(&ftLocal, &st);

            char timeBuf[32];
            sprintf_s(timeBuf, "%04d/%02d/%02d  %02d:%02d",
                st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);

            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                std::cout << timeBuf << "    <DIR>          "
                    << W2U8(fd.cFileName) << "\n";
                dirCount++;
                if (recursive) {
                    walk(dir + L"\\" + fd.cFileName);
                }
            }
            else {
                unsigned long long size =
                    ((unsigned long long)fd.nFileSizeHigh << 32) | fd.nFileSizeLow;
                totalSize += size;

                std::string sizeStr = FormatNumber(size);
                if (sizeStr.size() < 15)
                    sizeStr = std::string(15 - sizeStr.size(), ' ') + sizeStr;

                std::cout << timeBuf << "  " << sizeStr << "  "
                    << W2U8(fd.cFileName) << "\n";
                fileCount++;
            }
        } while (FindNextFileW(h, &fd));
        FindClose(h);
        };

    if (recursive) {
        walk(targetDir);
    }
    else {
        std::wstring search = targetDir + L"\\" + pattern;
        WIN32_FIND_DATAW fd;
        HANDLE h = FindFirstFileW(search.c_str(), &fd);
        if (h == INVALID_HANDLE_VALUE) {
            std::cout << " 找不到文件\n\n";
            return;
        }
        do {
            if (IsSystemDir(fd.cFileName)) continue;

            FILETIME ftLocal;
            FileTimeToLocalFileTime(&fd.ftLastWriteTime, &ftLocal);
            SYSTEMTIME st;
            FileTimeToSystemTime(&ftLocal, &st);

            char timeBuf[32];
            sprintf_s(timeBuf, "%04d/%02d/%02d  %02d:%02d",
                st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);

            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                std::cout << timeBuf << "    <DIR>          "
                    << W2U8(fd.cFileName) << "\n";
                dirCount++;
            }
            else {
                unsigned long long size =
                    ((unsigned long long)fd.nFileSizeHigh << 32) | fd.nFileSizeLow;
                totalSize += size;

                std::string sizeStr = FormatNumber(size);
                if (sizeStr.size() < 15)
                    sizeStr = std::string(15 - sizeStr.size(), ' ') + sizeStr;

                std::cout << timeBuf << "  " << sizeStr << "  "
                    << W2U8(fd.cFileName) << "\n";
                fileCount++;
            }
        } while (FindNextFileW(h, &fd));
        FindClose(h);
    }

    std::cout << "\n";
    std::cout << "              " << fileCount << " 个文件  "
        << FormatNumber(totalSize) << " 字节\n";

    if (rootPath[0] != 0) {
        ULARGE_INTEGER freeBytes, totalBytes;
        if (GetDiskFreeSpaceExW(rootPath, &freeBytes, &totalBytes, nullptr)) {
            std::cout << "              " << dirCount << " 个目录  "
                << FormatNumber(freeBytes.QuadPart) << " 字节可用\n\n";
        }
        else {
            std::cout << "              " << dirCount << " 个目录\n\n";
        }
    }
    else {
        std::cout << "              " << dirCount << " 个目录\n\n";
    }
}

void SecretFriendshipMode() {
    std::cout << "\n| 欢迎进入友情秘密模式！\n";
    std::cout << "这里可以写下只有你们知道的悄悄话...\n\n\n";
    int nm;
    nm = _getch();
    std::string newEntry;
    std::ifstream readFile("JH_FriendShip_diary.FR");
    std::string line;
    std::ofstream writeFile("JH_FriendShip_diary.FR", std::ios::app);
    std::cout << "===== 友情小日记 =====\n";
    if (readFile.is_open()) {

        while (std::getline(readFile, line)) {
            std::cout << "\\/ " << line << "\n";
        }
        readFile.close();
    }
    std::cout << "\n> 今天想记录什么？\n";
    std::getline(std::cin, newEntry);
    writeFile << __DATE__ << "：" << newEntry << "\n";
    writeFile.close();

    std::cout << "\n已保存到友情日记\n";
}

bool DeleteDirectoryRecursive(const std::wstring& path) {
    std::wstring searchPath = path + L"\\*";
    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findData);

    if (hFind == INVALID_HANDLE_VALUE) {
        return false;
    }

    bool success = true;

    do {
        if (wcscmp(findData.cFileName, L".") == 0 ||
            wcscmp(findData.cFileName, L"..") == 0) {
            continue;
        }

        std::wstring fullPath = path + L"\\" + findData.cFileName;

        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (!DeleteDirectoryRecursive(fullPath)) {
                success = false;
            }
        }
        else {
            if (!DeleteFileW(fullPath.c_str())) {
                DWORD err = GetLastError();
                success = false;
            }
        }
    } while (FindNextFileW(hFind, &findData) != 0);

    FindClose(hFind);

    if (success && !RemoveDirectoryW(path.c_str())) {
        DWORD err = GetLastError();
        success = false;
    }

    return success;
}

std::wstring CharToWstring(const char* charStr) {
    if (!charStr) return std::wstring();

    int size_needed = MultiByteToWideChar(CP_UTF8, 0, charStr, -1, NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, charStr, -1, &wstrTo[0], size_needed);
    return wstrTo;
}


bool ExecuteWithRedirection(const std::string& cmdLine, const CommandInfo& info);

CommandSequence ParseCommandSequence(const std::string& cmdLine) {
    CommandSequence result;
    result.hasParen = false;

    std::string expanded = ExpandEnvironmentVars(cmdLine);

    size_t firstParen = expanded.find('(');
    size_t lastParen = expanded.rfind(')');

    if (firstParen != std::string::npos && lastParen != std::string::npos &&
        lastParen > firstParen) {
        std::string beforeParen = expanded.substr(0, firstParen);
        std::string afterParen = expanded.substr(lastParen + 1);

        bool onlySpacesBefore = beforeParen.find_first_not_of(" \t") == std::string::npos;
        bool onlySpacesAfter = afterParen.find_first_not_of(" \t") == std::string::npos;

        if (onlySpacesBefore && onlySpacesAfter) {
            result.hasParen = true;
            result.groupedCommand = expanded.substr(firstParen + 1, lastParen - firstParen - 1);
            CommandSequence innerSeq = ParseCommandSequence(result.groupedCommand);
            result.commands = innerSeq.commands;
            result.operators = innerSeq.operators;
            return result;
        }
    }

    std::string currentCmd;
    bool inQuote = false;
    bool inParen = false;
    int parenDepth = 0;

    for (size_t i = 0; i < expanded.length(); i++) {
        char c = expanded[i];

        if (c == '"') {
            inQuote = !inQuote;
            currentCmd += c;
            continue;
        }

        if (!inQuote && c == '(') {
            inParen = true;
            parenDepth++;
            currentCmd += c;
            continue;
        }

        if (!inQuote && c == ')') {
            parenDepth--;
            if (parenDepth == 0) inParen = false;
            currentCmd += c;
            continue;
        }

        if (!inQuote && !inParen) {
            if (c == '&' && i + 1 < expanded.length() && expanded[i + 1] == '&') {
                size_t start = currentCmd.find_first_not_of(" \t");
                size_t end = currentCmd.find_last_not_of(" \t");
                if (start != std::string::npos) {
                    result.commands.push_back(currentCmd.substr(start, end - start + 1));
                    result.operators.push_back('&');  // '&' 表示 &&
                }
                else if (!currentCmd.empty()) {
                    result.commands.push_back("");
                    result.operators.push_back('&');
                }
                currentCmd.clear();
                i++;
                continue;
            }

            if (c == '|' && i + 1 < expanded.length() && expanded[i + 1] == '|') {
                size_t start = currentCmd.find_first_not_of(" \t");
                size_t end = currentCmd.find_last_not_of(" \t");
                if (start != std::string::npos) {
                    result.commands.push_back(currentCmd.substr(start, end - start + 1));
                    result.operators.push_back('|');  // '|' 表示 ||
                }
                else if (!currentCmd.empty()) {
                    result.commands.push_back("");
                    result.operators.push_back('|');
                }
                currentCmd.clear();
                i++;
                continue;
            }

            if (c == '&' && (i + 1 >= expanded.length() || expanded[i + 1] != '&')) {
                size_t start = currentCmd.find_first_not_of(" \t");
                size_t end = currentCmd.find_last_not_of(" \t");
                if (start != std::string::npos) {
                    result.commands.push_back(currentCmd.substr(start, end - start + 1));
                    result.operators.push_back(';');  // ';' 表示顺序执行（类似 &）
                }
                else if (!currentCmd.empty()) {
                    result.commands.push_back("");
                    result.operators.push_back(';');
                }
                currentCmd.clear();
                continue;
            }
        }

        currentCmd += c;
    }

    if (!currentCmd.empty()) {
        size_t start = currentCmd.find_first_not_of(" \t");
        size_t end = currentCmd.find_last_not_of(" \t");
        if (start != std::string::npos) {
            result.commands.push_back(currentCmd.substr(start, end - start + 1));
        }
        else if (!currentCmd.empty()) {
            result.commands.push_back("");
        }
    }

    return result;
}

static int ExecuteSingleCommandAndGetExitCode(const std::string& cmd, bool silentMode = false) {
    CommandInfo info = ParseCommand(cmd);

    if (info.hasRedirect) {
        if (ExecuteWithRedirection(cmd, info)) {
            return 0;
        }
        return 1;
    }

    if (HandleBuiltinCommand(cmd, silentMode)) {
        return 0;
    }

    std::string expandedCmd = ExpandEnvironmentVars(cmd);
    std::wstring wExpanded = U82W_Path(expandedCmd);
    std::vector<wchar_t> cmdBuf(wExpanded.begin(), wExpanded.end());
    cmdBuf.push_back(L'\0');

    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi = { 0 };
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

    DWORD exitCode = 1;

    if (CreateProcessW(NULL, cmdBuf.data(), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        GetExitCodeProcess(pi.hProcess, &exitCode);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    else {
        std::string psCmd = "powershell -Command \"" + cmd + "\"";
        std::wstring wPsCmd = U82W_Path(psCmd);
        std::vector<wchar_t> psBuf(wPsCmd.begin(), wPsCmd.end());
        psBuf.push_back(L'\0');

        STARTUPINFOW si2 = { sizeof(si2) };
        PROCESS_INFORMATION pi2 = { 0 };
        si2.dwFlags = STARTF_USESTDHANDLES;
        si2.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
        si2.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
        si2.hStdError = GetStdHandle(STD_ERROR_HANDLE);

        if (CreateProcessW(NULL, psBuf.data(), NULL, NULL, TRUE, 0, NULL, NULL, &si2, &pi2)) {
            WaitForSingleObject(pi2.hProcess, INFINITE);
            GetExitCodeProcess(pi2.hProcess, &exitCode);
            CloseHandle(pi2.hProcess);
            CloseHandle(pi2.hThread);
        }
    }

    return exitCode;
}

bool ExecuteSequence(const CommandSequence& seq) {
    if (seq.commands.empty()) return true;

    int lastExitCode = 0;
    bool shouldExecute = true;

    for (size_t i = 0; i < seq.commands.size(); i++) {
        const std::string& cmd = seq.commands[i];

        if (cmd.empty()) {
            continue;
        }

        CommandSequence innerSeq = ParseCommandSequence(cmd);
        if (innerSeq.hasParen || innerSeq.commands.size() > 1) {
            if (shouldExecute) {
                ExecuteSequence(innerSeq);

            }
            continue;
        }

        if (i > 0) {
            char op = seq.operators[i - 1];
            if (op == '&') {
                shouldExecute = (lastExitCode == 0);
            }
            else if (op == '|') {
                shouldExecute = (lastExitCode != 0);
            }
            else if (op == ';') {
                shouldExecute = true;
            }
        }

        if (shouldExecute) {
            lastExitCode = ExecuteSingleCommandAndGetExitCode(cmd, false);

            if (lastExitCode != 0) {
                std::cout << "命令执行失败，退出码: " << lastExitCode << std::endl;
            }
        }
    }

    return lastExitCode == 0;
}

std::string ExpandEnvironmentVars(const std::string& input) {
    return ExpandEnvironmentVarsEnhanced(input);
}

void ListDirectoryBare(const std::string& path, bool recursive) {
    std::wstring targetDir;
    std::wstring pattern = L"*";

    if (path.empty()) {
        wchar_t buf[MAX_PATH];
        GetCurrentDirectoryW(MAX_PATH, buf);
        targetDir = buf;
    }
    else {
        std::wstring wPath = U82W(ExpandEnvironmentVars(path));
        DWORD attrs = GetFileAttributesW(wPath.c_str());
        if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY)) {
            targetDir = wPath;
        }
        else {
            size_t slash = wPath.find_last_of(L"\\/");
            if (slash != std::wstring::npos) {
                targetDir = wPath.substr(0, slash);
                pattern = wPath.substr(slash + 1);
                if (pattern.empty()) pattern = L"*";
            }
            else {
                wchar_t buf[MAX_PATH];
                GetCurrentDirectoryW(MAX_PATH, buf);
                targetDir = buf;
                pattern = wPath;
            }
        }
    }

    std::function<void(const std::wstring&)> walk = [&](const std::wstring& dir) {
        WIN32_FIND_DATAW fd;
        HANDLE h = FindFirstFileW((dir + L"\\*").c_str(), &fd);
        if (h == INVALID_HANDLE_VALUE) return;
        do {
            if (IsSystemDir(fd.cFileName)) continue;
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                std::wstring full = dir + L"\\" + fd.cFileName;
                std::cout << W2U8(full) << "\n";
                if (recursive) walk(full);
            }
            else if (MatchPattern(W2U8(fd.cFileName), W2U8(pattern))) {
                std::cout << W2U8(dir + L"\\" + fd.cFileName) << "\n";
            }
        } while (FindNextFileW(h, &fd));
        FindClose(h);
        };

    walk(targetDir);
}

bool MatchPattern(const std::string& filename, const std::string& pattern) {
    if (pattern == "*") return true;
    if (pattern == filename) return true;

    size_t starPos = pattern.find('*');
    size_t questionPos = pattern.find('?');

    if (starPos == std::string::npos && questionPos == std::string::npos) {
        return filename == pattern;
    }

    if (starPos != std::string::npos && questionPos == std::string::npos) {
        std::string prefix = pattern.substr(0, starPos);
        std::string suffix = pattern.substr(starPos + 1);

        if (filename.size() < prefix.size() + suffix.size()) return false;

        return filename.substr(0, prefix.size()) == prefix &&
            filename.substr(filename.size() - suffix.size()) == suffix;
    }

    std::function<bool(size_t, size_t)> matchHelper = [&](size_t fi, size_t pi) -> bool {
        if (pi == pattern.size()) return fi == filename.size();

        if (pattern[pi] == '*') {
            for (size_t len = 0; fi + len <= filename.size(); len++) {
                if (matchHelper(fi + len, pi + 1)) return true;
            }
            return false;
        }

        if (fi >= filename.size()) return false;

        if (pattern[pi] == '?') {
            return matchHelper(fi + 1, pi + 1);
        }

        if (filename[fi] == pattern[pi]) {
            return matchHelper(fi + 1, pi + 1);
        }

        if (tolower(filename[fi]) == tolower(pattern[pi])) {
            return matchHelper(fi + 1, pi + 1);
        }

        return false;
        };

    return matchHelper(0, 0);
}

static const char* g_executableExtensions[] = { ".exe", ".com", ".bat", ".cmd", ".ps1", "" };

std::string FindExecutableInPath(const std::string& exeName) {
    if (exeName.empty()) return "";

    bool hasPathSeparator = (exeName.find('\\') != std::string::npos ||
        exeName.find('/') != std::string::npos);

    if (hasPathSeparator) {
        DWORD attrs = GetFileAttributesA(exeName.c_str());
        if (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY)) {
            return exeName;
        }
    }

    char curDir[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, curDir);

    for (int i = 0; g_executableExtensions[i] != nullptr; i++) {
        std::string curPath = std::string(curDir) + "\\" + exeName + g_executableExtensions[i];
        DWORD attrs = GetFileAttributesA(curPath.c_str());
        if (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY)) {
            return curPath;
        }
    }

    char* pathEnv = nullptr;
    size_t pathLen = 0;
    _dupenv_s(&pathEnv, &pathLen, "PATH");
    if (!pathEnv || pathLen == 0) {
        free(pathEnv);
        return exeName;
    }

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
            for (int i = 0; g_executableExtensions[i] != nullptr; i++) {
                std::string fullPath = dir + "\\" + exeName + g_executableExtensions[i];
                DWORD attrs = GetFileAttributesA(fullPath.c_str());
                if (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY)) {
                    return fullPath;
                }
            }
        }

        start = end + 1;
    }

    return exeName;
}

bool IsBuiltinCommand(const std::string& cmd) {
    static const char* builtins[] = {
        "cd", "cd~", "~", "dir", "type", "copy", "move", "del", "erase",
        "mkdir", "md", "rmdir", "rd", "ren", "rename", "tree", "attrib",
        "mklink", "xcopy", "robocopy", "replace",
        "echo", "cls", "clear", "color", "title", "prompt", "more",
        "set", "setlocal", "endlocal", "path", "dpath", "pushd", "popd",
        "taskkill", "tasklist", "ipconfig", "whoami", "hostname", "vol", "ver",
        "date", "time", "chcp", "ctty", "break", "verify",
        "for", "if", "goto", "call", "shift", "pause", "exit",
        "assoc", "ftype",
        "find", "sleep", "beep", "start", "home", "history",
        "hex", "dec", "bin", "_calc",
        "Jiefangcheng11", "Jiefangcheng12",
        "Jiefangcheng21", "Jiefangcheng31",
        "st", "JH_Shutdown",
        "_Internet_", "_solveproblems_", "_SH_", "friend", "friends",
        "dumpbin",
        "ping",
        nullptr
    };

    std::string lower = cmd;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    for (int i = 0; builtins[i] != nullptr; i++) {
        if (lower == builtins[i]) return true;
        std::string withSpace = std::string(builtins[i]) + " ";
        if (lower.find(withSpace) == 0) return true;
        if (std::string(builtins[i]) == "st" && lower.find("st ") == 0) return true;
    }
    return false;
}

void SetLastErrorCode(DWORD code) {
    g_lastErrorCode = code;
    char buf[32];
    sprintf_s(buf, "%u", code);
    SetEnvironmentVariableA("ERRORLEVEL", buf);
    SetEnvironmentVariableA("errorlevel", buf);
    SetEnvironmentVariableA("ErrorLevel", buf);
}

DWORD GetLastErrorCode() {
    return g_lastErrorCode;
}

bool IsLastCommandSuccessful() {
    return g_lastErrorCode == 0;
}

DWORD ExecuteAndGetExitCode(const std::string& cmd) {
    if (HandleBuiltinCommand(cmd, false)) {
        SetLastErrorCode(0);
        return 0;
    }

    std::string expandedCmd = ExpandEnvironmentVars(cmd);

    std::string cmdName = expandedCmd;
    size_t spacePos = cmdName.find(' ');
    if (spacePos != std::string::npos) {
        cmdName = cmdName.substr(0, spacePos);
    }

    std::string fullPath = FindExecutableInPath(cmdName);
    if (fullPath != cmdName) {
        std::string newCmd = fullPath;
        if (spacePos != std::string::npos) {
            newCmd += expandedCmd.substr(spacePos);
        }
        expandedCmd = newCmd;
    }

    std::wstring wExpanded = U82W_Path(expandedCmd);
    std::vector<wchar_t> cmdBuf(wExpanded.begin(), wExpanded.end());
    cmdBuf.push_back(L'\0');

    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi = { 0 };
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

    DWORD exitCode = 1;

    if (CreateProcessW(NULL, cmdBuf.data(), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        GetExitCodeProcess(pi.hProcess, &exitCode);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    else {
        std::wstring wPsCmd = U82W_Path(expandedCmd);
        std::vector<wchar_t> psBuf(wPsCmd.begin(), wPsCmd.end());
        psBuf.push_back(L'\0');

        STARTUPINFOW si2 = { sizeof(si2) };
        PROCESS_INFORMATION pi2 = { 0 };
        si2.dwFlags = STARTF_USESTDHANDLES;
        si2.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
        si2.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
        si2.hStdError = GetStdHandle(STD_ERROR_HANDLE);

        if (CreateProcessW(NULL, psBuf.data(), NULL, NULL, TRUE, 0, NULL, NULL, &si2, &pi2)) {
            WaitForSingleObject(pi2.hProcess, INFINITE);
            GetExitCodeProcess(pi2.hProcess, &exitCode);
            CloseHandle(pi2.hProcess);
            CloseHandle(pi2.hThread);
        }
    }

    SetLastErrorCode(exitCode);
    return exitCode;
}

bool HandleGetService(const std::string& cmd, bool silentMode) {
    SC_HANDLE hSCM = OpenSCManagerA(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
    if (!hSCM) {
        std::cout << "无法打开服务管理器\n";
        return true;
    }

    DWORD bytesNeeded = 0;
    DWORD servicesReturned = 0;
    DWORD resumeHandle = 0;

    EnumServicesStatusA(hSCM, SERVICE_WIN32, SERVICE_STATE_ALL,
        NULL, 0, &bytesNeeded, &servicesReturned, &resumeHandle);

    if (bytesNeeded == 0) {
        CloseServiceHandle(hSCM);
        return true;
    }

    std::vector<BYTE> buffer(bytesNeeded);
    ENUM_SERVICE_STATUSA* services = (ENUM_SERVICE_STATUSA*)buffer.data();

    if (!EnumServicesStatusA(hSCM, SERVICE_WIN32, SERVICE_STATE_ALL,
        services, bytesNeeded, &bytesNeeded, &servicesReturned, &resumeHandle)) {
        CloseServiceHandle(hSCM);
        return true;
    }

    std::cout << "\n服务状态:\n";
    std::cout << "----------------------------------------\n";

    int running = 0, stopped = 0;
    for (DWORD i = 0; i < servicesReturned; i++) {
        std::string status;
        if (services[i].ServiceStatus.dwCurrentState == SERVICE_RUNNING) {
            status = "Running";
            running++;
        }
        else if (services[i].ServiceStatus.dwCurrentState == SERVICE_STOPPED) {
            status = "Stopped";
            stopped++;
        }
        else {
            status = "Other";
        }

        std::cout << "  " << std::setw(10) << status
            << "     " << services[i].lpDisplayName << "\n";
    }

    std::cout << "----------------------------------------\n";
    std::cout << "共 " << servicesReturned << " 个服务"
        << " (Running: " << running << ", Stopped: " << stopped << ")\n\n";

    CloseServiceHandle(hSCM);
    return true;
}

void ShowExports(LPVOID pBase, PIMAGE_NT_HEADERS pNtHeaders) {
    DWORD exportRVA = 0, exportSize = 0;
    if (!GetDataDirectory(pNtHeaders, IMAGE_DIRECTORY_ENTRY_EXPORT, exportRVA, exportSize)) {
        std::cout << "  无法读取导出目录\n";
        return;
    }

    if (exportRVA == 0 || exportSize == 0) {
        std::cout << "  无导出表\n";
        return;
    }

    if (!IsRvaValid(pBase, pNtHeaders, exportRVA)) {
        std::cout << "  警告：导出目录 RVA (0x" << std::hex << exportRVA << std::dec
            << ") 不在任何节区内\n";
        std::cout << "        文件可能损坏、加壳或格式异常，跳过导出表解析\n";
        return;
    }

    // ---------- 步骤 3：获取导出目录结构 ----------
    PIMAGE_EXPORT_DIRECTORY pExport =
        (PIMAGE_EXPORT_DIRECTORY)GetPtrFromRva(pBase, pNtHeaders, exportRVA);
    if (!pExport) {
        std::cout << "  警告：无法读取导出目录结构\n";
        return;
    }

    const DWORD MAX_REASONABLE_FUNCS = 1048576;
    const DWORD MAX_REASONABLE_NAMES = 1048576;

    if (pExport->NumberOfFunctions == 0) {
        std::cout << "  导出表为空（NumberOfFunctions = 0）\n";
        return;
    }
    if (pExport->NumberOfFunctions > MAX_REASONABLE_FUNCS) {
        std::cout << "  警告：导出函数数量异常（" << pExport->NumberOfFunctions
            << "，超过合理上限 " << MAX_REASONABLE_FUNCS << "）\n";
        std::cout << "        文件可能损坏、加壳或恶意构造，跳过导出表解析\n";
        return;
    }
    if (pExport->NumberOfNames > MAX_REASONABLE_NAMES) {
        std::cout << "  警告：导出名称数量异常（" << pExport->NumberOfNames
            << "，超过合理上限）\n";
        std::cout << "        文件可能损坏、加壳或恶意构造，跳过导出表解析\n";
        return;
    }
    if (pExport->NumberOfNames > pExport->NumberOfFunctions) {
        std::cout << "  警告：导出名称数量（" << pExport->NumberOfNames
            << "）大于函数数量（" << pExport->NumberOfFunctions << "）\n";
        std::cout << "        文件格式异常，跳过导出表解析\n";
        return;
    }

    if (!IsRvaValid(pBase, pNtHeaders, pExport->AddressOfFunctions)) {
        std::cout << "  警告：函数地址表 RVA 无效\n";
        return;
    }
    if (pExport->NumberOfNames > 0) {
        if (!IsRvaValid(pBase, pNtHeaders, pExport->AddressOfNames)) {
            std::cout << "  警告：函数名称表 RVA 无效\n";
            return;
        }
        if (!IsRvaValid(pBase, pNtHeaders, pExport->AddressOfNameOrdinals)) {
            std::cout << "  警告：函数序号表 RVA 无效\n";
            return;
        }
    }

    DWORD* pFunctions = (DWORD*)GetPtrFromRva(pBase, pNtHeaders, pExport->AddressOfFunctions);
    DWORD* pNames = nullptr;
    WORD* pOrdinals = nullptr;

    if (pExport->NumberOfNames > 0) {
        pNames = (DWORD*)GetPtrFromRva(pBase, pNtHeaders, pExport->AddressOfNames);
        pOrdinals = (WORD*)GetPtrFromRva(pBase, pNtHeaders, pExport->AddressOfNameOrdinals);
    }

    if (!pFunctions) {
        std::cout << "  警告：无法读取函数地址表\n";
        return;
    }
    if (pExport->NumberOfNames > 0 && (!pNames || !pOrdinals)) {
        std::cout << "  警告：无法读取函数名称表或序号表\n";
        return;
    }

    std::cout << "\n  导出表 (共 " << pExport->NumberOfFunctions << " 个函数";
    if (pExport->NumberOfNames > 0) {
        std::cout << "，其中 " << pExport->NumberOfNames << " 个有名";
    }
    std::cout << "):\n";
    std::cout << "  ----------------------------------------\n";

    int exportedCount = 0;
    int skippedCount = 0;

    for (DWORD i = 0; i < pExport->NumberOfFunctions; i++) {
        DWORD funcRVA = pFunctions[i];
        if (funcRVA == 0) continue;

        if (!IsRvaValid(pBase, pNtHeaders, funcRVA)) {
            skippedCount++;
            continue;
        }

        std::string funcName;
        WORD ordinal = (WORD)(pExport->Base + i);

        for (DWORD j = 0; j < pExport->NumberOfNames; j++) {
            if (pOrdinals[j] == i) {
                if (IsRvaValid(pBase, pNtHeaders, pNames[j])) {
                    SafeReadString(pBase, pNtHeaders, pNames[j], funcName);
                }
                break;
            }
        }

        if (funcName.empty()) {
            std::cout << "    [Ordinal] " << ordinal
                << "  RVA: 0x" << std::hex << funcRVA << std::dec << "\n";
        }
        else {
            std::cout << "    " << funcName
                << "  (Ordinal: " << ordinal
                << ", RVA: 0x" << std::hex << funcRVA << std::dec << ")\n";
        }
        exportedCount++;
    }

    std::cout << "  ----------------------------------------\n";
    std::cout << "  共导出 " << exportedCount << " 个函数";
    if (skippedCount > 0) {
        std::cout << "（跳过 " << skippedCount << " 个非法项）";
    }
    std::cout << "\n";

    std::string dllName;
    if (SafeReadString(pBase, pNtHeaders, pExport->Name, dllName)) {
        std::cout << "  DLL 名称: " << dllName << "\n";
    }
}

bool IsRvaValid(LPVOID pBase, PIMAGE_NT_HEADERS pNtHeaders, DWORD rva) {
    if (rva == 0) return false;
    if (!pBase || !pNtHeaders) return false;

    PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION(pNtHeaders);
    WORD numSections = pNtHeaders->FileHeader.NumberOfSections;

    for (WORD i = 0; i < numSections; i++) {
        DWORD start = pSection[i].VirtualAddress;
        DWORD size = pSection[i].Misc.VirtualSize;
        DWORD end = start + size;
        if (end < start) end = 0xFFFFFFFF;

        if (rva >= start && rva < end) {
            return true;
        }
    }
    return false;
}

void ShowImports(LPVOID pBase, PIMAGE_NT_HEADERS pNtHeaders) {
    DWORD importRVA = 0, importSize = 0;
    if (!GetDataDirectory(pNtHeaders, IMAGE_DIRECTORY_ENTRY_IMPORT, importRVA, importSize)) {
        std::cout << "  无法读取导入目录\n";
        return;
    }

    if (importRVA == 0) {
        std::cout << "  无导入表\n";
        return;
    }

    if (!IsRvaValid(pBase, pNtHeaders, importRVA)) {
        std::cout << "  警告：导入表 RVA (0x" << std::hex << importRVA << std::dec
            << ") 不在任何节区内\n";
        std::cout << "        文件可能损坏、加壳或格式异常，跳过导入表解析\n";
        return;
    }

    PIMAGE_IMPORT_DESCRIPTOR pImport =
        (PIMAGE_IMPORT_DESCRIPTOR)GetPtrFromRva(pBase, pNtHeaders, importRVA);
    if (!pImport) {
        std::cout << "  警告：无法读取导入表\n";
        return;
    }

    std::cout << "\n  导入表:\n";
    std::cout << "  ----------------------------------------\n";

    int dllCount = 0;
    int funcCount = 0;
    int skippedDlls = 0;
    const int MAX_DLLS = 4096;
    const int MAX_FUNCS = 1048576;

    while (pImport->Name != 0 && dllCount + skippedDlls < MAX_DLLS) {
        std::string dllName;
        if (!SafeReadString(pBase, pNtHeaders, pImport->Name, dllName)) {
            std::cout << "  [警告] 遇到无效的 DLL 名称 RVA (0x"
                << std::hex << pImport->Name << std::dec << ")，停止解析\n";
            break;
        }

        std::cout << "  " << dllName << ":\n";

        DWORD thunkRVA = pImport->OriginalFirstThunk ? pImport->OriginalFirstThunk
            : pImport->FirstThunk;
        if (!IsRvaValid(pBase, pNtHeaders, thunkRVA)) {
            std::cout << "      (跳过：thunk 表 RVA 无效)\n";
            pImport++;
            skippedDlls++;
            continue;
        }

        PIMAGE_THUNK_DATA pThunk =
            (PIMAGE_THUNK_DATA)GetPtrFromRva(pBase, pNtHeaders, thunkRVA);

        if (pThunk) {
            int localFuncCount = 0;
            while (pThunk->u1.AddressOfData != 0 && localFuncCount < MAX_FUNCS) {
                if (!(pThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG)) {
                    std::string importName;
                    if (IsRvaValid(pBase, pNtHeaders, pThunk->u1.AddressOfData)) {
                        PIMAGE_IMPORT_BY_NAME pImportName = (PIMAGE_IMPORT_BY_NAME)
                            GetPtrFromRva(pBase, pNtHeaders, pThunk->u1.AddressOfData);
                        if (pImportName) {
                            std::string name;
                            DWORD nameRVA = pThunk->u1.AddressOfData + 2;  // Hint 占 2 字节
                            if (SafeReadString(pBase, pNtHeaders, nameRVA, name, 256)) {
                                std::cout << "      " << name << "\n";
                                funcCount++;
                                localFuncCount++;
                            }
                        }
                    }
                }
                else {
                    std::cout << "      [Ordinal] "
                        << (pThunk->u1.Ordinal & ~IMAGE_ORDINAL_FLAG) << "\n";
                    funcCount++;
                    localFuncCount++;
                }
                pThunk++;
            }
        }
        pImport++;
        dllCount++;
    }

    std::cout << "  ----------------------------------------\n";
    std::cout << "  共 " << dllCount << " 个 DLL, " << funcCount << " 个导入函数\n";
}

void ShowSections(PIMAGE_NT_HEADERS pNtHeaders) {
    std::cout << "\n  节区信息:\n";
    std::cout << "  ----------------------------------------\n";
    std::cout << "  名称     RVA      大小      文件偏移   属性\n";
    std::cout << "  ----------------------------------------\n";

    WORD numSections = pNtHeaders->FileHeader.NumberOfSections;
    if (numSections == 0 || numSections > 96) {
        std::cout << "  警告：节区数量异常（" << numSections << "）\n";
        return;
    }

    PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION(pNtHeaders);
    for (WORD i = 0; i < numSections; i++) {
        char name[9] = { 0 };
        memcpy(name, pSection[i].Name, 8);

        std::string flags;
        if (pSection[i].Characteristics & IMAGE_SCN_MEM_EXECUTE) flags += "X";
        if (pSection[i].Characteristics & IMAGE_SCN_MEM_READ)    flags += "R";
        if (pSection[i].Characteristics & IMAGE_SCN_MEM_WRITE)   flags += "W";

        std::cout << "  " << std::setw(8) << name
            << " 0x" << std::setw(6) << std::hex << pSection[i].VirtualAddress
            << " 0x" << std::setw(6) << pSection[i].Misc.VirtualSize
            << " 0x" << std::setw(6) << pSection[i].PointerToRawData
            << std::dec << "  " << flags << "\n";
    }

    std::cout << "  ----------------------------------------\n";
    std::cout << "  共 " << numSections << " 个节区\n";
}

void ShowHeaders(PIMAGE_NT_HEADERS pNtHeaders) {
    std::cout << "\n  PE 头信息:\n";
    std::cout << "  ----------------------------------------\n";

    std::cout << "  文件头:\n";
    std::cout << "    Machine: 0x" << std::hex << pNtHeaders->FileHeader.Machine << std::dec;
    if (pNtHeaders->FileHeader.Machine == IMAGE_FILE_MACHINE_I386) {
        std::cout << " (x86)";
    }
    else if (pNtHeaders->FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64) {
        std::cout << " (x64)";
    }
    else if (pNtHeaders->FileHeader.Machine == IMAGE_FILE_MACHINE_ARM64) {
        std::cout << " (ARM64)";
    }
    else if (pNtHeaders->FileHeader.Machine == IMAGE_FILE_MACHINE_ARM) {
        std::cout << " (ARM)";
    }
    std::cout << "\n";

    std::cout << "    NumberOfSections: " << pNtHeaders->FileHeader.NumberOfSections << "\n";
    std::cout << "    TimeDateStamp: 0x" << std::hex << pNtHeaders->FileHeader.TimeDateStamp
        << std::dec << "\n";
    std::cout << "    Characteristics: 0x" << std::hex
        << pNtHeaders->FileHeader.Characteristics << std::dec << "\n";

    WORD magic = pNtHeaders->OptionalHeader.Magic;

    std::cout << "\n  可选头:\n";
    std::cout << "    Magic: 0x" << std::hex << magic << std::dec;
    if (magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) std::cout << " (PE32)";
    else if (magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) std::cout << " (PE32+)";
    else std::cout << " (未知格式)";
    std::cout << "\n";

    if (magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
        PIMAGE_OPTIONAL_HEADER32 pOpt =
            (PIMAGE_OPTIONAL_HEADER32)&pNtHeaders->OptionalHeader;

        std::cout << "    ImageBase: 0x" << std::hex << pOpt->ImageBase << std::dec << "\n";
        std::cout << "    EntryPoint (RVA): 0x" << std::hex << pOpt->AddressOfEntryPoint
            << std::dec << "\n";
        std::cout << "    SizeOfImage: " << pOpt->SizeOfImage << " 字节\n";
        std::cout << "    SizeOfHeaders: " << pOpt->SizeOfHeaders << " 字节\n";

        std::cout << "    Subsystem: ";
        switch (pOpt->Subsystem) {
        case IMAGE_SUBSYSTEM_WINDOWS_GUI:     std::cout << "Windows GUI\n"; break;
        case IMAGE_SUBSYSTEM_WINDOWS_CUI:     std::cout << "Windows Console\n"; break;
        case IMAGE_SUBSYSTEM_NATIVE:          std::cout << "Native Driver\n"; break;
        case IMAGE_SUBSYSTEM_EFI_APPLICATION: std::cout << "EFI Application\n"; break;
        default: std::cout << "(" << pOpt->Subsystem << ")\n"; break;
        }

        std::cout << "    DLL Characteristics: 0x" << std::hex
            << pOpt->DllCharacteristics << std::dec << "\n";
        std::cout << "    Stack Reserve: " << pOpt->SizeOfStackReserve << " 字节\n";
        std::cout << "    Stack Commit: " << pOpt->SizeOfStackCommit << " 字节\n";
        std::cout << "    Heap Reserve: " << pOpt->SizeOfHeapReserve << " 字节\n";
        std::cout << "    Heap Commit: " << pOpt->SizeOfHeapCommit << " 字节\n";
        std::cout << "    SectionAlignment: 0x" << std::hex << pOpt->SectionAlignment
            << std::dec << "\n";
        std::cout << "    FileAlignment: 0x" << std::hex << pOpt->FileAlignment
            << std::dec << "\n";
        std::cout << "    OS Version: " << pOpt->MajorOperatingSystemVersion << "."
            << pOpt->MinorOperatingSystemVersion << "\n";
        std::cout << "    Subsystem Version: " << pOpt->MajorSubsystemVersion << "."
            << pOpt->MinorSubsystemVersion << "\n";
        std::cout << "    CheckSum: 0x" << std::hex << pOpt->CheckSum << std::dec << "\n";
        std::cout << "    NumberOfRvaAndSizes: " << pOpt->NumberOfRvaAndSizes << "\n";
    }
    else if (magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
        PIMAGE_OPTIONAL_HEADER64 pOpt =
            (PIMAGE_OPTIONAL_HEADER64)&pNtHeaders->OptionalHeader;

        std::cout << "    ImageBase: 0x" << std::hex << pOpt->ImageBase << std::dec << "\n";
        std::cout << "    EntryPoint (RVA): 0x" << std::hex << pOpt->AddressOfEntryPoint
            << std::dec << "\n";
        std::cout << "    SizeOfImage: " << pOpt->SizeOfImage << " 字节\n";
        std::cout << "    SizeOfHeaders: " << pOpt->SizeOfHeaders << " 字节\n";

        std::cout << "    Subsystem: ";
        switch (pOpt->Subsystem) {
        case IMAGE_SUBSYSTEM_WINDOWS_GUI:     std::cout << "Windows GUI\n"; break;
        case IMAGE_SUBSYSTEM_WINDOWS_CUI:     std::cout << "Windows Console\n"; break;
        case IMAGE_SUBSYSTEM_NATIVE:          std::cout << "Native Driver\n"; break;
        case IMAGE_SUBSYSTEM_EFI_APPLICATION: std::cout << "EFI Application\n"; break;
        default: std::cout << "(" << pOpt->Subsystem << ")\n"; break;
        }

        std::cout << "    DLL Characteristics: 0x" << std::hex
            << pOpt->DllCharacteristics << std::dec << "\n";
        std::cout << "    Stack Reserve: " << pOpt->SizeOfStackReserve << " 字节\n";
        std::cout << "    Stack Commit: " << pOpt->SizeOfStackCommit << " 字节\n";
        std::cout << "    Heap Reserve: " << pOpt->SizeOfHeapReserve << " 字节\n";
        std::cout << "    Heap Commit: " << pOpt->SizeOfHeapCommit << " 字节\n";
        std::cout << "    SectionAlignment: 0x" << std::hex << pOpt->SectionAlignment
            << std::dec << "\n";
        std::cout << "    FileAlignment: 0x" << std::hex << pOpt->FileAlignment
            << std::dec << "\n";
        std::cout << "    OS Version: " << pOpt->MajorOperatingSystemVersion << "."
            << pOpt->MinorOperatingSystemVersion << "\n";
        std::cout << "    Subsystem Version: " << pOpt->MajorSubsystemVersion << "."
            << pOpt->MinorSubsystemVersion << "\n";
        std::cout << "    CheckSum: 0x" << std::hex << pOpt->CheckSum << std::dec << "\n";
        std::cout << "    NumberOfRvaAndSizes: " << pOpt->NumberOfRvaAndSizes << "\n";
    }
    else {
        std::cout << "    警告：未知的 Optional Header 格式（Magic = 0x"
            << std::hex << magic << std::dec << "）\n";
        std::cout << "          可能不是标准 PE 文件，跳过可选头解析\n";
    }

    std::cout << "  ----------------------------------------\n";
}

void ShowResources(LPVOID pBase, PIMAGE_NT_HEADERS pNtHeaders) {
    DWORD resourceRVA = 0, resourceSize = 0;
    if (!GetDataDirectory(pNtHeaders, IMAGE_DIRECTORY_ENTRY_RESOURCE, resourceRVA, resourceSize)) {
        std::cout << "  无法读取资源目录\n";
        return;
    }

    if (resourceRVA == 0) {
        std::cout << "  无资源\n";
        return;
    }

    if (!IsRvaValid(pBase, pNtHeaders, resourceRVA)) {
        std::cout << "  警告：资源目录 RVA (0x" << std::hex << resourceRVA << std::dec
            << ") 不在任何节区内\n";
        std::cout << "        文件可能损坏、加壳或格式异常，跳过资源解析\n";
        return;
    }

    PIMAGE_RESOURCE_DIRECTORY pRootDir =
        (PIMAGE_RESOURCE_DIRECTORY)GetPtrFromRva(pBase, pNtHeaders, resourceRVA);
    if (!pRootDir) {
        std::cout << "  警告：无法读取资源目录\n";
        return;
    }

    std::cout << "\n  资源信息:\n";
    std::cout << "  ----------------------------------------\n";

    int resourceCount = 0;
    const int MAX_RESOURCES = 10000;

    std::function<void(PIMAGE_RESOURCE_DIRECTORY, int, const std::string&, int&)> TraverseResources =
        [&](PIMAGE_RESOURCE_DIRECTORY pDir, int level,
            const std::string& indent, int& count) {
                if (!pDir || count >= MAX_RESOURCES) return;

                DWORD entryCount = pDir->NumberOfNamedEntries + pDir->NumberOfIdEntries;
                if (entryCount > 4096) {
                    std::cout << indent << "  [警告] 资源目录项数量异常（" << entryCount << "），跳过\n";
                    return;
                }

                PIMAGE_RESOURCE_DIRECTORY_ENTRY pEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(pDir + 1);
                for (DWORD i = 0; i < entryCount && count < MAX_RESOURCES; i++) {
                    std::string typeName;

                    if (pEntry[i].NameIsString) {
                        if (IsRvaValid(pBase, pNtHeaders, resourceRVA + pEntry[i].NameOffset)) {
                            PIMAGE_RESOURCE_DIR_STRING_U pNameStr = (PIMAGE_RESOURCE_DIR_STRING_U)
                                GetPtrFromRva(pBase, pNtHeaders, resourceRVA + pEntry[i].NameOffset);
                            if (pNameStr) {
                                char nameBuf[256] = { 0 };
                                int len = (pNameStr->Length < 128) ? pNameStr->Length : 128;
                                WideCharToMultiByte(CP_UTF8, 0, pNameStr->NameString, len,
                                    nameBuf, sizeof(nameBuf) - 1, NULL, NULL);
                                typeName = nameBuf;
                            }
                        }
                    }
                    else {
                        DWORD id = pEntry[i].Id;
                        switch (id) {
                        case 1:  typeName = "CURSOR";       break;
                        case 2:  typeName = "BITMAP";       break;
                        case 3:  typeName = "ICON";         break;
                        case 4:  typeName = "MENU";         break;
                        case 5:  typeName = "DIALOG";       break;
                        case 6:  typeName = "STRING";       break;
                        case 7:  typeName = "FONTDIR";      break;
                        case 8:  typeName = "FONT";         break;
                        case 9:  typeName = "ACCELERATOR";  break;
                        case 10: typeName = "RCDATA";       break;
                        case 11: typeName = "MESSAGETABLE"; break;
                        case 12: typeName = "GROUP_CURSOR"; break;
                        case 14: typeName = "GROUP_ICON";   break;
                        case 16: typeName = "VERSION";      break;
                        case 24: typeName = "MANIFEST";     break;
                        default: typeName = "ID_" + std::to_string(id); break;
                        }
                    }

                    if (pEntry[i].DataIsDirectory) {
                        DWORD subDirRVA = resourceRVA + pEntry[i].OffsetToDirectory;
                        if (IsRvaValid(pBase, pNtHeaders, subDirRVA)) {
                            PIMAGE_RESOURCE_DIRECTORY pSubDir =
                                (PIMAGE_RESOURCE_DIRECTORY)GetPtrFromRva(pBase, pNtHeaders, subDirRVA);
                            TraverseResources(pSubDir, level + 1, indent + "  ", count);
                        }
                    }
                    else {
                        DWORD dataRVA = resourceRVA + pEntry[i].OffsetToData;
                        if (IsRvaValid(pBase, pNtHeaders, dataRVA)) {
                            PIMAGE_RESOURCE_DATA_ENTRY pData =
                                (PIMAGE_RESOURCE_DATA_ENTRY)GetPtrFromRva(pBase, pNtHeaders, dataRVA);
                            if (pData) {
                                std::cout << indent << "  " << typeName
                                    << "  大小: " << pData->Size << " 字节"
                                    << "  RVA: 0x" << std::hex << pData->OffsetToData << std::dec
                                    << "\n";
                                count++;
                            }
                        }
                    }
                }
        };

    TraverseResources(pRootDir, 0, "", resourceCount);

    std::cout << "  ----------------------------------------\n";
    std::cout << "  共 " << resourceCount << " 个资源\n";
}

void ShowDependencies(const std::string& filePath, const std::string& option = "/dependents") {
    
    std::wstring wFilePath;
    {
        int len = MultiByteToWideChar(CP_UTF8, 0, filePath.c_str(), -1, NULL, 0);
        if (len <= 0) {
            std::cout << "错误：文件路径转换失败\n\n";
            return;
        }
        wFilePath.resize(len - 1);
        MultiByteToWideChar(CP_UTF8, 0, filePath.c_str(), -1, &wFilePath[0], len);
    }

    HANDLE hFile = CreateFileW(wFilePath.c_str(), GENERIC_READ, FILE_SHARE_READ,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        DWORD err = GetLastError();
        std::cout << "无法打开文件: " << filePath
            << " (错误码: " << err << ")\n";
        switch (err) {
        case ERROR_FILE_NOT_FOUND:
            std::cout << "  原因：文件不存在\n\n";
            break;
        case ERROR_ACCESS_DENIED:
            std::cout << "  原因：访问被拒绝（可能被占用或无权限）\n\n";
            break;
        case ERROR_PATH_NOT_FOUND:
            std::cout << "  原因：路径不存在\n\n";
            break;
        default:
            std::cout << "\n";
            break;
        }
        return;
    }

    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile, &fileSize)) {
        std::cout << "错误：无法获取文件大小\n\n";
        CloseHandle(hFile);
        return;
    }

    if (fileSize.QuadPart < 64) {
        std::cout << "错误：文件太小（" << fileSize.QuadPart
            << " 字节），不是有效的 PE 文件\n\n";
        CloseHandle(hFile);
        return;
    }

    if (fileSize.QuadPart > 0x7FFFFFFF) {
        std::cout << "错误：文件过大（" << fileSize.QuadPart
            << " 字节），超过 2GB 上限\n\n";
        CloseHandle(hFile);
        return;
    }

    HANDLE hMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (hMapping == NULL) {
        std::cout << "错误：创建文件映射失败（错误码: " << GetLastError() << "）\n\n";
        CloseHandle(hFile);
        return;
    }

    LPVOID pBase = MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
    if (pBase == NULL) {
        std::cout << "错误：映射文件到内存失败（错误码: " << GetLastError() << "）\n\n";
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return;
    }

    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pBase;

    if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        std::cout << "无效的 PE 文件：DOS 签名错误（不是 MZ 开头）\n\n";
        UnmapViewOfFile(pBase);
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return;
    }

    if (pDosHeader->e_lfanew <= 0) {
        std::cout << "无效的 PE 文件：PE 头偏移非法（e_lfanew <= 0）\n\n";
        UnmapViewOfFile(pBase);
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return;
    }

    if ((DWORD)pDosHeader->e_lfanew > 0x10000000) {
        std::cout << "无效的 PE 文件：PE 头偏移过大（e_lfanew = 0x"
            << std::hex << pDosHeader->e_lfanew << std::dec << "）\n\n";
        UnmapViewOfFile(pBase);
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return;
    }

    if ((DWORD)pDosHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS) > (DWORD)fileSize.QuadPart) {
        std::cout << "无效的 PE 文件：PE 头超出文件范围\n\n";
        UnmapViewOfFile(pBase);
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return;
    }

    PIMAGE_NT_HEADERS pNtHeaders =
        (PIMAGE_NT_HEADERS)((BYTE*)pBase + pDosHeader->e_lfanew);

    if (pNtHeaders->Signature != IMAGE_NT_SIGNATURE) {
        std::cout << "无效的 PE 文件：NT 签名错误（不是 PE\\0\\0 开头）\n\n";
        UnmapViewOfFile(pBase);
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return;
    }

    if (pNtHeaders->FileHeader.NumberOfSections == 0) {
        std::cout << "无效的 PE 文件：节区数量为 0\n\n";
        UnmapViewOfFile(pBase);
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return;
    }
    if (pNtHeaders->FileHeader.NumberOfSections > 96) {
        std::cout << "无效的 PE 文件：节区数量异常（"
            << pNtHeaders->FileHeader.NumberOfSections << "，超过 96）\n\n";
        UnmapViewOfFile(pBase);
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return;
    }

    WORD magic = pNtHeaders->OptionalHeader.Magic;
    if (magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC &&
        magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
        std::cout << "无效的 PE 文件：Optional Header Magic 异常（0x"
            << std::hex << magic << std::dec << "）\n\n";
        UnmapViewOfFile(pBase);
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return;
    }

    std::cout << "\n文件: " << filePath << "\n";
    std::cout << "========================================\n";

    ParsePEFileSafe(pBase, pNtHeaders, option.c_str());

    std::cout << "========================================\n\n";

    UnmapViewOfFile(pBase);
    CloseHandle(hMapping);
    CloseHandle(hFile);
}

std::string GetZJHCMDConfigPath() {
    static std::string configPath;

    if (!configPath.empty()) {
        return configPath;
    }

    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);

    std::string exeDir = exePath;
    size_t lastSlash = exeDir.find_last_of("\\/");
    if (lastSlash != std::string::npos) {
        exeDir = exeDir.substr(0, lastSlash + 1);
    }

    configPath = exeDir + ".zjhcmd\\";

    CreateDirectoryA(configPath.c_str(), NULL);

    return configPath;
}

std::string ReadConfigFile(const std::string& fullPath, const std::string& defaultValue) {
    std::ifstream file(fullPath);
    if (!file.is_open()) {
        return defaultValue;
    }

    std::string content;
    std::getline(file, content);
    file.close();

    if (content.empty()) {
        return defaultValue;
    }

    const std::string whitespace = " \t\r\n";
    size_t start = content.find_first_not_of(whitespace);
    if (start == std::string::npos) {
        return defaultValue;
    }
    size_t end = content.find_last_not_of(whitespace);
    content = content.substr(start, end - start + 1);

    if (content.size() >= 3 &&
        (unsigned char)content[0] == 0xEF &&
        (unsigned char)content[1] == 0xBB &&
        (unsigned char)content[2] == 0xBF) {
        content = content.substr(3);
    }

    if (content.empty()) {
        return defaultValue;
    }

    return content;
}

void EnsureConfigDirectory() {
    std::string configPath = GetZJHCMDConfigPath();
    CreateDirectoryA(configPath.c_str(), NULL);
}

std::string GetPromptTips() {
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);

    std::string exeDir = exePath;
    size_t lastSlash = exeDir.find_last_of("\\/");
    if (lastSlash != std::string::npos) {
        exeDir = exeDir.substr(0, lastSlash + 1);
    }

    std::string configPath = exeDir + ".zjhcmd\\";
    std::string fullPath = configPath + "ZJHCMDTIPS";

    std::ifstream file(fullPath);
    if (!file.is_open()) {
        char currentDir[MAX_PATH];
        GetCurrentDirectoryA(MAX_PATH, currentDir);
        return std::string(currentDir);
    }

    std::string content;
    std::getline(file, content);
    file.close();

    size_t start = content.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }
    size_t end = content.find_last_not_of(" \t\r\n");
    content = content.substr(start, end - start + 1);

    if (content.size() >= 3 &&
        (unsigned char)content[0] == 0xEF &&
        (unsigned char)content[1] == 0xBB &&
        (unsigned char)content[2] == 0xBF) {
        content = content.substr(3);
    }

    return content;
}

std::string GenerateAsciiArt(const std::string& text) {
    std::string result = "\n";
    result += "  ╔══════════════════════════════════════════╗\n";
    result += "  ║  " + text + "\n";
    result += "  ╚══════════════════════════════════════════╝\n";
    result += "\n";
    return result;
}

const char* g_fortunes[] = {
    "Windows 1.0 发布于 1985 年，比 macOS 晚了整整一年",
    "第一行代码是 'Hello World'，由 Brian Kernighan 于 1972 年创造",
    "Linux 内核最初由 Linus Torvalds 于 1991 年发布，只有 1 万行代码",
    "ZJHCMD v34 拥有 132 个内置命令，比系统 CMD 多 2.5 倍",
    "世界上第一个计算机病毒 'Creeper' 于 1971 年诞生，只显示一段话",
    "Python 的命名来自 BBC 喜剧 'Monty Python'，不是蟒蛇",
    "Git 由 Linus Torvalds 于 2005 年开发，用于管理 Linux 内核源码",
    "世界上第一个程序员是 Ada Lovelace，生于 1815 年",
    "命令行已经存在了 50 多年，至今依然是开发者的最爱",
    "世界上 90% 的服务器运行 Linux 或 Unix 系统",
    "ZJHCMD 的数学引擎可以在 10ms 内计算三角函数、对数和虚数",
    "Windows 10/11 仍然保留着 MS-DOS 的底层兼容代码",
    "PowerShell 在 512MB 内存的电脑上根本跑不动，但 ZJHCMD 只需 0.5MB",
    "世界上第一封邮件发送于 1971 年，由 Ray Tomlinson 发送",
    "Vim 编辑器已经有 30 多年历史，至今仍是开发者的最爱之一",
    "IPv6 地址长度为 128 位，可以为地球上的每粒沙子分配一个地址",
    "程序员每天平均要花 40% 的时间在调试上",
    "ZJHCMD 的 hash 命令可以计算 MD5、SHA1、SHA256、SHA384、SHA512",
    "GitHub 上最受欢迎的编程语言是 JavaScript，Python 紧随其后",
    "1972 年，C 语言诞生，至今仍在广泛使用"
};

void HandleFortune() {
    int index = rand() % (sizeof(g_fortunes) / sizeof(g_fortunes[0]));
    std::cout << "\n";
    std::cout << "───────────────────────────────────────────────\n";
    std::cout << "     " << std::setw(40) << std::left << g_fortunes[index] << "\n";
    std::cout << "───────────────────────────────────────────────\n\n";
}

const char* g_quotes[][2] = {
    {"海内存知己，天涯若比邻", "—— 王勃"},
    {"学如逆水行舟，不进则退", "—— 增广贤文"},
    {"代码会过时，但思想永存", "—— ZJHCMD"},
    {"纸上得来终觉浅，绝知此事要躬行", "—— 陆游"},
    {"千里之行，始于足下", "—— 老子"},
    {"工欲善其事，必先利其器", "—— 论语"},
    {"Talk is cheap. Show me the code.", "—— Linus Torvalds"},
    {"Stay hungry, stay foolish.", "—— Steve Jobs"},
    {"任何足够先进的技术都与魔法无异", "—— Arthur C. Clarke"},
    {"人生苦短，我用Python", "—— 编程谚语"},
    {"真正的开发者，是解决问题的人", "—— ZJHCMD"},
    {"一个优秀的命令行工具，就是开发者的第二双手", "—— ZJHCMD"},
    {"学习新东西最好的方式，就是去用它", "—— ZJHCMD"},
    {"沉舟侧畔千帆过，病树前头万木春", "—— 刘禹锡"},
    {"长风破浪会有时，直挂云帆济沧海", "—— 李白"},
    {"业精于勤，荒于嬉", "—— 韩愈"},
    {"人生自古谁无死，留取丹心照汗青", "——文天祥"},
    {"Less is more", "—— Ludwig Mies van der Rohe"},
    {"Keep it simple, stupid", "—— 工程设计原则"},
    {"第一性原理：回归本质，重新构建", "—— Elon Musk"},
    {"在命令行里，你才是真正的掌控者", "—— ZJHCMD"}
};

void HandleQuote() {
    int index = rand() % (sizeof(g_quotes) / sizeof(g_quotes[0]));
    std::cout << "\n";
    std::cout << "───────────────────────────────────────────────\n";
    std::cout << "     " << std::setw(40) << std::left << g_quotes[index][0] << "\n";
    std::cout << "       " << std::setw(40) << std::left << g_quotes[index][1] << "\n";
    std::cout << "───────────────────────────────────────────────\n\n";
}

std::string GetWindowTitle() {
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);

    std::string exeDir = exePath;
    size_t lastSlash = exeDir.find_last_of("\\/");
    if (lastSlash != std::string::npos) {
        exeDir = exeDir.substr(0, lastSlash + 1);
    }

    std::string configPath = exeDir + ".zjhcmd\\";
    std::string fullPath = configPath + "WINDOWNAME";

    std::ifstream file(fullPath);
    if (!file.is_open()) {

        return "海内存知己，天涯若比邻";
    }

    std::string content;
    std::getline(file, content);
    file.close();

    const std::string whitespace = " \t\r\n";
    size_t start = content.find_first_not_of(whitespace);
    if (start == std::string::npos) {
        return "海内存知己，天涯若比邻";
    }
    size_t end = content.find_last_not_of(whitespace);
    content = content.substr(start, end - start + 1);

    if (content.size() >= 3 &&
        (unsigned char)content[0] == 0xEF &&
        (unsigned char)content[1] == 0xBB &&
        (unsigned char)content[2] == 0xBF) {
        content = content.substr(3);
    }

    if (content.empty()) {
        return "海内存知己，天涯若比邻";
    }

    return content;
}

std::string GetErrorTemplate() {
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);

    std::string exeDir = exePath;
    size_t lastSlash = exeDir.find_last_of("\\/");
    if (lastSlash != std::string::npos) {
        exeDir = exeDir.substr(0, lastSlash + 1);
    }

    std::string configPath = exeDir + ".zjhcmd\\";
    std::string fullPath = configPath + "ERROR_TEMPLATE";

    std::ifstream file(fullPath);
    if (!file.is_open()) {

        return "'{}' 不是JH的内部或外部命令，也不是可运行的程序或批处理文件。";
    }

    std::string content;
    std::getline(file, content);
    file.close();

    const std::string whitespace = " \t\r\n";
    size_t start = content.find_first_not_of(whitespace);
    if (start == std::string::npos) {
        return "'{}' 不是JH的内部或外部命令，也不是可运行的程序或批处理文件。";
    }
    size_t end = content.find_last_not_of(whitespace);
    content = content.substr(start, end - start + 1);

    if (content.size() >= 3 &&
        (unsigned char)content[0] == 0xEF &&
        (unsigned char)content[1] == 0xBB &&
        (unsigned char)content[2] == 0xBF) {
        content = content.substr(3);
    }

    if (content.empty()) {
        return "'{}' 不是JH的内部或外部命令，也不是可运行的程序或批处理文件。";
    }

    return content;
}

std::string FormatErrorMessage(const std::string& cmdName) {
    std::string templateStr = GetErrorTemplate();
    std::string result = templateStr;

    size_t pos = result.find("{}");
    if (pos != std::string::npos) {
        result.replace(pos, 2, cmdName);
    }

    size_t nPos = 0;
    while ((nPos = result.find("\\n", nPos)) != std::string::npos) {
        result.replace(nPos, 2, "\n");
        nPos += 1;
    }

    return result;
}

static std::string W2U8(const std::wstring& w) {
    if (w.empty()) return "";
    int len = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(),
        nullptr, 0, nullptr, nullptr);
    if (len <= 0) return "";
    std::string out(len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(),
        &out[0], len, nullptr, nullptr);
    return out;
}

static std::wstring U82W(const std::string& s) {
    if (s.empty()) return L"";
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(),
        nullptr, 0);
    if (len <= 0) return L"";
    std::wstring out(len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(),
        &out[0], len);
    return out;
}

static std::string FormatNumber(unsigned long long n) {
    std::string s = std::to_string(n);
    std::string r;
    int cnt = 0;
    for (int i = (int)s.size() - 1; i >= 0; --i) {
        r.push_back(s[i]);
        if (++cnt % 3 == 0 && i != 0) r.push_back(',');
    }
    std::reverse(r.begin(), r.end());
    return r;
}

static bool IsSystemDir(const std::wstring& name) {
    return name == L"$RECYCLE.BIN"
        || name == L"System Volume Information"
        || name == L"." || name == L"..";
}

bool IsCommandAllowed(const std::string& cmd) {
    if (cmd.empty()) return false;

    size_t start = cmd.find_first_not_of(" \t");
    if (start == std::string::npos) return false;
    std::string trimmed = cmd.substr(start);

    std::string lower = trimmed;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    for (int i = 0; g_commandWhitelist[i] != nullptr; i++) {
        std::string wl = g_commandWhitelist[i];
        std::transform(wl.begin(), wl.end(), wl.begin(), ::tolower);
        if (lower == wl) return true;
        if (lower.size() > wl.size() &&
            lower.compare(0, wl.size(), wl) == 0 &&
            (lower[wl.size()] == ' ' || lower[wl.size()] == '\t')) {
            return true;
        }
    }
    return false;
}

bool HasDangerousConstruct(const std::string& cmd) {
    if (cmd.find('|') != std::string::npos) return true;

    if (cmd.find("&&") != std::string::npos) return true;
    if (cmd.find("||") != std::string::npos) return true;
    if (cmd.find('&') != std::string::npos) return true;

    if (cmd.find('>') != std::string::npos) return true;
    if (cmd.find('<') != std::string::npos) return true;

    if (cmd.find('`') != std::string::npos) return true;
    if (cmd.find("$(") != std::string::npos) return true;

    if (cmd.find('\n') != std::string::npos) return true;
    if (cmd.find('\r') != std::string::npos) return true;

    return false;
}

bool IsCommandSafe(const std::string& cmd) {
    if (cmd.empty()) return false;

    if (HasDangerousConstruct(cmd)) {
        return false;
    }

    if (!IsCommandAllowed(cmd)) {
        return false;
    }

    return true;
}