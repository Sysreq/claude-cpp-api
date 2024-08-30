module;

#include <string>
#include <vector>

#include "Utility/fixed_string.h"

export module Claude:Objects;
import :Constants;
import :JSON;

using std::string;

export namespace Claude
{
    using namespace Claude::JSON;

    export inline namespace Objects
    {
        export using TextContent = Object <
                SetDefault<Fields::Type, Constants::Types::Text>,
                Fields::Text>;

        export class Content : public Parameter <"content", List<TextContent>>
        {
        public:
            size_t Serialize(char*& Destination, size_t& Remaining)
            {
                List<TextContent>& ContentList = this->Get();
                if (ContentList.Get().size() == 1)
                    if (auto item = dynamic_cast<TextContent*>(ContentList.Get()[0]))
                    {
                        Parameter<"content", string> temp;
                        temp.Set(item->Get<Fields::Text>(), false);
                        return temp.Serialize(Destination, Remaining);
                    }
                        

                return Parameter::Serialize(Destination, Remaining);
            }
        };

        export using Message = Object<Fields::Role, Content>;
        export using Messages = Parameter<"messages", List<Objects::Message>>;

        export using Transaction = Object <
            SetDefault<Fields::Model, Constants::Claude_3_5_Sonnet>, /* Required */
            SetDefault<Fields::Tokens, 1024>, /* Required */
            Fields::System,
            Fields::Temperature,
            Fields::Top_K,
            Fields::Top_P,
            Messages>; /* Required */
    }



}