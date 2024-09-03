module;

#include <string>
#include <string_view>

#include "Utility/fixed_string.h"
#include "Claude.Util.h"

export module Claude:Constants;
import :JSON;

using std::string;
using fixstr::fixed_string;

export namespace Claude
{
    export namespace Constants
    {
        inline namespace Header
        {
            CREATE_VALUE(VersionDate, "2023-06-01");
            CREATE_VALUE(ContentType, "application/json");
        }

        export namespace Models
        {
            CREATE_VALUE(Claude_3_5_Sonnet, "claude-3-5-sonnet-20240620");
            CREATE_VALUE(Claude_3_Opus, "claude-3-5-sonnet-20240620");
            CREATE_VALUE(Claude_3_Sonnet, "claude-3-sonnet-20240229");
            CREATE_VALUE(Claude_3_Haiku, "claude-3-haiku-20240307");
        }

        inline namespace Roles
        {
            CREATE_VALUE(User, "user");
            CREATE_VALUE(Asst, "assistant");
        }

        inline namespace Types
        {
            CREATE_VALUE(Message, "message");
            CREATE_VALUE(Text, "text");
            CREATE_VALUE(Image, "image");
        }
    }
}