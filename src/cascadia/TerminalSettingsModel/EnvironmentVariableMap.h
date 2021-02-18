#pragma once

#include "EnvironmentVariableMap.g.h"
#include "../inc/cppwinrt_utils.h"
#include <winrt/impl/Windows.Foundation.Collections.2.h>

using namespace winrt::Windows::Foundation::Collections;

namespace winrt::Microsoft::Terminal::Settings::Model::implementation
{
    struct EnvironmentVariableMap : EnvironmentVariableMapT<EnvironmentVariableMap>
    {
        EnvironmentVariableMap() = default;

        static com_ptr<EnvironmentVariableMap> FromJson(const Json::Value& json);
        void LayerJson(const Json::Value& json);
        Json::Value ToJson() const;

        com_array<winrt::hstring> Keys() const noexcept;
        com_array<winrt::hstring> RawValues() const noexcept;
        com_array<winrt::hstring> ResolvedValues() const noexcept;
        void SetValue(winrt::hstring key, winrt::hstring value) noexcept;
        StringMap GetResolvedEnvironmentVariables();
    private:
        void ResolveEnvironmentVariables();
        std::wstring ResolveEnvironmentVariableValue(const std::wstring& key, std::list<std::wstring> seenKeys);

        class VariablePair
        {
        public:
            VariablePair(std::wstring rawValueIn) :
                rawValue(std::move(rawValueIn))
            {
            }
            VariablePair() = default;
            virtual ~VariablePair() = default;
            VariablePair& operator=(const VariablePair&) = default;
            VariablePair& operator=(VariablePair&&) = default;

            std::wstring rawValue;
            std::wstring resolvedValue;
        };
        std::map<std::wstring, VariablePair> _environmentVariables;
    };
}

namespace winrt::Microsoft::Terminal::Settings::Model::factory_implementation
{
    BASIC_FACTORY(EnvironmentVariableMap);
}
