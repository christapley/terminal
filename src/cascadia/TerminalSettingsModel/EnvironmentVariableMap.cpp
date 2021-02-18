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

    StringMap EnvironmentVariableMap::GetResolvedEnvironmentVariables()
    {
        ResolveEnvironmentVariables();
        StringMap envVars{};
        for (auto it = _environmentVariables.begin(); it != _environmentVariables.end(); ++it)
        {
            envVars.Insert(it->first, it->second.resolvedValue);
        }
        return envVars;
    }

    void EnvironmentVariableMap::ResolveEnvironmentVariables()
    {
        for (auto it = _environmentVariables.begin(); it != _environmentVariables.end(); ++it)
        {
            it->second.resolvedValue = ResolveEnvironmentVariableValue(it->first, {});
        }
    }

    std::wstring EnvironmentVariableMap::ResolveEnvironmentVariableValue(const std::wstring& key, std::list<std::wstring> seenKeys)
    {
        auto it = _environmentVariables.find(key);
        if (it != _environmentVariables.end())
        {
            if (!it->second.resolvedValue.empty())
            {
                return it->second.resolvedValue;
            }
            if (std::find(seenKeys.cbegin(), seenKeys.cend(), key) != seenKeys.end())
            {
                std::wstringstream error;
                error << L"Self referencing keys in environment settings: ";
                for (auto seenKey : seenKeys)
                {
                    error << L"'" << seenKey << L"', ";
                }
                error << L"'" << key << L"'";
                throw winrt::hresult_error(WEB_E_INVALID_JSON_STRING, winrt::hstring(error.str()));
            }
            const std::wregex parameterRegex(L"\\$\\{env:(.*?)\\}");
            auto& value = it->second;
            auto textIter = value.rawValue.cbegin();
            std::wsmatch regexMatch;
            while (std::regex_search(textIter, value.rawValue.cend(), regexMatch, parameterRegex))
            {
                if (regexMatch.size() != 2)
                {
                    std::wstringstream error;
                    error << L"Unexpected regex match count '" << regexMatch.size()
                          << L"' (expected '2') when matching '" << std::wstring(textIter, value.rawValue.cend()) << L"'";
                    throw winrt::hresult_error(WEB_E_INVALID_JSON_STRING, winrt::hstring(error.str()));
                }
                value.resolvedValue += std::wstring(textIter, regexMatch[0].first);
                const auto referencedKey = regexMatch[1];
                if (referencedKey == key)
                {
                    std::wstringstream error;
                    error << L"Self referencing key '" << key << L"' in environment settings";
                    throw winrt::hresult_error(WEB_E_INVALID_JSON_STRING, winrt::hstring(error.str()));
                }
                seenKeys.push_back(key);
                value.resolvedValue += ResolveEnvironmentVariableValue(referencedKey, seenKeys);
                textIter = regexMatch[0].second;
            }
            std::copy(textIter, value.rawValue.cend(), std::back_inserter(value.resolvedValue));

            return value.resolvedValue;
        }
        const auto size = GetEnvironmentVariableW(key.c_str(), nullptr, 0);
        if (size > 0)
        {
            std::wstring value(static_cast<size_t>(size), L'\0');
            if (GetEnvironmentVariableW(key.c_str(), value.data(), size) > 0)
            {
                // Documentation (https://docs.microsoft.com/en-us/windows/win32/api/processenv/nf-processenv-getenvironmentvariablew)
                // says: "If the function succeeds, the return value is the number of characters stored in the buffer pointed to by lpBuffer,
                // not including the terminating null character."
                // However, my SystemDrive "C:" returns 3 and so without trimming we will end up with a stray NULL string terminator
                // in the string.
                value.erase(std::find_if(value.rbegin(), value.rend(), [](wchar_t ch) {
                                return ch != L'\0';
                            }).base(),
                            value.end());

                return value;
            }
        }
        return {};
    }
}
