#pragma once

#include <windows.h>
#include <bcrypt.h>
#include <winhttp.h>
#include <thread>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <vector>
#include <set>
#include <string>
#include <iostream>
#include <fstream>
#include <cctype>
#include <algorithm>
#include "JHCOMMAND3.h"

#pragma comment(lib, "bcrypt.lib")
#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "shell32.lib")

std::string BuildZJHCMDKnowledge();
static const std::string NO_WAY_TAG = "<JHNoWay!>";

bool IsDangerousCommand(const std::string& cmd) {
    static const char* blacklist[] = {
        "JH_Shutdown",
        "shutdown",
        "del /s",
        "del /f",
        "del /q",
        "format",
        "_admin_",
        "apt remove",
        "apt install",
        "st clean",
        "taskkill /F",
        "taskkill /f",
        "rmdir /s",
        "rd /s",
        "rm -rf",
        nullptr
    };

    std::string lower = cmd;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    for (int i = 0; blacklist[i] != nullptr; i++) {
        std::string bl = blacklist[i];
        std::transform(bl.begin(), bl.end(), bl.begin(), ::tolower);
        if (lower.find(bl) != std::string::npos) {
            return true;
        }
    }
    return false;
}

std::string escapeJson(const std::string& s) {
    std::string result;
    for (char c : s) {
        if (c == '"') result += "\\\"";
        else if (c == '\\') result += "\\\\";
        else if (c == '\n') result += "\\n";
        else if (c == '\r') result += "\\r";
        else if (c == '\t') result += "\\t";
        else if (c < 0x20) {
            char buf[8];
            sprintf_s(buf, "\\u%04x", (unsigned char)c);
            result += buf;
        }
        else result += c;
    }
    return result;
}

std::string GetTempDirectory() {
    char tempPath[MAX_PATH];
    DWORD length = GetTempPathA(MAX_PATH, tempPath);
    if (length == 0 || length > MAX_PATH) return "C:\\Windows\\Temp\\";
    return std::string(tempPath);
}

bool SendNotificationSimple(const std::string& title, const std::string& message) {
    std::string tempDir = GetTempDirectory();
    std::string scriptPath = tempDir + "jhcmd_notify.vbs";
    std::ofstream vbsFile(scriptPath);
    if (!vbsFile.is_open()) return false;
    std::string escapedTitle, escapedMsg;
    for (char c : title) {
        if (c == '"') escapedTitle += "\"\"";
        else escapedTitle += c;
    }
    for (char c : message) {
        if (c == '"') escapedMsg += "\"\"";
        else escapedMsg += c;
    }
    vbsFile << "Set WSHShell = WScript.CreateObject(\"WScript.Shell\")\n";
    vbsFile << "WSHShell.Popup \"" << escapedMsg << "\", 5, \"" << escapedTitle << "\", 64\n";
    vbsFile.close();
    SHELLEXECUTEINFOA sei = { sizeof(sei) };
    sei.lpVerb = "open";
    sei.lpFile = scriptPath.c_str();
    sei.nShow = SW_HIDE;
    BOOL result = ShellExecuteExA(&sei);
    Sleep(1000);
    DeleteFileA(scriptPath.c_str());
    return result == TRUE;
}

std::string HttpGetRequest(const std::string& host, const std::string& path) {
    HINTERNET hSession = WinHttpOpen(L"ZJHCMD/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return "";
    std::wstring wHost = StringToWString(host);
    HINTERNET hConnect = WinHttpConnect(hSession, wHost.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        return "";
    }
    std::wstring wPath = StringToWString(path);
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", wPath.c_str(),
        NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return "";
    }
    if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
        WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return "";
    }
    if (!WinHttpReceiveResponse(hRequest, NULL)) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return "";
    }
    std::string response;
    DWORD bytesRead = 0;
    char buffer[4096];
    while (WinHttpReadData(hRequest, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead > 0) {
        buffer[bytesRead] = '\0';
        response += buffer;
    }
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return response;
}

std::string HttpGetRequestWithHeaders(const std::string& host, const std::string& path, const std::string& headers = "") {
    HINTERNET hSession = WinHttpOpen(L"ZJHCMD/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return "";
    std::wstring wHost = StringToWString(host);
    HINTERNET hConnect = WinHttpConnect(hSession, wHost.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        return "";
    }
    std::wstring wPath = StringToWString(path);
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", wPath.c_str(),
        NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return "";
    }
    if (!headers.empty()) {
        std::wstring wHeaders = StringToWString(headers);
        WinHttpAddRequestHeaders(hRequest, wHeaders.c_str(), (DWORD)-1L, WINHTTP_ADDREQ_FLAG_ADD);
    }
    if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
        WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return "";
    }
    if (!WinHttpReceiveResponse(hRequest, NULL)) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return "";
    }
    std::string response;
    DWORD bytesRead = 0;
    char buffer[4096];
    while (WinHttpReadData(hRequest, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead > 0) {
        buffer[bytesRead] = '\0';
        response += buffer;
    }
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return response;
}

