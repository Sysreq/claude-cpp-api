module;

#include <iostream>
#include <format>
#include <string>
#include <type_traits>
#include <vector>
#include <variant>

#include "Utility/fixed_string.h"

export module Claude:Json;

using fixstr::fixed_string;
using std::string;

export namespace Claude
{
    export namespace JSON
    {
        struct Base;
        template<fixed_string Key, typename T> class Parameter;
        template<typename... Params> class Object;
        template<typename... Params> class List;

        template<typename P>
        concept IsParameter = std::is_base_of_v<Base, P> && requires {
            typename P::Type;
        };

        template<typename P, typename ...Params>
        concept HasCreate = requires (Object<Params...> o) {
            (requires { o.template Get<Params>().template Create<P>(); } || ...);
        };

        template<typename T>
        struct Enquote {
            constexpr static T wrap(const T& value) { return value; }
        };

        template<>
        struct Enquote<string> {
            constexpr static string wrap(const string& value) { return "\"" + value + "\""; }
        };


        export struct Base {
            virtual ~Base() = default;
            virtual string print() = 0;
        };

        export template<fixed_string Key, typename T>
        class Parameter : public virtual Base {
        public:
            using Type = T;
            Parameter();
            Parameter(const T& value);
            void Set(const T& value, const bool UseWrap = true);
            T& Get();
            string print() override;

        private:
            const string m_Key;
            T m_Value;
            bool m_InUse;
        };

        export template<IsParameter P, auto DefaultValue>
        class Default: public P {
        public:
            Default() : P() {
                this->Set(DefaultValue);
            }
        };

        template<IsParameter P, fixed_string DefaultValue>
            requires (std::is_same_v<typename P::Type, string>)
        class Default<P, DefaultValue> : public P {
        public:
            Default() : P() {
                this->Set(DefaultValue.data());
            }
        };

        export template<typename... Params>
        class Object : public virtual Base, public Params... {
        public:
            Object() = default;

            template<typename P, typename T>
            void Set(const T& value) requires
                (std::is_base_of_v<P, Object>) &&
                (std::is_same_v<T, typename P::Type> || std::is_same_v<typename P::Type, string>);

            template<typename P>
            typename P::Type& Get() requires std::is_base_of_v<P, Object>;

            template<typename P, typename... Args>
            P* Create(Args&&... args) requires (HasCreate<Params, P> || ...);

            string print() override;
        };

        export template<typename... Params>
        class List : public virtual Base {
        public:
            template<typename P>
            P& Create() requires (std::is_same_v<P, Params> || ...);
            string print() override;

        private:
            std::vector<std::variant<Params...>> m_ListObjects;
        };
    }
}

using namespace Claude::JSON;

template<fixed_string Key, typename T>
Parameter<Key, T>::Parameter() : m_Key(Key.data()), m_InUse(false), m_Value() {}

template<fixed_string Key, typename T>
Parameter<Key, T>::Parameter(const T& value) : m_Key(Key.data()), m_InUse(false), m_Value() { this->Set(value); }

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

template<fixed_string Key, typename T>
string Parameter<Key, T>::print() {
    if (!m_InUse)
        return "";

    if constexpr (std::is_base_of_v<Base, T>)
        return std::format("\"{}\": {}", m_Key, m_Value.print());
    else
        return std::format("\"{}\": {}", m_Key, m_Value);
}

template<typename... Params>
template<typename P, typename T>
void Object<Params...>::Set(const T& value) requires
(std::is_base_of_v<P, Object>) &&
(std::is_same_v<T, typename P::Type> || std::is_same_v<typename P::Type, string>)
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
        //result->template Set<std::decay_t<Arg>>(arg.Get());
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
string Object<Params...>::print() {
    int first = 0;
    string temp = "";
    string result = "{";
    bool printer[]{ (result += (!(temp = Params::print()).empty() && ++first > 1 ? ", " + temp : temp), true)... };
    result += "}";
    return result;
}

template<typename... Params>
template<typename P>
P& List<Params...>::Create() requires (std::is_same_v<P, Params> || ...) {
    m_ListObjects.emplace_back(P());
    return std::get<P>(m_ListObjects.back());
}

template<typename... Params>
string List<Params...>::print() {
    std::string result = "[";
    for (size_t i = 0; i < m_ListObjects.size(); ++i) {
        if (i > 0) result += ", ";
        result += std::visit([](auto&& obj) { return obj.print(); }, m_ListObjects[i]);
    }
    result += "]";
    return result;
}