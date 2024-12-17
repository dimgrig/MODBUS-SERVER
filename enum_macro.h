#ifndef ENUM_MACRO_H
#define ENUM_MACRO_H

#include <iomanip>
#include <string.h>
#include <tuple>

#ifdef QT_imit
#include "TestRunner.h"

void EnumMacroTest();
#endif

#define ENUM_MACRO_GET_NAMES(name, ...) \
    { \
        char s[] = #__VA_ARGS__; \
        char *p; \
        unsigned int size = std::tuple_size<decltype(std::make_tuple(__VA_ARGS__))>::value / 2; \
        unsigned int i = 0; \
        for (p = strtok(s,","); p != NULL; p = strtok(NULL,","), ++i) { \
            while ((*p == ' ') || (*p == '"')) { \
                p++; \
            } \
            if (i < size) { \
                name##_strings_EN[i] = p; \
                name##_strings_EN[i].pop_back(); \
            } else { \
                name##_strings[i - size] = p; \
                name##_strings[i - size].pop_back(); \
            } \
        } \
    }

#define ENUM_MACRO(name, ...) \
    enum name { __VA_ARGS__ }; \
    std::string name##_to_text(unsigned int value); \
    std::string name##_to_text_EN(unsigned int value); \
    name name##_from_text(std::string value);

#define ENUM_MACRO_CPP(name, ...) \
    static unsigned int name##_strings_size = std::tuple_size<decltype(std::make_tuple(__VA_ARGS__))>::value; \
    static std::string name##_strings[std::tuple_size<decltype(std::make_tuple(__VA_ARGS__))>::value]; \
    static std::string name##_strings_EN[std::tuple_size<decltype(std::make_tuple(__VA_ARGS__))>::value]; \
    std::string name##_to_text(unsigned int value) { \
        if (value < name##_strings_size) { \
            return name##_strings[value]; \
        } else { \
            return "Не определено"; \
        } \
    } \
    std::string name##_to_text_EN(unsigned int value) { \
        if (value < name##_strings_size) { \
            return name##_strings_EN[value]; \
        } else { \
            return "Undefined"; \
        } \
    } \
    name name##_from_text(std::string value) { \
        for (unsigned int i = 0; i < name##_strings_size; ++i) {\
            if ((strcmp(value.c_str(), name##_strings[i].c_str()) == 0) || (strcmp(value.c_str(), name##_strings_EN[i].c_str()) == 0)) { \
                return (name)i; \
            } \
        } \
        return (name)0; \
    }

#define ENUM_MACRO_FUN(name) \
    [[maybe_unused]] constexpr const char *name##_to_text(unsigned int value) { \
        for (unsigned int i = 0; i < name##_strings_size; ++i) {\
            if (name##_numbers[i] == value) { \
                return name##_strings[i]; \
            } \
        } \
        return "Не определено"; \
    } \
    [[maybe_unused]] constexpr const char *name##_to_text_EN(unsigned int value) { \
        for (unsigned int i = 0; i < name##_strings_size; ++i) {\
            if (name##_numbers[i] == value) { \
                return name##_strings_EN[i]; \
            } \
        } \
        return "Undefined"; \
    } \
    [[maybe_unused]] static name name##_from_text(const char * value) { \
        for (unsigned int i = 0; i < name##_strings_size; ++i) {\
            if ((strcmp(value, name##_strings[i]) == 0) || (strcmp(value, name##_strings_EN[i]) == 0)) { \
                return (name)name##_numbers[i]; \
            } \
        } \
        return (name)0; \
    }

#define ENUM_MACRO2(name, v1, v2, e1, e2, r1, r2, d1, d2) \
    enum name {v1, v2}; \
    constexpr unsigned int name##_strings_size = 2; \
    constexpr const unsigned int name##_numbers[] = {d1, d2}; \
    constexpr const char *name##_strings[] = {#r1, #r2}; \
    constexpr const char *name##_strings_EN[] = {#e1, #e2}; \
    ENUM_MACRO_FUN(name)

#define ENUM_MACRO3(name, v1, v2, v3, e1, e2, e3, r1, r2, r3, d1, d2, d3) \
    enum name {v1, v2, v3}; \
    constexpr unsigned int name##_strings_size = 3; \
    constexpr const unsigned int name##_numbers[] = {d1, d2, d3}; \
    constexpr const char *name##_strings[] = {#r1, #r2, #r3}; \
    constexpr const char *name##_strings_EN[] = {#e1, #e2, #e3}; \
    ENUM_MACRO_FUN(name)

#define ENUM_MACRO4(name, v1, v2, v3, v4, e1, e2, e3, e4, r1, r2, r3, r4, d1, d2, d3, d4) \
    enum name {v1, v2, v3, v4}; \
    constexpr unsigned int name##_strings_size = 4; \
    constexpr const unsigned int name##_numbers[] = {d1, d2, d3, d4}; \
    constexpr const char *name##_strings[] = {#r1, #r2, #r3, #r4};\
    constexpr const char *name##_strings_EN[] = {#e1, #e2, #e3, #e4};\
    ENUM_MACRO_FUN(name)

#define ENUM_MACRO5(name, v1, v2, v3, v4, v5, e1, e2, e3, e4, e5, r1, r2, r3, r4, r5, d1, d2, d3, d4, d5) \
    enum name {v1, v2, v3, v4, v5}; \
    constexpr unsigned int name##_strings_size = 5; \
    constexpr const unsigned int name##_numbers[] = {d1, d2, d3, d4, d5}; \
    constexpr const char *name##_strings[] = {#r1, #r2, #r3, #r4, #r5}; \
    constexpr const char *name##_strings_EN[] = {#e1, #e2, #e3, #e4, #e5}; \
    ENUM_MACRO_FUN(name)

#define ENUM_MACRO6(name, v1, v2, v3, v4, v5, v6, \
                          e1, e2, e3, e4, e5, e6, \
                          r1, r2, r3, r4, r5, r6, \
                          d1, d2, d3, d4, d5, d6) \
    enum name {v1, v2, v3, v4, v5, v6}; \
    constexpr unsigned int name##_strings_size = 6; \
    constexpr const unsigned int name##_numbers[] = {d1, d2, d3, d4, d5, d6}; \
    constexpr const char *name##_strings[] = {#r1, #r2, #r3, #r4, #r5, #r6}; \
    constexpr const char *name##_strings_EN[] = {#e1, #e2, #e3, #e4, #e5, #e6}; \
    ENUM_MACRO_FUN(name)

