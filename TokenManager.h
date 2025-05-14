#pragma once
#include <afxstr.h>

class TokenManager
{
public:
	static CString getToken() {
		return m_token;
	}

	static void setToken(const CString& token) {
		m_token = token;
	}

	static void removeToken() {
		m_token.Empty();
	}

	static bool isTokenValid() {
		return !m_token.IsEmpty();
	}

private:
	inline static CString m_token;
};