void HandleWeather(const std::string& city) {
    if (city.empty()) {
        std::cout << "用法: st weather <城市名>\n";
        std::cout << "示例: st weather Beijing\n";
        std::cout << "      st weather shanghai\n";
        std::cout << "      st weather London\n\n";
        return;
    }
    std::cout << "   正在查询 " << city << " 的天气...\n";
    std::string host = "wttr.in";
    std::string path = "/" + city + "?format=%t+%w+%h+%p&lang=en";
    std::string result = HttpGetRequest(host, path);
    if (result.empty()) {
        std::cout << "错误: 无法获取天气数据，请检查网络连接\n\n";
        return;
    }
    std::string filtered;
    for (char c : result) {
        if ((c >= 32 && c <= 126) || c == '+' || c == '-' || c == '.' || c == '%') {
            filtered += c;
        }
    }
    while (!filtered.empty() && (filtered.back() == '\n' || filtered.back() == '\r')) {
        filtered.pop_back();
    }
    std::cout << "\n========================================\n";
    std::cout << "      " << city << ": " << filtered << "\n";
    std::cout << "========================================\n\n";
}

std::string CalculateFileHashCryptoAPI(const std::string& filePath, ALG_ID algId) {
    std::wstring wFilePath;
    {
        int len = MultiByteToWideChar(CP_UTF8, 0, filePath.c_str(), -1, NULL, 0);
        if (len > 0) {
            wFilePath.resize(len - 1);
            MultiByteToWideChar(CP_UTF8, 0, filePath.c_str(), -1, &wFilePath[0], len);
        }
    }

    HANDLE hFile = CreateFileW(wFilePath.c_str(), GENERIC_READ, FILE_SHARE_READ,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        DWORD err = GetLastError();
        std::stringstream ss;
        ss << "错误: 无法打开文件 (错误码: " << err << ")";
        return ss.str();
    }
    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile, &fileSize) || fileSize.QuadPart == 0) {
        CloseHandle(hFile);
        return "错误: 文件为空或无法获取大小";
    }
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
        if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
            DWORD err = GetLastError();
            CloseHandle(hFile);
            std::stringstream ss;
            ss << "错误: CryptAcquireContext 失败 (错误码: " << err << ")";
            return ss.str();
        }
    }
    if (!CryptCreateHash(hProv, algId, 0, 0, &hHash)) {
        DWORD err = GetLastError();
        CryptReleaseContext(hProv, 0);
        CloseHandle(hFile);
        std::stringstream ss;
        ss << "错误: CryptCreateHash 失败 (错误码: " << err << ")";
        return ss.str();
    }
    const DWORD BUFFER_SIZE = 64 * 1024;
    std::vector<BYTE> buffer(BUFFER_SIZE);
    DWORD bytesRead = 0;
    while (ReadFile(hFile, buffer.data(), BUFFER_SIZE, &bytesRead, NULL) && bytesRead > 0) {
        if (!CryptHashData(hHash, buffer.data(), bytesRead, 0)) {
            DWORD err = GetLastError();
            CryptDestroyHash(hHash);
            CryptReleaseContext(hProv, 0);
            CloseHandle(hFile);
            std::stringstream ss;
            ss << "错误: CryptHashData 失败 (错误码: " << err << ")";
            return ss.str();
        }
    }
    DWORD hashLen = 0;
    DWORD dwLen = sizeof(DWORD);
    if (!CryptGetHashParam(hHash, HP_HASHSIZE, (BYTE*)&hashLen, &dwLen, 0)) {
        DWORD err = GetLastError();
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        CloseHandle(hFile);
        std::stringstream ss;
        ss << "错误: CryptGetHashParam 失败 (错误码: " << err << ")";
        return ss.str();
    }
    std::vector<BYTE> hash(hashLen);
    if (!CryptGetHashParam(hHash, HP_HASHVAL, hash.data(), &hashLen, 0)) {
        DWORD err = GetLastError();
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        CloseHandle(hFile);
        std::stringstream ss;
        ss << "错误: 获取哈希值失败 (错误码: " << err << ")";
        return ss.str();
    }
    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);
    CloseHandle(hFile);
    std::stringstream ss;
    ss << std::hex << std::setfill('0') << std::uppercase;
    for (BYTE b : hash) {
        ss << std::setw(2) << (int)b;
    }
    return ss.str();
}

std::string HandleHashCommand(const std::string& param) {
    std::string filePath = param;
    size_t s = filePath.find_first_not_of(" \t");
    if (s == std::string::npos) return "用法: hash <文件路径> [算法]";
    filePath = filePath.substr(s);
    size_t spacePos = filePath.find(' ');
    std::string algo = "ALL";
    if (spacePos != std::string::npos) {
        algo = filePath.substr(spacePos + 1);
        filePath = filePath.substr(0, spacePos);
        s = filePath.find_first_not_of(" \t");
        if (s != std::string::npos) filePath = filePath.substr(s);
        s = filePath.find_last_not_of(" \t");
        if (s != std::string::npos) filePath = filePath.substr(0, s + 1);
    }
    if (filePath.size() >= 2 && filePath.front() == '"' && filePath.back() == '"') {
        filePath = filePath.substr(1, filePath.size() - 2);
    }
    filePath = ExpandEnvironmentVars(filePath);
    std::string result = "文件: " + filePath + "\n";
    result += "========================================\n";
    if (algo == "ALL" || algo == "MD5") {
        result += "MD5:    " + CalculateFileHashCryptoAPI(filePath, CALG_MD5) + "\n";
    }
    if (algo == "ALL" || algo == "SHA1") {
        result += "SHA1:   " + CalculateFileHashCryptoAPI(filePath, CALG_SHA1) + "\n";
    }
    if (algo == "ALL" || algo == "SHA256") {
        result += "SHA256: " + CalculateFileHashCryptoAPI(filePath, CALG_SHA_256) + "\n";
    }
    if (algo == "SHA384") {
        result += "SHA384: " + CalculateFileHashCryptoAPI(filePath, CALG_SHA_384) + "\n";
    }
    if (algo == "SHA512") {
        result += "SHA512: " + CalculateFileHashCryptoAPI(filePath, CALG_SHA_512) + "\n";
    }
    return result;
}

