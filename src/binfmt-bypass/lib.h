#pragma once

// system headers
#include <string>
#include <vector>

#define EXIT_CODE_FAILURE 0xff

int bypassBinfmtAndRunAppImage(const std::string& appimage_path, const std::vector<char*>& target_args);