#define ENUM_MACRO7(name, v1, v2, v3, v4, v5, v6, v7, \
                          e1, e2, e3, e4, e5, e6, e7, \
                          r1, r2, r3, r4, r5, r6, r7, \
                          d1, d2, d3, d4, d5, d6, d7) \
    enum name {v1, v2, v3, v4, v5, v6, v7}; \
    constexpr unsigned int name##_strings_size = 7; \
    constexpr const unsigned int name##_numbers[] = {d1, d2, d3, d4, d5, d6, d7}; \
    constexpr const char *name##_strings[] = {#r1, #r2, #r3, #r4, #r5, #r6, #r7}; \
    constexpr const char *name##_strings_EN[] = {#e1, #e2, #e3, #e4, #e5, #e6, #e7}; \
    ENUM_MACRO_FUN(name)

#define ENUM_MACRO8(name, v1, v2, v3, v4, v5, v6, v7, v8, \
                          e1, e2, e3, e4, e5, e6, e7, e8, \
                          r1, r2, r3, r4, r5, r6, r7, r8, \
                          d1, d2, d3, d4, d5, d6, d7, d8) \
    enum name {v1, v2, v3, v4, v5, v6, v7, v8}; \
    constexpr unsigned int name##_strings_size = 8; \
    constexpr const unsigned int name##_numbers[] = {d1, d2, d3, d4, d5, d6, d7, d8}; \
    constexpr const char *name##_strings[] = {#r1, #r2, #r3, #r4, #r5, #r6, #r7, #r8}; \
    constexpr const char *name##_strings_EN[] = {#e1, #e2, #e3, #e4, #e5, #e6, #e7, #e8}; \
    ENUM_MACRO_FUN(name)

