/******************************************************************************
Helper for pop-up window handling
Author: Seung-Tak Noh (seungtak.noh [at] gmail.com)
******************************************************************************/
#include <vector>
#include <string>

#ifdef _WIN32
#include <Windows.h>
#elif
// header to handle OpenFileDialog in Linux or MACOS ************************** [TODO]
#endif

using namespace std::string_literals;

namespace cvgl {

std::string GetCurrentDateTime();

bool OpenFileWindow(std::string &filePath, const std::string filter = "All files (*.*)\0*\0"s);
bool SaveFileWindow(std::string &filePath, const std::string filter = ""s);

}
