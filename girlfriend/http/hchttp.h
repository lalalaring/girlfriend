//////////////////////////////////////////////////////////////////////////
//Common Cpp Class
//File:Hchttp.h
//Author:dingruiqiang(YanYuHongChen)
//2014-04-28
//Version:0.2
//	--没有完全读取，只读了一次BUFF_SIZE字节
//////////////////////////////////////////////////////////////////////////
#pragma once
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#include <atlbase.h>
#include <atlstr.h>
//
#define USE_WINHTTP    //Comment this line to user wininet.
#ifdef USE_WINHTTP
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")
#else
#include <wininet.h>
#pragma comment(lib, "wininet.lib")
#endif
#define BUF_SIZE (1024)

class CrackedUrl{
	int m_scheme;
	CStringW m_host;
	int m_port;
	CStringW m_path;
public:
	CrackedUrl(LPCWSTR url)
	{
		URL_COMPONENTS uc = { 0 };
		uc.dwStructSize = sizeof(uc);

		const DWORD BUF_LEN = 256;

		WCHAR host[BUF_LEN];
		uc.lpszHostName = host;
		uc.dwHostNameLength = BUF_LEN;

		WCHAR path[BUF_LEN];
		uc.lpszUrlPath = path;
		uc.dwUrlPathLength = BUF_LEN;

		WCHAR extra[BUF_LEN];
		uc.lpszExtraInfo = extra;
		uc.dwExtraInfoLength = BUF_LEN;

#ifdef USE_WINHTTP
		if (!WinHttpCrackUrl(url, 0, ICU_ESCAPE, &uc)) {
			printf("Error:WinHttpCrackUrl failed!\n");

		}

#else
		if (!InternetCrackUrl(url, 0, ICU_ESCAPE, &uc)) {
			printf("Error:InternetCrackUrl failed!\n");

		}
#endif
		m_scheme = uc.nScheme;
		m_host = host;
		m_port = uc.nPort;
		m_path = path;
	}

	int GetScheme() const
	{
		return m_scheme;
	}

	LPCWSTR GetHostName() const
	{
		return m_host;
	}

	int GetPort() const
	{
		return m_port;
	}

	LPCWSTR GetPath() const
	{
		return m_path;
	}

	static CStringA UrlEncode(const char* p)
	{
		if (p == 0) {
			return CStringA();

		}

		CStringA buf;

		for (;;) {
			int ch = (BYTE)(*(p++));
			if (ch == '\0') {
				break;

			}

			if (isalnum(ch) || ch == '_' || ch == '-' || ch == '.') {
				buf += (char)ch;

			}
			else if (ch == ' ') {
				buf += '+';

			}
			else {
				char c[16];
				wsprintfA(c, "%%%02X", ch);
				buf += c;

			}

		}

		return buf;
	}
};
enum Endode
{
	UTF8,
	//
	UNKNOWN

};
typedef struct RequestData 
{

	int contextLength;
	int statuscode;
	LPWSTR statuscodetext;
	Endode encode;
	char* data;
	int size;
}RequestData;
enum Method
{
	POST,
	GET
};
class hchttp
{
public:
	hchttp(LPCWSTR url);
	~hchttp();

	bool OneKeyGoNoWait(Method verb, const wchar_t* urlpass, const void* data, int size);
	bool OneKeyGoWait(Method verb, const wchar_t* urlpass, const void* data, int size, RequestData* out);
private:
	bool OpenSession(LPCWSTR userAgent);
	bool CloseSession();
	bool Connect();
	bool Close();
	bool Request(Method verb, const wchar_t* urlpass, const void* data, int size);
	bool GetRequest(RequestData* rdata);
	bool CloseRequest();

private:
	HINTERNET _session = 0;
	HINTERNET _connection = 0;
	HINTERNET _request = 0;
	//LPCWSTR url;
	CrackedUrl* crurl=nullptr;
};