#define ENUM_MACRO9(name, v1, v2, v3, v4, v5, v6, v7, v8, v9, \
                          e1, e2, e3, e4, e5, e6, e7, e8, e9, \
                          r1, r2, r3, r4, r5, r6, r7, r8, r9, \
                          d1, d2, d3, d4, d5, d6, d7, d8, d9) \
    enum name {v1, v2, v3, v4, v5, v6, v7, v8, v9}; \
    constexpr unsigned int name##_strings_size = 9; \
    constexpr const unsigned int name##_numbers[] = {d1, d2, d3, d4, d5, d6, d7, d8, d9}; \
    constexpr const char *name##_strings[] = {#r1, #r2, #r3, #r4, #r5, #r6, #r7, #r8, #r9}; \
    constexpr const char *name##_strings_EN[] = {#e1, #e2, #e3, #e4, #e5, #e6, #e7, #e8, #e9}; \
    ENUM_MACRO_FUN(name)

#define ENUM_MACRO10(name, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, \
                           e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, \
                           r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, \
                           d1, d2, d3, d4, d5, d6, d7, d8, d9, d10) \
    enum name {v1, v2, v3, v4, v5, v6, v7, v8, v9, v10}; \
    constexpr unsigned int name##_strings_size = 10; \
    constexpr const unsigned int name##_numbers[] = {d1, d2, d3, d4, d5, d6, d7, d8, d9, d10}; \
    constexpr const char *name##_strings[] = {#r1, #r2, #r3, #r4, #r5, #r6, #r7, #r8, #r9, #r10}; \
    constexpr const char *name##_strings_EN[] = {#e1, #e2, #e3, #e4, #e5, #e6, #e7, #e8, #e9, #e10}; \
    ENUM_MACRO_FUN(name)

#define ENUM_MACRO11(name, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, \
                           e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, \
                           r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, \
                           d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11) \
    enum name {v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11}; \
    constexpr unsigned int name##_strings_size = 11; \
    constexpr const unsigned int name##_numbers[] = {d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11}; \
    constexpr const char *name##_strings[] = {#r1, #r2, #r3, #r4, #r5, #r6, #r7, #r8, #r9, #r10, #r11}; \
    constexpr const char *name##_strings_EN[] = {#e1, #e2, #e3, #e4, #e5, #e6, #e7, #e8, #e9, #e10, #e11}; \
    ENUM_MACRO_FUN(name)

#define ENUM_MACRO12(name, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, \
                           e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, e12, \
                           r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, \
                           d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12) \
    enum name {v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12}; \
    constexpr unsigned int name##_strings_size = 12; \
    constexpr const unsigned int name##_numbers[] = {d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12}; \
    constexpr const char *name##_strings[] = {#r1, #r2, #r3, #r4, #r5, #r6, #r7, #r8, #r9, #r10, #r11, #r12}; \
    constexpr const char *name##_strings_EN[] = {#e1, #e2, #e3, #e4, #e5, #e6, #e7, #e8, #e9, #e10, #e11, #e12}; \
    ENUM_MACRO_FUN(name)

#define ENUM_MACRO15(name, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, \
                           e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, e12, e13, e14, e15, \
                           r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15, \
                           d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12, d13, d14, d15) \
    enum name {v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15}; \
    constexpr unsigned int name##_strings_size = 15; \
    constexpr const unsigned int name##_numbers[] = {d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12, d13, d14, d15}; \
    constexpr const char *name##_strings[] = {#r1, #r2, #r3, #r4, #r5, #r6, #r7, #r8, #r9, #r10, #r11, #r12, #r13, #r14, #r15}; \
    constexpr const char *name##_strings_EN[] = {#e1, #e2, #e3, #e4, #e5, #e6, #e7, #e8, #e9, #e10, #e11, #e12, #e13, #e14, #e15}; \
    ENUM_MACRO_FUN(name)