void HandleAlarm(int delaySeconds, const std::string& message) {
    if (delaySeconds <= 0) {
        std::cout << "错误: 请输入正数秒数\n";
        return;
    }
    if (delaySeconds > 86400) {
        std::cout << "最大秒数不能超过86400秒（24小时）\n";
        return;
    }
    std::string msg = message.empty() ? "闹钟响了！" : message;
    std::cout << "   闹钟已设置，将在 " << delaySeconds << " 秒后提醒\n";
    std::cout << "   提醒内容: " << msg << "\n";
    std::thread([delaySeconds, msg]() {
        Sleep(delaySeconds * 1000);
        printf("\n\n==============================\n");
        printf("   闹钟响了！\n");
        printf("   %s\n", msg.c_str());
        printf("==============================\n\n");
        for (int i = 0; i < 5; i++) {
            Beep(880, 300);
            Sleep(200);
        }
        SendNotificationSimple("  闹钟", msg);
        }).detach();
}

void HandleTimer(int seconds) {
    if (seconds <= 0) {
        std::cout << "错误: 请输入正数秒数\n";
        return;
    }
    if (seconds > 3600) {
        std::cout << "最大秒数不能超过3600秒（1小时）\n";
        return;
    }
    std::cout << "   倒计时 " << seconds << " 秒开始...\n";
    for (int i = seconds; i > 0; i--) {
        printf("\r   剩余: %d 秒  ", i);
        fflush(stdout);
        Sleep(1000);
    }
    printf("\r   时间到！                    \n\n");
    Beep(1000, 500);
}

std::string UrlEncode(const std::string& str) {
    std::string result;
    for (char c : str) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            result += c;
        }
        else if (c == ' ') {
            result += '+';
        }
        else {
            char hex[4];
            sprintf_s(hex, "%%%02X", (unsigned char)c);
            result += hex;
        }
    }
    return result;
}

// [UTF8-FIX] UnicodeDecode 改为输出 UTF-8，不再转 GBK
std::string UnicodeDecode(const std::string& str) {
    std::string processed;
    for (size_t i = 0; i < str.length(); i++) {
        if (str[i] == 'u' && i + 4 < str.length()) {
            std::string hex = str.substr(i + 1, 4);
            bool valid = true;
            for (char c : hex) {
                if (!isxdigit(c)) { valid = false; break; }
            }
            if (valid) {
                processed += "\\u" + hex;
                i += 5;
                continue;
            }
        }
        processed += str[i];
    }
    std::string result;
    size_t i = 0;
    while (i < processed.length()) {
        if (processed[i] == '\\' && i + 1 < processed.length() && processed[i + 1] == 'u') {
            if (i + 5 < processed.length()) {
                std::string hex = processed.substr(i + 2, 4);
                bool valid = true;
                for (char c : hex) {
                    if (!isxdigit(c)) { valid = false; break; }
                }
                if (valid) {
                    unsigned int code = 0;
                    sscanf_s(hex.c_str(), "%x", &code);
                    wchar_t wc = (wchar_t)code;
                    char utf8[8] = { 0 };
                    // [UTF8-FIX] CP_ACP -> CP_UTF8
                    int len = WideCharToMultiByte(CP_UTF8, 0, &wc, 1, utf8, sizeof(utf8), NULL, NULL);
                    if (len > 0) {
                        result += std::string(utf8, len);
                    }
                    else {
                        result += processed.substr(i, 6);
                    }
                    i += 6;
                    continue;
                }
            }
            result += processed[i];
            i++;
            continue;
        }
        result += processed[i];
        i++;
    }
    return result;
}

std::string TranslateText(const std::string& text, const std::string& fromLang, const std::string& toLang) {
    if (text.empty()) return "错误: 请输入要翻译的文本";
    std::string host = "api.mymemory.translated.net";
    std::string path = "/get?q=" + UrlEncode(text) + "&langpair=" + fromLang + "|" + toLang;
    std::string response = HttpGetRequestWithHeaders(host, path, "Accept: application/json\r\n");
    if (response.empty()) {
        return "错误: 翻译服务不可用，请检查网络";
    }
    size_t pos = response.find("\"translatedText\":\"");
    if (pos == std::string::npos) {
        pos = response.find("\"responseData\"");
        if (pos != std::string::npos) {
            pos = response.find("\"translatedText\"", pos);
            if (pos != std::string::npos) {
                pos = response.find(":\"", pos);
            }
        }
        if (pos == std::string::npos) {
            return "错误: 无法解析翻译结果";
        }
        pos += 2;
    }
    else {
        pos += 19;
    }
    size_t end = response.find("\"", pos);
    if (end == std::string::npos) {
        return "错误: 解析翻译结果失败";
    }
    std::string result = response.substr(pos, end - pos);
    result = UnicodeDecode(result);  // [UTF8-FIX] 现在返回 UTF-8
    return result;
}

