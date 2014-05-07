//////////////////////////////////////////////////////////////////////////
//Common Cpp Class
//File:Hchttp.cpp
//Author:dingruiqiang(YanYuHongChen)
//2014-04-28
//Version:0.2
//	--没有完全读取，只读了一次BUFF_SIZE字节,统计那边不给加长度
//////////////////////////////////////////////////////////////////////////
#include "hchttp.h"

static LPWSTR strHeader(L"Content-type: application/x-www-form-urlencoded\r\n");
hchttp::hchttp(LPCWSTR url)
{
	crurl = new CrackedUrl(url);
}


hchttp::~hchttp()
{
}


bool hchttp::OpenSession(LPCWSTR userAgent = 0)
{
	
#ifdef USE_WINHTTP
	_session = WinHttpOpen(userAgent, NULL, NULL, NULL, NULL);;
#else
	_session = InternetOpen(userAgent, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
#endif
	if (_session)
	{
		return true;
	}
	return false;
}
bool hchttp::CloseSession()
{
	if (_session)
	{
#ifdef USE_WINHTTP
		bool rt = WinHttpCloseHandle(_session);
#else
		bool rt = InternetCloseHandle(_session);
#endif

		return rt;
	}
}
bool hchttp::Connect()
{
#ifdef USE_WINHTTP
	_connection = WinHttpConnect(_session, crurl->GetHostName(), (INTERNET_PORT)crurl->GetPort(), 0);
#else
	_connection =  InternetConnect(_session,  crurl->GetHostName(), crurl->GetPort(), NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
#endif
	if (_connection)
	{
		return true;
	}
	return false;

}
bool hchttp::Close()
{
	if (_connection)
	{
		bool rt = false;
#ifdef USE_WINHTTP
		rt = WinHttpCloseHandle(_connection);
#else
		rt = InternetCloseHandle(_connection);
#endif
		return rt;
	}
	return true;
}
bool hchttp::Request(Method verb, const wchar_t* urlpass, const void* data, int size)
{
	DWORD flags = 0;
	wchar_t tempurl[1024] =L"";//
	wcscat(tempurl, crurl->GetPath());
	wcscat(tempurl, L"?");
	wcscat(tempurl, urlpass);
#ifdef USE_WINHTTP
	if (crurl->GetScheme() == INTERNET_SCHEME_HTTPS) {
		flags |= WINHTTP_FLAG_SECURE;

	}

	switch (verb)
	{
	case POST:
		_request = WinHttpOpenRequest(_connection, L"POST", crurl->GetPath(), NULL, NULL, NULL, flags);
		break;
	case GET:
		_request = WinHttpOpenRequest(_connection, L"", tempurl, NULL, NULL, NULL, flags);
		break;
	default:
		break;
	}


#else
	if (crurl->GetScheme() == INTERNET_SCHEME_HTTPS) {
		flags |= INTERNET_FLAG_SECURE;

	}
	switch (verb)
	{
	case POST:
		_request = HttpOpenRequest(_connection, L"POST", crurl->GetPath(), NULL, NULL, NULL, flags, 0);
		break;
	case GET:
		_request = HttpOpenRequest(_connection, L"", tempurl, NULL, NULL, NULL, flags, 0);
		break;
	default:
		break;
	}

#endif

	bool rt = false;
	SIZE_T len = lstrlenW(strHeader);
	if (verb==POST)
	{
#ifdef USE_WINHTTP
		rt = WinHttpSendRequest(_request, strHeader, DWORD(len),const_cast<void*>(data), size, size, 0);
#else
		rt = HttpSendRequest(_request, strHeader, DWORD(len), const_cast<void*>(data), size);
#endif
	}
	else
	{
#ifdef USE_WINHTTP
		rt = WinHttpSendRequest(_request, strHeader, DWORD(len),0, 0, 0, 0);
#else
		rt = HttpSendRequest(_request, strHeader, DWORD(len), 0, 0);
#endif
	}


	return rt;
}
bool hchttp::GetRequest(RequestData* rdata)
{

#ifdef USE_WINHTTP
	int contextLengthId = WINHTTP_QUERY_CONTENT_LENGTH;
	int statusCodeId = WINHTTP_QUERY_STATUS_CODE;
	int statusTextId = WINHTTP_QUERY_STATUS_TEXT;
#else
	int contextLengthId = HTTP_QUERY_CONTENT_LENGTH;
	int statusCodeId = HTTP_QUERY_STATUS_CODE;
	int statusTextId = HTTP_QUERY_STATUS_TEXT;
#endif
	bool rt = false;
#ifdef USE_WINHTTP
	rt = WinHttpReceiveResponse(_request, 0);
#else
	// if you use HttpSendRequestEx to send request then use HttpEndRequest in here!
	//return TRUE;
#endif
	DWORD dwSize = BUF_SIZE;// BUF_SIZE;
	wchar_t szBuf[BUF_SIZE];
	memset(szBuf,0,BUF_SIZE);
#ifdef USE_WINHTTP
	rt = WinHttpQueryHeaders(_request, (DWORD)contextLengthId, 0, szBuf, &dwSize, 0);
#else
	rt = HttpQueryInfo(_request, (DWORD)contextLengthId, szBuf, &dwSize, 0);
#endif
	rdata->contextLength = _wtoi(szBuf);
	 dwSize = BUF_SIZE;
	 memset(szBuf, 0, BUF_SIZE);
#ifdef USE_WINHTTP
	 rt = WinHttpQueryHeaders(_request, (DWORD)statusCodeId, 0, szBuf, &dwSize, 0);
#else
	 rt = HttpQueryInfo(_request, (DWORD)statusCodeId, szBuf, &dwSize, 0);
#endif
	 rdata->statuscode = _wtoi(szBuf);
	 dwSize = BUF_SIZE;
	 memset(szBuf, 0, BUF_SIZE);
#ifdef USE_WINHTTP
	 rt =  WinHttpQueryHeaders(_request, (DWORD)statusTextId, 0, szBuf, &dwSize, 0);
#else
	 rt = HttpQueryInfo(_request, (DWORD)statusTextId, szBuf, &dwSize, 0);
#endif
	 rdata->statuscodetext = new wchar_t[dwSize];// szBuf;
	 wcscpy(rdata->statuscodetext, szBuf);
	 DWORD cbRead = 0;
	 char en[3] = { 0 };
#ifdef USE_WINHTTP
	 rt = WinHttpReadData(_request, en, 3, &cbRead);
#else
	 rt = InternetReadFile(_request, en, 3, &cbRead);

#endif
	 //服务器加了UTF8的BOM,特殊情况特殊对待,UTF8不需要加的
	 if (en[0] == '\xef'&& en[1] == '\xbb' && en[2] == '\xbf')
	 {
		 rdata->encode = UTF8;
	 }
	 else
		 rdata->encode = UNKNOWN;
	 //WARNNING:服务器没有返回长度,不然可以直接用content length
	 //此逻辑是特殊情况特殊对待
	 rdata->data = new char[BUF_SIZE+1];
	 memset(rdata->data, 0, BUF_SIZE + 1);
#ifdef USE_WINHTTP
	 rt = WinHttpReadData(_request, rdata->data, BUF_SIZE, &cbRead);
#else
	 rt = InternetReadFile(_request,  rdata->data , BUF_SIZE, &cbRead);

#endif
	 rdata->size = cbRead;
	 if (cbRead < rdata->contextLength)
	 {
		 //没有全取，只取了BUF_SIZE个,特殊情况特殊对待
		 return false;
	 }
	return true;
}
bool hchttp::CloseRequest()
{
	if (_request)
	{
#ifdef USE_WINHTTP
		bool rt = WinHttpCloseHandle(_request);
#else
		bool rt = InternetCloseHandle(_request);
#endif
		return rt;
	}

}


bool hchttp::OneKeyGoNoWait(Method verb, const wchar_t* urlpass, const void* data, int size)
{
	if (OpenSession((L"HongChen http Module")))
	{
		if (Connect())
		{
			if (Request(verb, urlpass,data, size))
			{
				CloseRequest();
			}
			Close();
		}
		CloseSession();
	}
	return true;
}
bool hchttp::OneKeyGoWait(Method verb, const wchar_t* urlpass,const void* data, int size, RequestData* out)
{
	if (OpenSession((L"HongChen http Module")))
	{
		if (Connect())
		{
			if (Request(verb,urlpass, data, size))
			{
				
				GetRequest(out);
				CloseRequest();
			}
			Close();
		}
		CloseSession();
	}
	return true;
}