/******************************************************************************
Helper for pop-up window handling
Author: Seung-Tak Noh (seungtak.noh [at] gmail.com)
******************************************************************************/
#include <vector>
#include <string>

using namespace std::string_literals;

namespace cvgl {

std::string GetCurrentDateTime();

bool OpenFileWindow(std::string &filePath, const std::string filter = "All files (*.*)\0*\0"s, const std::string title = "");
bool SaveFileWindow(std::string &filePath, const std::string filter = ""s, const std::string title = ""s);

bool OpenFolderWindow(std::string &folderPath);

}