void HandleTranslate(const std::string& input) {
    std::string text = input;
    size_t s = text.find_first_not_of(" \t");
    if (s == std::string::npos) {
        std::cout << "用法: st translate <文本> [目标语言]\n";
        std::cout << "示例:\n";
        std::cout << "  st translate \"Hello\" zh-CN    - 英文转中文\n";
        std::cout << "  st translate \"你好\" en          - 中文转英文\n";
        std::cout << "  st translate \"Bonjour\" zh-CN   - 法文转中文\n\n";
        std::cout << "支持的语言代码:\n";
        std::cout << "  zh-CN  简体中文\n";
        std::cout << "  zh-TW  繁体中文\n";
        std::cout << "  en     英语\n";
        std::cout << "  ja     日语\n";
        std::cout << "  ko     韩语\n";
        std::cout << "  fr     法语\n";
        std::cout << "  de     德语\n";
        std::cout << "  es     西班牙语\n";
        std::cout << "  ru     俄语\n";
        std::cout << "  it     意大利语\n";
        std::cout << "  pt     葡萄牙语\n";
        std::cout << "  ar     阿拉伯语\n\n";
        return;
    }
    text = text.substr(s);
    std::string textToTranslate;
    std::string targetLang = "zh-CN";
    bool inQuote = false;
    std::string current;
    std::vector<std::string> parts;
    for (char c : text) {
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
    if (parts.empty()) {
        std::cout << "错误: 请输入要翻译的文本\n\n";
        return;
    }
    if (parts.size() >= 2) {
        textToTranslate = parts[0];
        targetLang = parts[1];
    }
    else {
        textToTranslate = parts[0];
    }
    std::string fromLang = "en";
    if (targetLang == "en") {
        fromLang = "zh-CN";
    }
    bool hasChinese = false;
    for (char c : textToTranslate) {
        if ((unsigned char)c >= 0x80) {
            hasChinese = true;
            break;
        }
    }
    if (hasChinese && targetLang != "en") {
        fromLang = "zh-CN";
    }
    std::cout << "  翻译中: \"" << textToTranslate << "\" ...\n";
    std::string result = TranslateText(textToTranslate, fromLang, targetLang);
    std::cout << "\n========================================\n";
    std::cout << "  原文 (" << fromLang << "): " << textToTranslate << "\n";
    std::cout << "  译文 (" << targetLang << "): " << result << "\n";
    std::cout << "========================================\n\n";
}

void HandleNews() {
    std::cout << "  正在获取今日头条新闻...\n";
    std::string host = "api.rss2json.com";
    std::string path = "/v1/api.json?rss_url=https%3A%2F%2Fwww.xinhuanet.com%2Fpolitics%2Fxw.xml";
    std::string response = HttpGetRequestWithHeaders(host, path, "Accept: application/json\r\n");
    if (response.empty() || response.length() < 100) {
        host = "api.rss2json.com";
        path = "/v1/api.json?rss_url=http%3A%2F%2Fwww.36kr.com%2Ffeed";
        response = HttpGetRequestWithHeaders(host, path, "Accept: application/json\r\n");
    }
    if (response.empty() || response.length() < 100) {
        host = "api.rss2json.com";
        path = "/v1/api.json?rss_url=https%3A%2F%2Fnews.sina.com.cn%2Froll%2Fnews%2Fsudoku%2Fsudoku_news_1.xml";
        response = HttpGetRequestWithHeaders(host, path, "Accept: application/json\r\n");
    }
    if (response.empty() || response.length() < 100) {
        std::cout << "错误: 无法获取新闻，请检查网络连接\n\n";
        return;
    }
    std::cout << "\n========================================\n";
    std::cout << "             今日头条新闻\n";
    std::cout << "========================================\n\n";
    int count = 0;
    size_t pos = 0;
    std::set<std::string> seenTitles;
    while (count < 10 && pos < response.length()) {
        size_t titlePos = response.find("\"title\"", pos);
        if (titlePos == std::string::npos) break;
        size_t colonPos = response.find(":", titlePos);
        if (colonPos == std::string::npos) break;
        size_t quoteStart = response.find("\"", colonPos + 1);
        if (quoteStart == std::string::npos) break;
        size_t quoteEnd = response.find("\"", quoteStart + 1);
        if (quoteEnd == std::string::npos) break;
        std::string title = response.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
        title = UnicodeDecode(title);  // [UTF8-FIX] 现在返回 UTF-8
        size_t ampPos;
        while ((ampPos = title.find("&amp;")) != std::string::npos) {
            title.replace(ampPos, 5, "&");
        }
        while ((ampPos = title.find("&quot;")) != std::string::npos) {
            title.replace(ampPos, 6, "\"");
        }
        while ((ampPos = title.find("&#39;")) != std::string::npos) {
            title.replace(ampPos, 5, "'");
        }
        while ((ampPos = title.find("&lt;")) != std::string::npos) {
            title.replace(ampPos, 4, "<");
        }
        while ((ampPos = title.find("&gt;")) != std::string::npos) {
            title.replace(ampPos, 4, ">");
        }
        if (!title.empty() && title.length() > 10 &&
            title.find("http") == std::string::npos &&
            title.find("广告") == std::string::npos &&
            title.find("Advertisement") == std::string::npos &&
            title != "null" && title != "[Removed]" &&
            title.find("专题") == std::string::npos &&
            seenTitles.find(title) == seenTitles.end()) {
            seenTitles.insert(title);
            count++;
            std::cout << "  " << count << ". " << title << "\n";
        }
        pos = quoteEnd + 1;
    }
    if (count == 0) {
        std::cout << "  没有找到新闻条目\n";
    }
    std::cout << "\n========================================\n";
    std::cout << "  共 " << count << " 条新闻\n\n";
}

std::string GbkToUtf8(const std::string& gbkStr) {
    if (gbkStr.empty()) return "";
    int wLen = MultiByteToWideChar(CP_ACP, 0, gbkStr.c_str(), -1, NULL, 0);
    if (wLen <= 0) return gbkStr;
    std::wstring wStr(wLen, L'\0');
    MultiByteToWideChar(CP_ACP, 0, gbkStr.c_str(), -1, &wStr[0], wLen);
    int utf8Len = WideCharToMultiByte(CP_UTF8, 0, wStr.c_str(), -1, NULL, 0, NULL, NULL);
    if (utf8Len <= 0) return gbkStr;
    std::string utf8Str(utf8Len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wStr.c_str(), -1, &utf8Str[0], utf8Len, NULL, NULL);
    if (!utf8Str.empty() && utf8Str.back() == '\0') utf8Str.pop_back();
    return utf8Str;
}

std::string HttpPostRequestW(const std::string& host, const std::string& path,
    const std::string& headers, const std::string& data) {
    HINTERNET hSession = WinHttpOpen(L"ZJHCMD/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return "";
    std::wstring wHost = StringToWString(host);
    HINTERNET hConnect = WinHttpConnect(hSession, wHost.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        return "";
    }
    std::wstring wPath = StringToWString(path);
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", wPath.c_str(),
        NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return "";
    }
    if (!headers.empty()) {
        std::wstring wHeaders = StringToWString(headers);
        WinHttpAddRequestHeaders(hRequest, wHeaders.c_str(), (DWORD)-1L, WINHTTP_ADDREQ_FLAG_ADD);
    }
    // 注意：这里是“发出去”的方向，ANSI -> UTF-8，必须保留
    int wLen = MultiByteToWideChar(CP_ACP, 0, data.c_str(), -1, NULL, 0);
    if (wLen > 0) {
        std::wstring wData(wLen, L'\0');
        MultiByteToWideChar(CP_ACP, 0, data.c_str(), -1, &wData[0], wLen);
        int utf8Len = WideCharToMultiByte(CP_UTF8, 0, wData.c_str(), -1, NULL, 0, NULL, NULL);
        if (utf8Len > 0) {
            std::string utf8Data(utf8Len, '\0');
            WideCharToMultiByte(CP_UTF8, 0, wData.c_str(), -1, &utf8Data[0], utf8Len, NULL, NULL);
            while (!utf8Data.empty() && utf8Data.back() == '\0') utf8Data.pop_back();
            if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                (LPVOID)utf8Data.c_str(), (DWORD)utf8Data.size(),
                (DWORD)utf8Data.size(), 0)) {
                WinHttpCloseHandle(hRequest);
                WinHttpCloseHandle(hConnect);
                WinHttpCloseHandle(hSession);
                return "";
            }
        }
        else {
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return "";
        }
    }
    else {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return "";
    }
    if (!WinHttpReceiveResponse(hRequest, NULL)) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return "";
    }
    std::string response;
    DWORD bytesRead = 0;
    char buffer[4096];
    while (WinHttpReadData(hRequest, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead > 0) {
        buffer[bytesRead] = '\0';
        response += buffer;
    }
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return response;
}

