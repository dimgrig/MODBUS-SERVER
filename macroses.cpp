#include "macroses.h"

ENUM_MACRO(ENUM, ENUM1, ENUM2, ENUM3, ENUM4);

void MacrosesTest() {

    ENUM_MACRO_GET_NAMES(ENUM, ENUM1, ENUM2, ENUM3, ENUM4);

    ENUM enum_ = ENUM1;
    ASSERT_EQUAL(enum_, ENUM1);
    ASSERT_EQUAL(std::string(ENUMToString(enum_)), "ENUM1");
    enum_ = ENUM2;
    ASSERT_EQUAL(enum_, ENUM2);
    ASSERT_EQUAL(std::string(ENUMToString(enum_)), "ENUM2");
    ASSERT_EQUAL(ENUMFromString("ENUM4"), ENUM4);
    ASSERT_EQUAL(std::string(ENUMToString(ENUMFromString("ENUM4"))), "ENUM4");
    enum_ = ENUMFromString("ENUM3");
    ASSERT_EQUAL(enum_, ENUM3);
    ASSERT_EQUAL(std::string(ENUMToString(enum_)), "ENUM3");

}