#define ENUM_MACRO19(name, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, \
                           e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, e12, e13, e14, e15, e16, e17, e18, e19, \
                           r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15, r16, r17, r18, r19, \
                           d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12, d13, d14, d15, d16, d17, d18, d19) \
    enum name {v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19}; \
    constexpr unsigned int name##_strings_size = 19; \
    constexpr const unsigned int name##_numbers[] = {d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12, d13, d14, d15, d16, d17, d18, d19}; \
    constexpr const char *name##_strings[] = {#r1, #r2, #r3, #r4, #r5, #r6, #r7, #r8, #r9, #r10, #r11, #r12, #r13, #r14, #r15, #r16, #r17, #r18, #r19}; \
    constexpr const char *name##_strings_EN[] = {#e1, #e2, #e3, #e4, #e5, #e6, #e7, #e8, #e9, #e10, #e11, #e12, #e13, #e14, #e15, #e16, #e17, #e18, #e19}; \
    ENUM_MACRO_FUN(name)

#define ENUM_MACRO20(name, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, \
                           e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, e12, e13, e14, e15, e16, e17, e18, e19, e20, \
                           r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15, r16, r17, r18, r19, r20, \
                           d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12, d13, d14, d15, d16, d17, d18, d19, d20) \
    enum name {v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20}; \
    constexpr unsigned int name##_strings_size = 20; \
    constexpr const unsigned int name##_numbers[] = {d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12, d13, d14, d15, d16, d17, d18, d19, d20}; \
    constexpr const char *name##_strings[] = {#r1, #r2, #r3, #r4, #r5, #r6, #r7, #r8, #r9, #r10, #r11, #r12, #r13, #r14, #r15, #r16, #r17, #r18, #r19, #r20}; \
    constexpr const char *name##_strings_EN[] = {#e1, #e2, #e3, #e4, #e5, #e6, #e7, #e8, #e9, #e10, #e11, #e12, #e13, #e14, #e15, #e16, #e17, #e18, #e19, #e20}; \
    ENUM_MACRO_FUN(name)

#define ENUM_MACRO21(name, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, \
                           e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, e12, e13, e14, e15, e16, e17, e18, e19, e20, e21, \
                           r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15, r16, r17, r18, r19, r20, r21, \
                           d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12, d13, d14, d15, d16, d17, d18, d19, d20, d21) \
    enum name {v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21}; \
    constexpr unsigned int name##_strings_size = 21; \
    constexpr const unsigned int name##_numbers[] = {d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12, d13, d14, d15, d16, d17, d18, d19, d20, d21}; \
    constexpr const char *name##_strings[] = {#r1, #r2, #r3, #r4, #r5, #r6, #r7, #r8, #r9, #r10, #r11, #r12, #r13, #r14, #r15, #r16, #r17, #r18, #r19, #r20, #r21}; \
    constexpr const char *name##_strings_EN[] = {#e1, #e2, #e3, #e4, #e5, #e6, #e7, #e8, #e9, #e10, #e11, #e12, #e13, #e14, #e15, #e16, #e17, #e18, #e19, #e20, #e21}; \
    ENUM_MACRO_FUN(name)

#define ENUM_MACRO22(name, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, \
                           e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, e12, e13, e14, e15, e16, e17, e18, e19, e20, e21, e22, \
                           r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15, r16, r17, r18, r19, r20, r21, r22, \
                           d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12, d13, d14, d15, d16, d17, d18, d19, d20, d21, d22) \
    enum name {v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22}; \
    constexpr unsigned int name##_strings_size = 22; \
    constexpr const unsigned int name##_numbers[] = {d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12, d13, d14, d15, d16, d17, d18, d19, d20, d21, d22}; \
    constexpr const char *name##_strings[] = {#r1, #r2, #r3, #r4, #r5, #r6, #r7, #r8, #r9, #r10, #r11, #r12, #r13, #r14, #r15, #r16, #r17, #r18, #r19, #r20, #r21, #r22}; \
    constexpr const char *name##_strings_EN[] = {#e1, #e2, #e3, #e4, #e5, #e6, #e7, #e8, #e9, #e10, #e11, #e12, #e13, #e14, #e15, #e16, #e17, #e18, #e19, #e20, #e21, #e22}; \
    ENUM_MACRO_FUN(name)

