module;

#include <string>
#include <string_view>

#include "Utility/fixed_string.h"

export module Claude:Constants;
import :Json;

using std::string_view;
using std::string;

using fixstr::fixed_string;

export namespace Claude
{
	export inline namespace Constants
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
            constexpr fixed_string    TextMessage { "text" };
            constexpr fixed_string    Image{ "image" };
        }
	}

    using namespace JSON;
    export inline namespace MessageFields
    {
        using Role = Default<Parameter<"role", string>, Roles::User>;

        using Type = Parameter<"type", string>;

        using Message = Parameter<"content", string>;
        using SimpleMessage = Object<Role, Message>;

        using Text = Parameter<"text", string>;
        using Line = Object<Default<Type, Types::TextMessage>, Text>;

        using MessageLines = Parameter<"content", List<Line>>;
        using MessageList = Object<Role, MessageLines>;
    }

    export inline namespace TransactionFields
    {
        using Model = Parameter<"model", string>;
        using System = Parameter<"system", string>;

        using Tokens = Parameter<"max_tokens", int>;
        using Top_K = Parameter<"top_k", int>;
        using Top_P = Parameter<"top_p", int>;

        using Temperature = Parameter<"temperature", float>;
    }

    export inline namespace Objects
    {
        using namespace TransactionFields;
        using namespace MessageFields;

        using Messages = Parameter<"messages", List<SimpleMessage, MessageList>>;

        using Transaction = Object<
            Default<Model, Constants::Claude_3_5_Sonnet>,
            Default<Tokens, 1024>,
            System,
            Temperature,
            Top_K,
            Top_P,
            Messages>;
    }
}