#pragma once

#define PARENS ()

#define EXPAND(...) EXPAND2(EXPAND2(EXPAND2(EXPAND2(__VA_ARGS__))))
#define EXPAND2(...) EXPAND1(EXPAND1(EXPAND1(EXPAND1(__VA_ARGS__))))
#define EXPAND1(...) __VA_ARGS__

#define INSERT_ADDER(_NAME_) inline _NAME_* Add##_NAME_() {   \
    return &this->Get().Create<_NAME_>(); }

#define MAKE_ALL(...) __VA_OPT__(EXPAND(MAKE_NEXT(INSERT_ADDER, __VA_ARGS__)))
#define MAKE_NEXT(MACRO, x, ...)  MACRO(x) __VA_OPT__(AND_AGAIN PARENS (MACRO, __VA_ARGS__))
#define AND_AGAIN() MAKE_NEXT

#define SIMPLE_PARAMETER(_NAME_, TYPE, ...) \
    export struct _NAME_ : public JSON::Parameter<#_NAME_, TYPE>        \
    { __VA_OPT__ (_NAME_() : Parameter() { this->Set(__VA_ARGS__); })   \
      inline auto Get##_NAME_() { return this->Get(); }                 \
      inline void Set##_NAME_(auto value) { this->Set(value); } }

#define COMPLEX_PARAMETER_OBJ(_NAME_, ...) \
            export struct _NAME_ : public JSON::Parameter<#_NAME_, JSON::Object<__VA_ARGS__>> \
            { auto * Get##_NAME_() { return &this->Get(); } }

#define COMPLEX_PARAMETER_LIST(_NAME_, ...) \
            export struct _NAME_ : public JSON::Parameter<#_NAME_, JSON::List<__VA_ARGS__>>   \
            {inline std::vector<JSON::Base*> * Get##_NAME_() { return &this->Get().Get();}   \
            MAKE_ALL(__VA_ARGS__) }

#define CREATE_VALUE(_NAME_, VALUE) \
    constexpr fixed_string Static_##_NAME_ { VALUE }; \
    export const char* _NAME_ = Static_##_NAME_.data()