#pragma once
#include <string>
#include <vector>
#include <rapidjson/fwd.h>

namespace JT
{
    std::wstring wstring(const rapidjson::Value &str);
    std::string string(const rapidjson::Value &str);
}; // namespace StringUtil