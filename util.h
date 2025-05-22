#pragma once
#include <afxwin.h>
#include <string>

CString Utf8ToCString(const std::string& utf8Str);
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* data);