#define ENUM_MACRO23(name, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, \
                           e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, e12, e13, e14, e15, e16, e17, e18, e19, e20, e21, e22, e23, \
                           r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15, r16, r17, r18, r19, r20, r21, r22, r23, \
                           d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12, d13, d14, d15, d16, d17, d18, d19, d20, d21, d22, d23) \
    enum name {v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23}; \
    constexpr unsigned int name##_strings_size = 23; \
    constexpr const unsigned int name##_numbers[] = {d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12, d13, d14, d15, d16, d17, d18, d19, d20, d21, d22, d23}; \
    constexpr const char *name##_strings[] = {#r1, #r2, #r3, #r4, #r5, #r6, #r7, #r8, #r9, #r10, #r11, #r12, #r13, #r14, #r15, #r16, #r17, #r18, #r19, #r20, #r21, #r22, #r23}; \
    constexpr const char *name##_strings_EN[] = {#e1, #e2, #e3, #e4, #e5, #e6, #e7, #e8, #e9, #e10, #e11, #e12, #e13, #e14, #e15, #e16, #e17, #e18, #e19, #e20, #e21, #e22, #e23}; \
    ENUM_MACRO_FUN(name)

#define ENUM_MACRO26(name, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, \
                           e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, e12, e13, e14, e15, e16, e17, e18, e19, e20, e21, e22, e23, e24, e25, e26, \
                           r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15, r16, r17, r18, r19, r20, r21, r22, r23, r24, r25, r26, \
                           d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12, d13, d14, d15, d16, d17, d18, d19, d20, d21, d22, d23, d24, d25, d26) \
    enum name             {v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26}; \
    constexpr unsigned int name##_strings_size = 26; \
    constexpr const unsigned int name##_numbers[] = {d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12, d13, d14, d15, d16, d17, d18, d19, d20, d21, d22, d23, d24, d25, d26}; \
    constexpr const char *name##_strings[] =    {#r1, #r2, #r3, #r4, #r5, #r6, #r7, #r8, #r9, #r10, #r11, #r12, #r13, #r14, #r15, #r16, #r17, #r18, #r19, #r20, #r21, #r22, #r23, #r24, #r25, #r26}; \
    constexpr const char *name##_strings_EN[] = {#e1, #e2, #e3, #e4, #e5, #e6, #e7, #e8, #e9, #e10, #e11, #e12, #e13, #e14, #e15, #e16, #e17, #e18, #e19, #e20, #e21, #e22, #e23, #e24, #e25, #e26}; \
    ENUM_MACRO_FUN(name)

#define ENUM_MACRO27(name, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, \
                           e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, e12, e13, e14, e15, e16, e17, e18, e19, e20, e21, e22, e23, e24, e25, e26, e27, \
                           r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15, r16, r17, r18, r19, r20, r21, r22, r23, r24, r25, r26, r27, \
                           d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12, d13, d14, d15, d16, d17, d18, d19, d20, d21, d22, d23, d24, d25, d26, d27) \
    enum name             {v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27}; \
    constexpr unsigned int name##_strings_size = 27; \
    constexpr const unsigned int name##_numbers[] = {d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12, d13, d14, d15, d16, d17, d18, d19, d20, d21, d22, d23, d24, d25, d26, d27}; \
    constexpr const char *name##_strings[] =    {#r1, #r2, #r3, #r4, #r5, #r6, #r7, #r8, #r9, #r10, #r11, #r12, #r13, #r14, #r15, #r16, #r17, #r18, #r19, #r20, #r21, #r22, #r23, #r24, #r25, #r26, #r27}; \
    constexpr const char *name##_strings_EN[] = {#e1, #e2, #e3, #e4, #e5, #e6, #e7, #e8, #e9, #e10, #e11, #e12, #e13, #e14, #e15, #e16, #e17, #e18, #e19, #e20, #e21, #e22, #e23, #e24, #e25, #e26, #e27}; \
    ENUM_MACRO_FUN(name)

