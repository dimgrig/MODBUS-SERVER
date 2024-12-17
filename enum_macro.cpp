#include "enum_macro.h"

ENUM_MACRO_CPP(SignalsEnum, VoltageSignal, CurrentSignal);

void EnumMacroTest() {
#ifdef QT_imit
    {
        ENUM_MACRO_GET_NAMES(SignalsEnum, "Voltage V", "Current I", "Напряжение В", "Ток А");

        {
            ASSERT_EQUAL(SignalsEnum_strings_size, 2);
            SignalsEnum signal = VoltageSignal;
            ASSERT_EQUAL(SignalsEnum_from_text(SignalsEnum_to_text(signal)), signal);
            ASSERT_EQUAL(SignalsEnum_to_text(signal), "Напряжение В");
            ASSERT_EQUAL(SignalsEnum_to_text_EN(signal), "Voltage V");
            signal = CurrentSignal;
            ASSERT_EQUAL(SignalsEnum_from_text(SignalsEnum_to_text(signal)), signal);
            ASSERT_EQUAL(SignalsEnum_to_text(signal), "Ток А");
            ASSERT_EQUAL(SignalsEnum_to_text_EN(signal), "Current I");

            ASSERT_EQUAL(SignalsEnum_from_text("Напряжение В"), VoltageSignal);
            ASSERT_EQUAL(SignalsEnum_from_text("Voltage V"), VoltageSignal);
            ASSERT_EQUAL(SignalsEnum_from_text("Ток А"), CurrentSignal);
            ASSERT_EQUAL(SignalsEnum_from_text("Current I"), CurrentSignal);
        }
        {
            ASSERT_EQUAL(VoltageNumSignal, 0x77);
            ASSERT_EQUAL(CurrentNumSignal, 0x88);
        }
    }
    {
        {
            ASSERT_EQUAL(SignalsEnum2_strings_size, 2);
            SignalsEnum2 signal = VoltageSignal2;
            ASSERT_EQUAL(SignalsEnum2_from_text(SignalsEnum2_to_text(signal)), signal);
            ASSERT_EQUAL(SignalsEnum2_to_text(signal), "Напряжение В");
            ASSERT_EQUAL(SignalsEnum2_to_text_EN(signal), "Voltage V");
            signal = CurrentSignal2;
            ASSERT_EQUAL(SignalsEnum2_from_text(SignalsEnum2_to_text(signal)), signal);
            ASSERT_EQUAL(SignalsEnum2_to_text(signal), "Ток А");
            ASSERT_EQUAL(SignalsEnum2_to_text_EN(signal), "Current I");

            ASSERT_EQUAL(SignalsEnum2_from_text("Напряжение В"), VoltageSignal2);
            ASSERT_EQUAL(SignalsEnum2_from_text("Voltage V"), VoltageSignal2);
            ASSERT_EQUAL(SignalsEnum2_from_text("Ток А"), CurrentSignal2);
            ASSERT_EQUAL(SignalsEnum2_from_text("Current I"), CurrentSignal2);
        }
        {
            ASSERT_EQUAL(VoltageSignal2, 0x77);
            ASSERT_EQUAL(CurrentSignal2, 0x88);
        }
    }
#endif
}