struct ChatContext {
    std::vector<std::pair<std::string, std::string>> messages;
    bool hasContext;
    bool knowledgeInjected;
    std::string lastQuestion;
    std::string lastAnswer;
    ChatContext() : hasContext(false), knowledgeInjected(false) {}
    void AddMessage(const std::string& role, const std::string& content) {
        messages.push_back({ role, content });
        if (messages.size() > 20) {
            messages.erase(messages.begin());
        }
        hasContext = true;
    }
    void Clear() {
        messages.clear();
        hasContext = false;
        knowledgeInjected = false;
        lastQuestion.clear();
        lastAnswer.clear();
    }
    std::string BuildContext() const {
        std::string ctx;
        for (const auto& msg : messages) {
            ctx += "{\"role\": \"" + msg.first + "\", \"content\": \"" + escapeJson(msg.second) + "\"},";
        }
        if (!ctx.empty()) ctx.pop_back();
        return ctx;
    }
};

static ChatContext g_chatContext;

struct AskAIParams {
    std::string question;
    bool deepThink = false;
    std::string imagePath;
    bool newTopic = false;
    bool runMode = false;
    bool quietMode = false;
};

std::string Base64Encode(const std::vector<BYTE>& data) {
    static const char* base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string result;
    int i = 0, j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];
    for (BYTE byte : data) {
        char_array_3[i++] = byte;
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            for (i = 0; i < 4; i++) result += base64_chars[char_array_4[i]];
            i = 0;
        }
    }
    if (i) {
        for (j = i; j < 3; j++) char_array_3[j] = '\0';
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;
        for (j = 0; j < i + 1; j++) result += base64_chars[char_array_4[j]];
        while (i++ < 3) result += '=';
    }
    return result;
}