#define ENUM_MACRO28(name, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, \
                           e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, e12, e13, e14, e15, e16, e17, e18, e19, e20, e21, e22, e23, e24, e25, e26, e27, e28, \
                           r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15, r16, r17, r18, r19, r20, r21, r22, r23, r24, r25, r26, r27, r28, \
                           d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12, d13, d14, d15, d16, d17, d18, d19, d20, d21, d22, d23, d24, d25, d26, d27, d28) \
    enum name             {v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28}; \
    constexpr unsigned int name##_strings_size = 28; \
    constexpr const unsigned int name##_numbers[] = {d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12, d13, d14, d15, d16, d17, d18, d19, d20, d21, d22, d23, d24, d25, d26, d27, d28}; \
    constexpr const char *name##_strings[] =    {#r1, #r2, #r3, #r4, #r5, #r6, #r7, #r8, #r9, #r10, #r11, #r12, #r13, #r14, #r15, #r16, #r17, #r18, #r19, #r20, #r21, #r22, #r23, #r24, #r25, #r26, #r27, #r28}; \
    constexpr const char *name##_strings_EN[] = {#e1, #e2, #e3, #e4, #e5, #e6, #e7, #e8, #e9, #e10, #e11, #e12, #e13, #e14, #e15, #e16, #e17, #e18, #e19, #e20, #e21, #e22, #e23, #e24, #e25, #e26, #e27, #e28}; \
    ENUM_MACRO_FUN(name)

#define ENUM_MACRO33(name, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, \
                           e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, e12, e13, e14, e15, e16, e17, e18, e19, e20, e21, e22, e23, e24, e25, e26, e27, e28, e29, e30, e31, e32, e33, \
                           r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15, r16, r17, r18, r19, r20, r21, r22, r23, r24, r25, r26, r27, r28, r29, r30, r31, r32, r33, \
                           d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12, d13, d14, d15, d16, d17, d18, d19, d20, d21, d22, d23, d24, d25, d26, d27, d28, d29, d30, d31, d32, d33) \
    enum name             {v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33}; \
    constexpr unsigned int name##_strings_size = 33; \
    constexpr const unsigned int name##_numbers[] = {d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12, d13, d14, d15, d16, d17, d18, d19, d20, d21, d22, d23, d24, d25, d26, d27, d28, d29, d30, d31, d32, d33}; \
    constexpr const char *name##_strings[] =    {#r1, #r2, #r3, #r4, #r5, #r6, #r7, #r8, #r9, #r10, #r11, #r12, #r13, #r14, #r15, #r16, #r17, #r18, #r19, #r20, #r21, #r22, #r23, #r24, #r25, #r26, #r27, #r28, #r29, #r30, #r31, #r32, #r33}; \
    constexpr const char *name##_strings_EN[] = {#e1, #e2, #e3, #e4, #e5, #e6, #e7, #e8, #e9, #e10, #e11, #e12, #e13, #e14, #e15, #e16, #e17, #e18, #e19, #e20, #e21, #e22, #e23, #e24, #e25, #e26, #e27, #e28, #e29, #e30, #e31, #e32, #e33}; \
    ENUM_MACRO_FUN(name)

