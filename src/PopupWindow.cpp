/******************************************************************************
Helper for pop-up window handling
Author: Seung-Tak Noh (seungtak.noh [at] gmail.com)
******************************************************************************/
#include "cvgl/PopupWindow.h"
#include <ctime>
#include <iostream>

namespace cvgl {

std::string GetCurrentDateTime()
{
	char currentDateTime[16];
	std::time_t current_time = std::time(nullptr);

#ifdef _WIN32
	tm current;
	localtime_s(&current, &current_time);

	int yyyy = current.tm_year + 1900;
	int mm = current.tm_mon + 1;
	int dd = current.tm_mday;

	int HH = current.tm_hour;
	int MM = current.tm_min;
	int ss = current.tm_sec;

	sprintf_s(currentDateTime, "%4d%02d%02d_%02d%02d%02d", yyyy, mm, dd, HH, MM, ss);
#else
	tm* current = std::localtime(&current_time);

	int yyyy = current->tm_year + 1900;
	int mm = current->tm_mon + 1;
	int dd = current->tm_mday;

	int HH = current->tm_hour;
	int MM = current->tm_min;
	int ss = current->tm_sec;

	snprintf(currentDateTime, 16, "%4d%02d%02d_%02d%02d%02d", yyyy, mm, dd, HH, MM, ss);
#endif

	return std::string(currentDateTime);
}

///////////////////////////////////////////////////////////////////////////////
// conversion between std::string and std::wstring 
// [CAUTION] it has '\0' for tokenizing
///////////////////////////////////////////////////////////////////////////////
std::wstring str2wstr(const std::string str)
{
	std::wstring wStr(str.begin(), str.end());
	return wStr;
}
std::string wstr2str(const std::wstring wstr)
{
	std::string _str = "";
	_str.assign(wstr.begin(), wstr.end());
	const char* filePath_c_str = _str.c_str();

	// conversion '\\' to '/'
	std::string str = "";
	for (int i = 0; i < _str.size(); i++) {
		char c = filePath_c_str[i];
		if (c == 92) c = 47;
		str += c;
	}

	return str;
}

///////////////////////////////////////////////////////////////////////////////
// open/save using native file dialog window
///////////////////////////////////////////////////////////////////////////////
bool OpenFileWindow(std::string &filePath, const std::string filter)
{
	// add "All files (*.*)\0*" in the end of list
	std::string filter_all(filter + "All files (*.*)\0*\0"s);

#ifdef _WIN32
	OPENFILENAME ofn;

	#if UNICODE
		static WCHAR szFile[4096];
		std::wstring _filter = str2wstr(filter_all);
	#else
		static char szFile[4096];
		std::string _filter = filter_all;
	#endif

	// common settings for open a file
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0'; // empty string as default
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = _filter.c_str();
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// open file window
	if (TRUE != GetOpenFileName(&ofn)) return false;

	#if UNICODE
		filePath = std::string(wstr2str(ofn.lpstrFile));
	#else
		filePath = std::string(ofn.lpstrFile);
	#endif

#elif
	// equivalent code in Linux or MACOS ************************************** [TODO]
	std::cerr << "ERROR: it only supports Windows ..." << std::endl;
#endif

	return true;
}
bool SaveFileWindow(std::string &filePath, const std::string filter)
{
	// get the extensions from filter string ... ****************************** [TODO] find smarter way
	std::vector<std::string> exts; exts.push_back(""); // empty string at 0-th

	auto GetFileExtension = [](const std::string& FileName)->std::string
	{
		if (FileName.find_last_of(".") != std::string::npos)
			return FileName.substr(FileName.find_last_of(".") + 1);
		return "";
	};

	std::string buf = "";
	int count = 0;
	int flip = 0;
	while (count < filter.size()) {
		char ch = filter[count];
		if ('\0' == ch) {
			flip++;
			if (flip % 2 == 0) {
				buf.push_back('\0'); // ex) "*.txt(null)"
				std::string ext = GetFileExtension(buf);
				exts.push_back(ext); // convert to "txt"
			}
			buf.clear();
		}
		else {
			buf.push_back(ch);
		}
		count++;
	}

#ifdef _WIN32
	OPENFILENAME ofn;

	// current datetime as default filename
	#if UNICODE
		static WCHAR szFile[4096];
		swprintf_s(szFile, 4096, L"%s.%s", str2wstr(GetCurrentDateTime()).c_str(), str2wstr(exts[1]).c_str());
		std::wstring _filter = str2wstr(filter);
	#else
		static char szFile[4096];
		sprintf_s(szFile, "%s.%s", GetCurrentDateTime().c_str(), exts[1].c_str());
		std::string _filter = filter;
	#endif

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = szFile;
//	ofn.lpstrFile[0] = '\0'; // empty string as default
	ofn.nMaxFile = sizeof(szFile);
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.lpstrFilter = _filter.c_str();
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_HIDEREADONLY | OFN_ENABLEHOOK | OFN_EXPLORER;

	// open file window
	if (TRUE != GetSaveFileName(&ofn)) return false;

	#if UNICODE
		filePath = std::string(wstr2str(ofn.lpstrFile));
	#else
		filePath = std::string(ofn.lpstrFile);
	#endif

	// add extension if it is empty
	if (0 == ofn.nFileExtension) {
		std::string ext = exts[ofn.nFilterIndex];
		if ("" != ext) filePath.append("." + ext);
	}

#elif
	// equivalent code in Linux or MACOS ************************************** [TODO]
	std::cerr << "ERROR: it only supports Windows ..." << std::endl;
#endif

	return true;
}

}
