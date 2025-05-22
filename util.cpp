#include "pch.h"
#include "util.h"
#include <vector>

CString Utf8ToCString(const std::string& utf8Str)
{
    int wideCharLen = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, nullptr, 0);
    if (wideCharLen == 0) return CString();

    std::vector<WCHAR> wideCharBuf(wideCharLen);
    MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, wideCharBuf.data(), wideCharLen);

    return CString(wideCharBuf.data());
}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* data) 
{
    size_t totalSize = size * nmemb;
    data->append((char*)contents, totalSize);
    return totalSize;
}