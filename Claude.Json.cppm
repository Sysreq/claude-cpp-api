module;

#include <algorithm>
#include <format>
#include <ranges>
#include <string>
#include <type_traits>
#include <vector>
#include <iostream>
#include "Utility/fixed_string.h"

#include "Claude.Util.h"

export module Claude:JSON;

using fixstr::fixed_string;
using std::string;

template<typename T>
struct Enquote {
    constexpr static T wrap(const T& value) { return value; }
};

template<>
struct Enquote<std::string> {
    constexpr static std::string wrap(const std::string& value) { return "\"" + value + "\""; }
};

constexpr bool is_upper(char c) {
    return c >= 'A' && c <= 'Z';
}

constexpr bool is_lower(char c) {
    return c >= 'a' && c <= 'z';
}

constexpr char to_lower(char c) {
    if (c == '_') return c;
    return is_upper(c) ? c + ('a' - 'A') : c;
}

std::string ToLower(const std::string& input) {
    std::string result;

    bool lastWasLower = false;

    for (char c : input) {
        char x = to_lower(c);
        if (x != c && lastWasLower) {
            if (result.length() > 1) {
                result += '_';
            }
            lastWasLower = false;
        }
        else {
            if (c == '_')
                lastWasLower = false;
            else
                lastWasLower = true;
        }
        result += x;
    }

    return result;
}

export namespace Claude
{
    export namespace JSON
    {
        struct Base;
        template<fixed_string Key, typename T> class Parameter;
        template<typename... Params> class Object;
        template<typename... Params> class List;

        export template<typename P>
        concept IsParameter = std::is_base_of_v<Base, P> && requires {
            typename P::Type;
        };

        export template<typename O, typename P, typename T>
        concept HasParameter = (IsParameter<P> && std::is_base_of_v<P, O> &&
            (std::is_same_v<T, typename P::Type> || std::is_same_v<typename P::Type, std::string>));

        template<typename P, typename ...Params>
        concept HasCreate = requires (Object<Params...> o) {
            (requires { o.template Get<Params>().template Create<P>(); } || ...);
        };

        export struct Base {
            virtual ~Base() = default;
            virtual bool IsActive() const = 0;
            virtual size_t Serialize(char*& Destination, size_t& Remaining) = 0;
            virtual size_t Deserialize(char*& Source, size_t& Remaining) = 0;
        };

        export template<fixed_string Key, typename T>
        class Parameter : public virtual Base {
        public:
            using Type = T;
            Parameter();
            Parameter(const T& value);

            bool IsActive() const override;
            size_t Serialize(char*& Destination, size_t& Remaining) override;
            size_t Deserialize(char*& Destination, size_t& Remaining) override;

            void Set(const T& value, const bool UseWrap = true);
            T& Get();

        private:
            const string m_Key;
            T m_Value;
            bool m_InUse;
        };

        export template<typename... Params>
        class Object : public virtual Base, public Params... {
        public:
            Object();

            bool IsActive() const override;
            size_t Serialize(char*& Destination, size_t& Remaining) override;
            size_t Deserialize(char*& Destination, size_t& Remaining) override;

            template<typename P, typename T>
            void Set(const T& value) requires (HasParameter<Object, P, T>);

            template<typename P>
            P::Type& Get() requires std::is_base_of_v<P, Object>;

            template<typename P, typename... Args>
            P* Create(Args&&... args) requires (HasCreate<Params, P> || ...);
        private:
            std::vector<Base*> m_Components;
        };

        export template<typename... Params>
        class List : public virtual Base {
        public:
            List() = default;
            ~List() { for (auto ptr : m_Components) delete ptr; }

            bool IsActive() const override;
            size_t Serialize(char*& Destination, size_t& Remaining) override;
            size_t Deserialize(char*& Destination, size_t& Remaining) override;

            std::vector<Base*>& Get() { return m_Components; }

            template<typename P>
            P& Create() requires (std::is_same_v<P, Params> || ...);
        private:
            std::vector<Base *> m_Components;
        };
    }
}

