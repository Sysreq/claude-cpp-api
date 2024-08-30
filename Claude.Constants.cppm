module;

#include <string>

#include "Utility/fixed_string.h"

export module Claude:Constants;
import :Json;

using std::string_view;
using std::string;

using fixstr::fixed_string;

export namespace Claude
{
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
            constexpr fixed_string    Text { "text" };
            constexpr fixed_string    Image{ "image" };
        }
	}

    using namespace JSON;
    export namespace Fields
    {
        export inline namespace Message
        {
            using Type = Parameter<"type", string>;
            using Text = Parameter<"text", string>;
            using Role = Parameter<"role", string>;

            using TextContent = Object<Default<Type, Constants::Types::Text>, Text>;
            using Content = Parameter<"content", List<Fields::TextContent>>;
        }

        export inline namespace Transaction
        {
            using Model = Parameter<"model", string>;
            using System = Parameter<"system", string>;

            using Tokens = Parameter<"max_tokens", int>;
            using Top_K = Parameter<"top_k", int>;
            using Top_P = Parameter<"top_p", int>;

            using Temperature = Parameter<"temperature", float>;

            using MessageContent = Parameter<"content", List<Text>>;
        }
    }

    export using Message = Object<Default<Fields::Role, Constants::User>, Fields::Content>;

    export namespace Fields
    {
        using Messages = Parameter<"messages", List<Claude::Message>>;
    }


    export using Transaction = Object<
                    Default<Fields::Model, Constants::Claude_3_5_Sonnet>,
                    Default<Fields::Tokens, 1024>,
                    Fields::System,
                    Fields::Temperature,
                    Fields::Top_K,
                    Fields::Top_P,
                    Fields::Messages>;
}