//
// Created by Dima on 08.03.2021.
//

#ifndef ASTRA_MACROSES_H
#define ASTRA_MACROSES_H

#include "functions.h"
#include "Logger.h"

void MacrosesTest();

#define AS_KV(x) (string_clearify((#x))).c_str() << " = " << x << "\n"

#define UNUSED(x) (void)(x)

#define SET_TRISTATE(S) \
S->exclusive(); \
S->addButton(ui->S##_Down, 2); \
S->addButton(ui->S##_Mid, 1); \
S->addButton(ui->S##_Up, 0);

#define IMITATOR_FRAME_HIDING(btn, frame) \
if (btn->isChecked()) { \
    frame->hide(); \
} else { \
    frame->show(); \
}

#define CHOICE_TRISTATE(S, S1, S2) \
if (!S->isChecked()) { \
    ui->S1##_Down->setProperty("grayed", true); \
    ui->S1##_Mid->setProperty("grayed", true); \
    ui->S1##_Up->setProperty("grayed", true); \
    ui->S2##_Down->setProperty("grayed", false); \
    ui->S2##_Mid->setProperty("grayed", false); \
    ui->S2##_Up->setProperty("grayed", false); \
} else { \
    ui->S1##_Down->setProperty("grayed", false); \
    ui->S1##_Mid->setProperty("grayed", false); \
    ui->S1##_Up->setProperty("grayed", false); \
    ui->S2##_Down->setProperty("grayed", true); \
    ui->S2##_Mid->setProperty("grayed", true); \
    ui->S2##_Up->setProperty("grayed", true); \
} \
ui->S1##_Down->setStyleSheet(ui->S1##_Down->styleSheet()); \
ui->S1##_Mid->setStyleSheet(ui->S1##_Mid->styleSheet()); \
ui->S1##_Up->setStyleSheet(ui->S1##_Up->styleSheet()); \
ui->S2##_Down->setStyleSheet(ui->S2##_Down->styleSheet()); \
ui->S2##_Mid->setStyleSheet(ui->S2##_Mid->styleSheet()); \
ui->S2##_Up->setStyleSheet(ui->S2##_Up->styleSheet());

#define CHECK_SIZE(text, x, y)                                                      \
    if (x.size() != y.size()) {                                                         \
        qCDebug(logFatal()) << text << "boost::combine sizes not equal: "          \
                            << x.size() << "!=" << y.size()                             \
                            << "(" << #x << "!=" << #y << ")";                          \
    }                                                                                   \

#define ENUM_MACRO_GET_NAMES(name, ...) \
    char s[] = #__VA_ARGS__; \
    char *p; \
    unsigned int i = 0; \
    for (p = strtok(s,","); p != NULL; p = strtok(NULL,","), i++) { \
        while (*p == ' ') { \
            p++; \
        } \
        name##Strings[i] = p; \
    }

#define ENUM_MACRO(name, ...) \
    enum name { __VA_ARGS__ }; \
    char *name##Strings[std::tuple_size<decltype(std::make_tuple(__VA_ARGS__))>::value]; \
    const char *name##ToString(int value) { return name##Strings[value]; } \
    name name##FromString(std::string value) { \
        for (unsigned int i = 0; i < (std::tuple_size<decltype(std::make_tuple(__VA_ARGS__))>::value); ++i) {\
            if (strcmp(value.c_str(), name##Strings[i]) == 0) { \
                return (name)i; \
            } \
        } \
        return (name)0; \
    }


#endif //ASTRA_MACROSES_H
