module;

#include <string>

#include "Utility/fixed_string.h"

export module Claude:Constants;
import :JSON;

using std::string;

using fixstr::fixed_string;

export namespace Claude
{
    export inline namespace Utilities
    {
        export template<JSON::IsParameter P, auto DefaultValue>
        class SetDefault : public P {
        public:
            SetDefault() : P() {
                this->Set(DefaultValue);
            }
        };

        export template<JSON::IsParameter P, fixed_string DefaultValue>
            requires (std::is_same_v<typename P::Type, string>)
        class SetDefault<P, DefaultValue> : public P {
        public:
            SetDefault() : P() {
                this->Set(DefaultValue.data());
            }
        };
    }

    export namespace Constants
    {
        inline namespace Header
        {
            constexpr fixed_string   VersionDate{ "2023-06-01" };
            constexpr fixed_string   ContentType{ "application/json" };
        }

        inline namespace Models
        {
            constexpr fixed_string  Claude_3_5_Sonnet{ "claude-3-5-sonnet-20240620" };
            constexpr fixed_string  Claude_3_Opus{ "claude-3-5-sonnet-20240620" };
            constexpr fixed_string  Claude_3_Sonnet{ "claude-3-sonnet-20240229" };
            constexpr fixed_string  Claude_3_Haiku{ "claude-3-haiku-20240307" };
        }

        inline namespace Roles
        {
            constexpr fixed_string    User{ "user" };
            constexpr fixed_string    Asst{ "assistant" };
        }

        inline namespace Types
        {
            constexpr fixed_string    Text{ "text" };
            constexpr fixed_string    Image{ "image" };
        }
    }

    using namespace JSON;
    export namespace Fields
    {
        inline namespace Message
        {
            export using Type = Parameter<"type", string>;
            export using Text = Parameter<"text", string>;
            export using Role = Parameter<"role", string>;
        }

        inline namespace Transaction
        {
            using Model = Parameter<"model", string>;
            using System = Parameter<"system", string>;

            using Tokens = Parameter<"max_tokens", int>;
            using Top_K = Parameter<"top_k", int>;
            using Top_P = Parameter<"top_p", int>;

            using Temperature = Parameter<"temperature", float>;
        }
    }
}