std::string ImageToBase64(const std::string& imagePath) {
    HANDLE hFile = CreateFileA(imagePath.c_str(), GENERIC_READ, FILE_SHARE_READ,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return "";
    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize == 0 || fileSize > 20 * 1024 * 1024) {
        CloseHandle(hFile);
        return "";
    }
    std::vector<BYTE> buffer(fileSize);
    DWORD bytesRead;
    if (!ReadFile(hFile, buffer.data(), fileSize, &bytesRead, NULL)) {
        CloseHandle(hFile);
        return "";
    }
    CloseHandle(hFile);
    return Base64Encode(buffer);
}

std::string GetImageMimeType(const std::string& path) {
    std::string lower = path;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    if (lower.find(".png") != std::string::npos) return "image/png";
    if (lower.find(".jpg") != std::string::npos || lower.find(".jpeg") != std::string::npos) return "image/jpeg";
    if (lower.find(".gif") != std::string::npos) return "image/gif";
    if (lower.find(".bmp") != std::string::npos) return "image/bmp";
    if (lower.find(".webp") != std::string::npos) return "image/webp";
    return "image/png";
}

AskAIParams ParseAskAICommand(const std::string& input) {
    AskAIParams params;
    std::string remaining = input;

    size_t runQPos = remaining.find("/run:q");
    if (runQPos != std::string::npos) {
        bool leftOk = (runQPos == 0) ||
            (remaining[runQPos - 1] == ' ' || remaining[runQPos - 1] == '\t');
        bool rightOk = (runQPos + 6 >= remaining.size()) ||
            (remaining[runQPos + 6] == ' ' || remaining[runQPos + 6] == '\t');
        if (leftOk && rightOk) {
            params.runMode = true;
            params.quietMode = true;
            remaining.erase(runQPos, 6);
        }
    }

    if (!params.runMode) {
        size_t runPos = remaining.find("/run");
        if (runPos != std::string::npos) {
            bool leftOk = (runPos == 0) ||
                (remaining[runPos - 1] == ' ' || remaining[runPos - 1] == '\t');
            bool rightOk = (runPos + 4 >= remaining.size()) ||
                (remaining[runPos + 4] == ' ' || remaining[runPos + 4] == '\t');
            if (leftOk && rightOk) {
                params.runMode = true;
                remaining.erase(runPos, 4);
            }
        }
    }

    size_t newPos = remaining.find("/new");
    if (newPos != std::string::npos) {
        bool leftOk = (newPos == 0) ||
            (remaining[newPos - 1] == ' ' || remaining[newPos - 1] == '\t');
        bool rightOk = (newPos + 4 >= remaining.size()) ||
            (remaining[newPos + 4] == ' ' || remaining[newPos + 4] == '\t');
        if (leftOk && rightOk) {
            params.newTopic = true;
            remaining.erase(newPos, 4);
        }
    }

    size_t deepPos = remaining.find("/deep");
    if (deepPos != std::string::npos) {
        bool leftOk = (deepPos == 0) ||
            (remaining[deepPos - 1] == ' ' || remaining[deepPos - 1] == '\t');
        bool rightOk = (deepPos + 5 >= remaining.size()) ||
            (remaining[deepPos + 5] == ' ' || remaining[deepPos + 5] == '\t');
        if (leftOk && rightOk) {
            params.deepThink = true;
            remaining.erase(deepPos, 5);
        }
    }

    size_t picPos = remaining.find("picture:");
    if (picPos != std::string::npos) {
        size_t start = picPos + 8;

        while (start < remaining.size() &&
            (remaining[start] == ' ' || remaining[start] == '\t')) {
            start++;
        }

        if (start < remaining.size() && remaining[start] == '"') {
            size_t end = remaining.find('"', start + 1);
            if (end != std::string::npos) {
                params.imagePath = remaining.substr(start + 1, end - start - 1);
                remaining.erase(picPos, end - picPos + 1);
            }
            else {
                params.imagePath = remaining.substr(start + 1);
                remaining.erase(picPos);
            }
        }
        else {
            size_t end = start;
            while (end < remaining.size() &&
                remaining[end] != ' ' && remaining[end] != '\t') {
                end++;
            }
            if (end > start) {
                params.imagePath = remaining.substr(start, end - start);
                remaining.erase(picPos, end - picPos);
            }
            else {
                remaining.erase(picPos, 8);
            }
        }

        while (!remaining.empty() && remaining[0] == ' ') {
            remaining.erase(0, 1);
        }
    }

    size_t s = remaining.find_first_not_of(" \t");
    if (s != std::string::npos) {
        params.question = remaining.substr(s);
        size_t e = params.question.find_last_not_of(" \t\r\n");
        if (e != std::string::npos) {
            params.question = params.question.substr(0, e + 1);
        }
    }

    if (params.newTopic && params.question.empty() && params.imagePath.empty()) {
        params.question = "__NEW_TOPIC__";
    }

    return params;
}

