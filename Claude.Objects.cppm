module;

#include <string>
#include <vector>
#include "Utility/fixed_string.h"

#include "Claude.Util.h"

export module Claude:Objects;
import :JSON;
import :Constants;


using std::string;
using fixstr::fixed_string;

export namespace Claude
{
    using namespace Claude::JSON;

    export namespace OverloadedFields
    {
        SIMPLE_PARAMETER(Text, string);
    }

    export namespace Fields
    {
        SIMPLE_PARAMETER(Model, string, Constants::Models::Claude_3_5_Sonnet);
        SIMPLE_PARAMETER(Role, string, Constants::Roles::User);
        SIMPLE_PARAMETER(Type, string, Constants::Types::Text);

        SIMPLE_PARAMETER(System, string);
        SIMPLE_PARAMETER(Id, string); // Needs to be "id"
        SIMPLE_PARAMETER(StopReason, string);
        SIMPLE_PARAMETER(StopSequence, string);

        SIMPLE_PARAMETER(MaxTokens, int, 1024);
        SIMPLE_PARAMETER(Top_K, int);
        SIMPLE_PARAMETER(Top_P, int);
        SIMPLE_PARAMETER(InputTokens, int);
        SIMPLE_PARAMETER(OutputTokens, int);

        SIMPLE_PARAMETER(Temperature, float);

        export using TokenCount = Object<Fields::InputTokens, Fields::OutputTokens>;
        export using Text = Object<Fields::Type, OverloadedFields::Text>;

        COMPLEX_PARAMETER_LIST(Content, Text);
        export using Message = JSON::Object<Fields::Role, Content>;
        COMPLEX_PARAMETER_LIST(Messages, Message);
    }
    export namespace Requests
    {     
            export using Model = Fields::Model;
            export using MaxTokens = Fields::MaxTokens;
            export using System = Fields::System;
            export using Role = Fields::Role;
            export using Type = Fields::Type;
            export using Top_K = Fields::Top_K;
            export using Top_P = Fields::Top_P;
            export using Temperature = Fields::Temperature;
            export using Messages = Fields::Messages;
    }

    export using Request = Object <
        Requests::Model, /* Required */
        Requests::MaxTokens, /* Required */
        Requests::System,
        Requests::Temperature,
        Requests::Top_K,
        Requests::Top_P,
        Requests::Messages>; /* Required */

        

        //export using Response = Object <
        //    Responses::Identifier,
        //    Fields::Type,
        //    Fields::Role,
        //    Fields::Model,
        //    Objects::Content,
        //    Responses::StopReason,
        //    Responses::StopSequence,
        //    Responses::Usage>;
}

using namespace Claude;

//size_t Claude::Fields::Content::Serialize(char*& Destination, size_t& Remaining)
//{
//    List<Objects::LineOfText>& ContentList = this->Get();
//    if (ContentList.Get().size() == 1)
//        if (auto item = dynamic_cast<Objects::LineOfText*>(ContentList.Get()[0]))
//        {
//            Parameter<"content", string> temp;
//            temp.Set(item->Get<Fields::Text>(), false);
//            return temp.Serialize(Destination, Remaining);
//        }
//
//    return Parameter::Serialize(Destination, Remaining);
//}