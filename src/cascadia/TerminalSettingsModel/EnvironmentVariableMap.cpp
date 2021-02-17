#include "pch.h"
#include "EnvironmentVariableMap.h"
#if __has_include("EnvironmentVariableMap.g.cpp")
#include "EnvironmentVariableMap.g.cpp"
#endif

#include "JsonUtils.h"

using namespace Microsoft::Terminal::Settings::Model;

namespace winrt::Microsoft::Terminal::Settings::Model::implementation
{
    com_ptr<EnvironmentVariableMap> EnvironmentVariableMap::FromJson(const Json::Value& json)
    {
        auto result = winrt::make_self<EnvironmentVariableMap>();
        result->LayerJson(json);
        return result;
    }

    void EnvironmentVariableMap::LayerJson(const Json::Value& json)
    {
        for (auto it = json.begin(); it != json.end(); ++it)
        {
            _environmentVariables[JsonUtils::GetValue<std::wstring>(it.key())] = VariablePair{ JsonUtils::GetValue<std::wstring>(*it) };
        }
        ResolveEnvironmentVariables();
    }

    Json::Value EnvironmentVariableMap::ToJson() const
    {
        Json::Value json{ Json::ValueType::objectValue };
        std::for_each(_environmentVariables.cbegin(), _environmentVariables.cend(), [&](const std::pair<const std::wstring, VariablePair>& pair) { JsonUtils::SetValueForKey(json, til::u16u8(pair.first), pair.second.rawValue); });
        return json;
    }

    com_array<winrt::hstring> EnvironmentVariableMap::Keys() const noexcept
    {
        winrt::com_array<winrt::hstring> result{ base::checked_cast<uint32_t>(_environmentVariables.size()) };
        std::transform(_environmentVariables.begin(), _environmentVariables.end(), result.begin(), [](const std::pair<const std::wstring, VariablePair>& pair) -> winrt::hstring { return winrt::hstring(pair.first.c_str()); });
        return result;
    }

    com_array<winrt::hstring> EnvironmentVariableMap::RawValues() const noexcept
    {
        winrt::com_array<winrt::hstring> result{ base::checked_cast<uint32_t>(_environmentVariables.size()) };
        std::transform(_environmentVariables.begin(), _environmentVariables.end(), result.begin(), [](const std::pair<const std::wstring, VariablePair>& pair) -> winrt::hstring { return winrt::hstring(pair.second.rawValue.c_str()); });
        return result;
    }

    com_array<winrt::hstring> EnvironmentVariableMap::ResolvedValues() const noexcept
    {
        winrt::com_array<winrt::hstring> result{ base::checked_cast<uint32_t>(_environmentVariables.size()) };
        std::transform(_environmentVariables.begin(), _environmentVariables.end(), result.begin(), [](const std::pair<const std::wstring, VariablePair>& pair) -> winrt::hstring { return winrt::hstring(pair.second.resolvedValue.c_str()); });
        return result;
    }
    
    void EnvironmentVariableMap::SetValue(winrt::hstring key, winrt::hstring value) noexcept
    {
    }

    void EnvironmentVariableMap::ResolveEnvironmentVariables() noexcept
    {

    }
}
