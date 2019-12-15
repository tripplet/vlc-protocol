#define _WIN32_WINNT 0x0600
#define _WIN32_IE 0x0600

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <shlwapi.h>
#include <wininet.h>

#include <string>
#include <array>
#include <memory>

static bool starts_with(const std::wstring str, const std::wstring prefix)
{
	return ((prefix.size() <= str.size()) && std::equal(prefix.begin(), prefix.end(), str.begin()));
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
	auto cmdLine = std::wstring(reinterpret_cast<wchar_t*>(lpCmdLine));

	if (cmdLine.length() < 10)
	{
		return 1;
	}

	// Handle quotes
	if (cmdLine[0] == '"')
	{
		cmdLine = cmdLine.substr(1);

		auto quotePos = cmdLine.find('"');
		if (quotePos == std::string::npos)
		{
			return 2;
		}
		else
		{
			cmdLine = cmdLine.substr(0, quotePos);
		}
	}

	// Strip vlc://
	auto url = cmdLine.substr(6);

	// Only allow urls starting with http:// or https://
	if (!starts_with(url, L"http://") && !starts_with(url, L"https://"))
	{
		// protocol not allowed
		return 3;
	}

	// Some browsers don't forcibly URL-encode links with unencoded characters, so make sure at least spaces are encoded
	DWORD buflen = INTERNET_MAX_URL_LENGTH;
	std::unique_ptr<wchar_t> encoded_url(new wchar_t[INTERNET_MAX_URL_LENGTH]);

	UrlEscapeSpaces(url.c_str(), encoded_url.get(), &buflen);

	// Get vlc.exe path
	int pathSize = MAX_PATH;
	DWORD result;
	std::unique_ptr<wchar_t> vlcDirectory;

	do
	{
		vlcDirectory = std::unique_ptr<wchar_t>(new wchar_t[pathSize]);
		result = GetModuleFileName(hInstance, vlcDirectory.get(), pathSize);

		if (result == pathSize)
		{
			vlcDirectory.release();
		}

		pathSize += MAX_PATH;
	} while (result == pathSize);

	PathRemoveFileSpec(vlcDirectory.get());
	auto vlcPath = std::wstring(vlcDirectory.get()).append(L"\\vlc.exe");

	// Assemble arguments
	std::wstring arguments;
	arguments.append(L"--open \"");
	arguments.append(encoded_url.get());
	arguments.append(L"\"");
	arguments.append(L" :network-caching=20000");

	// Start vlc.exe
	auto ret = reinterpret_cast<INT_PTR>(ShellExecute(NULL, NULL, vlcPath.c_str(), arguments.c_str(), NULL, SW_SHOWNORMAL));
	if (ret <= INT_PTR(32))
	{
		return 6;
	}
	else
	{
		return 0;
	}
}