std::string AskDeepSeekEnhanced(
    const std::string& question,
    bool deepThink,
    const std::string& imagePath,
    bool newTopic,
    bool runMode,
    int& promptTokens,
    int& completionTokens,
    int& totalTokens
) {
    promptTokens = 0;
    completionTokens = 0;
    totalTokens = 0;
    if (question == "__NEW_TOPIC__") {
        g_chatContext.Clear();
        return "已开启新话题，对话上下文已清空";
    }
    if (newTopic) {
        g_chatContext.Clear();
    }
    std::string API_KEY;
    std::ifstream keyFile("API_KEY");
    if (!keyFile.is_open()) {
        return "错误：找不到 API_KEY 文件，请在 ZJHCMD.exe 同目录下创建 API_KEY 文件并填入你的 DeepSeek API Key";
    }
    std::getline(keyFile, API_KEY);
    keyFile.close();
    while (!API_KEY.empty() && (API_KEY.back() == '\n' || API_KEY.back() == '\r' || API_KEY.back() == ' ')) {
        API_KEY.pop_back();
    }
    if (API_KEY.empty() || API_KEY.substr(0, 3) != "sk-") {
        return "错误：API_KEY 无效，应以 sk- 开头";
    }
    std::string messagesJson;
    bool hasImage = !imagePath.empty();

    std::string systemPrompt;

    if (runMode) {
        systemPrompt =
            "你是 ZJHCMD v" + version + " 的命令生成器。\n"
            "以下是 ZJHCMD 的全部内置命令：\n"
            "========================================\n"
            + BuildZJHCMDKnowledge() +
            "========================================\n"
            "\n"
            "【输出规则 — 必须严格遵守】\n"
            "\n"
            "情况A：能用上述命令完成用户请求\n"
            "  → 只输出那一行命令，不要任何解释、不要代码块、不要标点。\n"
            "\n"
            "情况B：不能用上述命令完成\n"
            "  → 第一行输出 " + NO_WAY_TAG + "\n"
            "  → 第二行开始输出原因，原因要简短明确。\n"
            "\n"
            "【禁止】\n"
            "  - 禁止输出问候语（好的、你可以、没问题）\n"
            "  - 禁止输出 markdown 代码块（```）\n"
            "  - 禁止输出\"命令：\"这样的前缀\n"
            "  - 禁止在情况A输出多行\n"
            "  - 禁止在情况B把原因写在 " + NO_WAY_TAG + " 同一行\n"
            "\n"
            "【正确示例】\n"
            "\n"
            "用户: 查一下C盘还剩多少空间\n"
            "输出:\n"
            "st drives\n"
            "\n"
            "用户: 把音量调到30\n"
            "输出:\n"
            "st volume 30\n"
            "\n"
            "用户: 帮我发一封邮件\n"
            "输出:\n"
            + NO_WAY_TAG + "\n"
            "ZJHCMD 没有内置发送邮件的命令，可用 start mailto:xxx@example.com\n"
            "\n"
            "【错误示例 — 禁止】\n"
            "\n"
            "好的，你可以使用 st drives 命令来查看磁盘空间。\n"
            "```zjhcmd\nst drives\n```\n"
            "命令：st drives\n"
            + NO_WAY_TAG + "ZJHCMD 没有内置发邮件命令\n";

        if (deepThink) {
            systemPrompt += "\n【深度思考】\n用户要求深度思考，请在内部充分推理后再输出最终命令。\n";
        }
        if (hasImage) {
            systemPrompt += "\n【图片识别】\n用户附加了图片，请结合图片内容生成命令。\n";
        }
        g_chatContext.AddMessage("system",
            "ZJHCMD 的命令列表：\n" + BuildZJHCMDKnowledge());
        g_chatContext.knowledgeInjected = true;
    }
    else {
        systemPrompt = "你是一个智能助手。";
        if (deepThink) {
            systemPrompt += " 用户要求你进行深度思考，请在回答前进行详细推理和分析，给出全面、深入、结构化的答案。不要急于给出结论，要展示思考过程。";
        }
        if (hasImage) {
            systemPrompt += " 你具备图像识别能力，请仔细分析用户提供的图片内容。";
        }
    }

    messagesJson += "{\"role\": \"system\", \"content\": \"" + escapeJson(systemPrompt) + "\"},";

    if (g_chatContext.hasContext && !g_chatContext.messages.empty()) {
        for (const auto& msg : g_chatContext.messages) {
            messagesJson += "{\"role\": \"" + msg.first + "\", \"content\": \"" + escapeJson(msg.second) + "\"},";
        }
    }
    std::string userContent = question;
    if (hasImage) {
        std::string base64Image = ImageToBase64(imagePath);
        if (base64Image.empty()) {
            return "错误：无法读取图片文件，请检查路径是否正确";
        }
        userContent = "[图片附件: " + imagePath + "]\n" + question;
    }
    messagesJson += "{\"role\": \"user\", \"content\": \"" + escapeJson(userContent) + "\"}";
    std::string jsonBody = "{\"model\": \"deepseek-chat\", \"messages\": [" + messagesJson + "], \"stream\": false";
    if (deepThink) {
        jsonBody += ", \"temperature\": 0.3, \"top_p\": 0.9";
    }
    if (runMode) {
        jsonBody += ", \"temperature\": 0.1, \"top_p\": 0.95";
    }
    jsonBody += "}";
    std::string headers = "Content-Type: application/json\r\n" + std::string("Authorization: Bearer ") + API_KEY + "\r\n";
    std::string response = HttpPostRequestW("api.deepseek.com", "/v1/chat/completions", headers, jsonBody);
    if (response.empty()) {
        return "错误：网络请求失败，请检查网络连接";
    }
    size_t usagePos = response.find("\"usage\"");
    if (usagePos != std::string::npos) {
        size_t ptPos = response.find("\"prompt_tokens\"", usagePos);
        if (ptPos != std::string::npos) {
            size_t start = response.find(":", ptPos);
            if (start != std::string::npos) {
                start++;
                while (start < response.size() && (response[start] == ' ' || response[start] == '\t')) start++;
                size_t end = response.find_first_of(",}", start);
                if (end != std::string::npos) {
                    std::string numStr = response.substr(start, end - start);
                    promptTokens = std::stoi(numStr);
                }
            }
        }
        size_t ctPos = response.find("\"completion_tokens\"", usagePos);
        if (ctPos != std::string::npos) {
            size_t start = response.find(":", ctPos);
            if (start != std::string::npos) {
                start++;
                while (start < response.size() && (response[start] == ' ' || response[start] == '\t')) start++;
                size_t end = response.find_first_of(",}", start);
                if (end != std::string::npos) {
                    std::string numStr = response.substr(start, end - start);
                    completionTokens = std::stoi(numStr);
                }
            }
        }
        size_t ttPos = response.find("\"total_tokens\"", usagePos);
        if (ttPos != std::string::npos) {
            size_t start = response.find(":", ttPos);
            if (start != std::string::npos) {
                start++;
                while (start < response.size() && (response[start] == ' ' || response[start] == '\t')) start++;
                size_t end = response.find_first_of(",}", start);
                if (end != std::string::npos) {
                    std::string numStr = response.substr(start, end - start);
                    totalTokens = std::stoi(numStr);
                }
            }
        }
    }
    size_t contentPos = response.find("\"content\"");
    if (contentPos == std::string::npos) {
        size_t errorPos = response.find("\"error\"");
        if (errorPos != std::string::npos) {
            size_t msgPos = response.find("\"message\"", errorPos);
            if (msgPos != std::string::npos) {
                size_t start = response.find("\"", msgPos + 10);
                if (start != std::string::npos) {
                    start++;
                    size_t end = response.find("\"", start);
                    if (end != std::string::npos) {
                        return "API 错误：" + response.substr(start, end - start);
                    }
                }
            }
            return "API 错误：请检查 API Key 是否正确或账户余额是否充足";
        }
        return "错误：解析 AI 响应失败";
    }
    size_t start = response.find("\"", contentPos + 10);
    if (start == std::string::npos) return "错误：解析失败";
    start++;
    std::string result;
    bool inEscape = false;
    size_t pos = start;
    while (pos < response.length()) {
        char c = response[pos];
        if (inEscape) {
            if (c == 'n') result += '\n';
            else if (c == 'r') result += '\r';
            else if (c == 't') result += '\t';
            else if (c == '\\') result += '\\';
            else if (c == '"') result += '"';
            else if (c == '/') result += '/';
            else result += c;
            inEscape = false;
        }
        else if (c == '\\') {
            inEscape = true;
        }
        else if (c == '"') {
            break;
        }
        else {
            result += c;
        }
        pos++;
    }

    if (runMode) {
        g_chatContext.AddMessage("user", question);
        g_chatContext.AddMessage("assistant", "[命令生成模式] " + result);
    }
    else {
        g_chatContext.AddMessage("user", question);
        g_chatContext.AddMessage("assistant", result);
    }

    // [UTF8-FIX] 不再转 GBK，直接返回 UTF-8
    return result;
}

std::string AskDeepSeek(const std::string& question, int& promptTokens, int& completionTokens, int& totalTokens) {
    return AskDeepSeekEnhanced(question, false, "", false, false, promptTokens, completionTokens, totalTokens);
}

std::string BuildZJHCMDKnowledge() {
    std::string kb;
    kb.reserve(16384);
    for (int i = 0; helpcommand[i] != nullptr; i++) {
        kb += helpcommand[i];
        kb += "\n";
    }
    return kb;
}