#define ENUM_MACRO35(name, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, v34, v35, \
                           e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, e12, e13, e14, e15, e16, e17, e18, e19, e20, e21, e22, e23, e24, e25, e26, e27, e28, e29, e30, e31, e32, e33, e34, e35, \
                           r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15, r16, r17, r18, r19, r20, r21, r22, r23, r24, r25, r26, r27, r28, r29, r30, r31, r32, r33, r34, r35, \
                           d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12, d13, d14, d15, d16, d17, d18, d19, d20, d21, d22, d23, d24, d25, d26, d27, d28, d29, d30, d31, d32, d33, d34, d35) \
    enum name             {v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, v34, v35}; \
    constexpr unsigned int name##_strings_size = 35; \
    constexpr const unsigned int name##_numbers[] = {d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12, d13, d14, d15, d16, d17, d18, d19, d20, d21, d22, d23, d24, d25, d26, d27, d28, d29, d30, d31, d32, d33, d34, d35}; \
    constexpr const char *name##_strings[] =    {#r1, #r2, #r3, #r4, #r5, #r6, #r7, #r8, #r9, #r10, #r11, #r12, #r13, #r14, #r15, #r16, #r17, #r18, #r19, #r20, #r21, #r22, #r23, #r24, #r25, #r26, #r27, #r28, #r29, #r30, #r31, #r32, #r33, #r34, #r35}; \
    constexpr const char *name##_strings_EN[] = {#e1, #e2, #e3, #e4, #e5, #e6, #e7, #e8, #e9, #e10, #e11, #e12, #e13, #e14, #e15, #e16, #e17, #e18, #e19, #e20, #e21, #e22, #e23, #e24, #e25, #e26, #e27, #e28, #e29, #e30, #e31, #e32, #e33, #e34, #e35}; \
    ENUM_MACRO_FUN(name)

#define ENUM_MACRO36(name, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, v34, v35, v36, \
                           e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, e12, e13, e14, e15, e16, e17, e18, e19, e20, e21, e22, e23, e24, e25, e26, e27, e28, e29, e30, e31, e32, e33, e34, e35, e36, \
                           r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15, r16, r17, r18, r19, r20, r21, r22, r23, r24, r25, r26, r27, r28, r29, r30, r31, r32, r33, r34, r35, r36, \
                           d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12, d13, d14, d15, d16, d17, d18, d19, d20, d21, d22, d23, d24, d25, d26, d27, d28, d29, d30, d31, d32, d33, d34, d35, d36) \
    enum name             {v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, v34, v35, v36}; \
    constexpr unsigned int name##_strings_size = 36; \
    constexpr const unsigned int name##_numbers[] = {d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12, d13, d14, d15, d16, d17, d18, d19, d20, d21, d22, d23, d24, d25, d26, d27, d28, d29, d30, d31, d32, d33, d34, d35, d36}; \
    constexpr const char *name##_strings[] =    {#r1, #r2, #r3, #r4, #r5, #r6, #r7, #r8, #r9, #r10, #r11, #r12, #r13, #r14, #r15, #r16, #r17, #r18, #r19, #r20, #r21, #r22, #r23, #r24, #r25, #r26, #r27, #r28, #r29, #r30, #r31, #r32, #r33, #r34, #r35, #r36}; \
    constexpr const char *name##_strings_EN[] = {#e1, #e2, #e3, #e4, #e5, #e6, #e7, #e8, #e9, #e10, #e11, #e12, #e13, #e14, #e15, #e16, #e17, #e18, #e19, #e20, #e21, #e22, #e23, #e24, #e25, #e26, #e27, #e28, #e29, #e30, #e31, #e32, #e33, #e34, #e35, #e36}; \
    ENUM_MACRO_FUN(name)

ENUM_MACRO(SignalsEnum, VoltageSignal, CurrentSignal);
ENUM_MACRO(SignalsNumEnum, VoltageNumSignal = 0x77, CurrentNumSignal = 0x88);

ENUM_MACRO2(SignalsEnum2, VoltageSignal2 = 0x77, CurrentSignal2 = 0x88,
            Voltage V, Current I,
            Напряжение В, Ток А,
            0x77, 0x88);

#endif // ENUM_MACRO_H