using namespace Claude::JSON;

template<fixed_string Key, typename T>
Parameter<Key, T>::Parameter() : m_Key(ToLower(Key.data())), m_InUse(false), m_Value() { }

template<fixed_string Key, typename T>
Parameter<Key, T>::Parameter(const T& value) : Parameter() { this->Set(value); }

template<fixed_string Key, typename T>
void Parameter<Key, T>::Set(const T& value, const bool UseWrap) {
    m_Value = UseWrap ? Enquote<T>::wrap(value) : value;
    m_InUse = true;
}

template<fixed_string Key, typename T>
T& Parameter<Key, T>::Get() {
    if (!m_InUse) {
        m_Value = T();
        m_InUse = true;
    }
    return m_Value;
}

template<typename... Params>
Object<Params...>::Object() : Params()... {
    //(m_Components.push_back(static_cast<Base*>(static_cast<Params*>(this))), ...);
}

template<typename... Params>
template<typename P, typename T>
void Object<Params...>::Set(const T& value) requires (HasParameter<Object, P, T>)
{
    static_cast<P&>(*this).Set(value);
}

template<typename... Params>
template<typename P>
typename P::Type& Object<Params...>::Get() requires std::is_base_of_v<P, Object>
{
    return static_cast<P&>(*this).Get();
}

template<typename Result, typename Arg>
constexpr void TrySet(Result* result, Arg&& arg) {
    if constexpr (requires { result->template Set<std::decay_t<Arg>>(arg); }) {
        static_cast<std::decay_t<Arg>*>(result)->Set(arg.Get(), false);
    }
}

template<typename... Params>
template<typename P, typename... Args>
typename P* Object<Params...>::Create(Args&&... args) requires (HasCreate<Params, P> || ...)
{
    P* result = nullptr;
    bool created = (... || [&]() -> bool {
        if constexpr (requires { this->Get<Params>().template Create<P>(); }) {
            result = &(this->Get<Params>().template Create<P>());
            (..., TrySet(result, args));
            return true;
        }
            return false;
        }());

    if (!created || !result) {
        throw std::runtime_error("Unable to create P");
    }

    return result;
}

template<typename... Params>
template<typename P>
P& List<Params...>::Create() requires (std::is_same_v<P, Params> || ...) {
    auto* newObject = new P();
    m_Components.push_back(newObject);
    return *newObject;
}

template<fixed_string Key, typename T>
bool Parameter<Key, T>::IsActive() const { return m_InUse; }

template<typename... Params>
bool Object<Params...>::IsActive() const { return (... || Params::IsActive()); }

template<typename... Params>
bool List<Params...>::IsActive() const {
    if (m_Components.empty())
        return false;

    for (auto const& x : m_Components)
        if (x->IsActive())
            return true;

    return false;
}

template<typename... Args>
constexpr size_t WriteChar(char*& Destination, size_t& Capacity, Args... args) {
    const size_t Count = sizeof...(Args);
    if ((Capacity < Count))
        return false;

    Capacity -= Count;
    ((*(Destination++) = args), ...);
    return Count;
}

template<fixed_string Key, typename T>
size_t Parameter<Key, T>::Serialize(char*& Destination, size_t& Capacity) {
    if (!m_InUse)
        return 0;

    size_t Written = 0;
    if constexpr (std::is_base_of_v<Base, T>) {

        std::format_to_n_result results = std::format_to_n(Destination, Capacity, "\"{}\":", m_Key);
        Written = results.size;
        Destination += results.size;
        Capacity -= results.size;
        Written += m_Value.Serialize(Destination, Capacity);
    }
    else {
        std::format_to_n_result results = std::format_to_n(Destination, Capacity, "\"{}\":{}", m_Key, m_Value);
        Written = results.size;
        Destination += results.size;
        Capacity -= results.size;
    }

    *Destination = '\0';
    return Written;
}

template<typename... Params>
size_t Object<Params...>::Serialize(char*& Destination, size_t& Capacity) {
    if (Capacity < 2)
        return 0;

    size_t Written = WriteChar(Destination, Capacity, '{');
    (..., ((Params::IsActive() && Written > 1 ? (Written += WriteChar(Destination, Capacity, ',')) : (false)),
        (Written += Params::Serialize(Destination, Capacity))));
    Written += WriteChar(Destination, Capacity, '}');

    *(Destination) = '\0';
    return Written;
}

template<typename... Params>
size_t List<Params...>::Serialize(char*& Destination, size_t& Capacity) {
    if (Capacity < 2)
        return 0;

    size_t Written = WriteChar(Destination, Capacity, '[');
    for(auto x : m_Components) {
        if (Written > 1)
            Written += WriteChar(Destination, Capacity, ',');
        Written += x->Serialize(Destination, Capacity);
    }

    Written += WriteChar(Destination, Capacity, ']');
    *Destination = '\0';
    return Written;
}

struct _Deserializer
{
private:
    char*& m_Source;
    size_t& m_Remaining;

public:
    void Next()
    {
        while (m_Remaining > 0 && (*m_Source == ' ' || *m_Source == ','))
        {
            ++m_Source;
            --m_Remaining;
        }
    }

    _Deserializer(char*& Source, size_t& Remaining) : m_Source(Source), m_Remaining(Remaining) {} 

    bool ExpectChar(const char x)
    {
        Next();
        if (m_Remaining > 0 && *m_Source == x)
        {
            m_Source++;
            m_Remaining--;
            return true;
        }
        return false;
    }

    bool ExpectWord(const string word)
    {
        const int WordLength = word.length();
        Next();
        ExpectChar('"');
        int Comparator = std::strncmp(m_Source, word.c_str(), WordLength);
        if (m_Remaining < WordLength || Comparator != 0)
            return false;
        m_Source += WordLength;
        m_Remaining -= WordLength;
        ExpectChar('"');
        Next();
        return true;
    }
};

template<fixed_string Key, typename T>
size_t Parameter<Key, T>::Deserialize(char*& Source, size_t& Remaining)
{
    size_t Start = Remaining;
    _Deserializer input(Source, Remaining);
    if (input.ExpectWord(m_Key) && input.ExpectChar(':'))
    {
        char* NextComma = nullptr;
        if (std::is_same_v<T, string> && input.ExpectChar('\"'))
        {
            NextComma = std::strchr(Source, '\"');
            if (*(++NextComma) != ',')
                std::cout << "Expected Comma.. Found.. Not Comma.. \n";
            Source--;
        }
        else
        {
            NextComma = std::strchr(Source, ',');
        }

        if (NextComma != nullptr)
        {
            size_t Count = NextComma - Source;
            if constexpr (std::is_base_of_v<Base, T>)
            {
                m_Value.Deserialize(Source, Remaining);
            }
            else {
                if constexpr (std::is_same_v<T, string>)
                {
                    m_Value = std::string_view(Source, Count);
                }
                else  if constexpr (std::is_same_v<int, T>)
                {
                    m_Value = std::stoi(Source);
                }
                Remaining -= Count;
                Source = NextComma;
            }

            m_InUse = true;
            input.ExpectChar(',');
        }
    }
    return Start - Remaining;
}

template<typename... Params>
size_t Object<Params...>::Deserialize(char*& Source, size_t& Remaining)
{
    size_t Start = Remaining;
    _Deserializer input(Source, Remaining);
    input.ExpectChar('{');
    (Params::Deserialize(Source, Remaining), ...);
    input.ExpectChar('}');
    return Start - Remaining;
}

template<typename... Params>
size_t List<Params...>::Deserialize(char*& Source, size_t& Remaining)
{
    size_t Start = Remaining;
    _Deserializer input(Source, Remaining);
    input.ExpectChar('[');
    while (!input.ExpectChar(']'))
    {
        bool created = (... || [&]() -> bool {
            Params().Deserialize(Source, Remaining);
            return true;
            }());
    }
    return Start - Remaining;
}