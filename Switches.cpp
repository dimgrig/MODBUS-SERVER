#include "Switches.h"

#ifdef QT_imit

uint32_t U_for_switches::U_for_switches_default_u = 0b1111;
uint32_t I_for_switches::I_for_switches_default_u = 0b00111;
uint32_t U_rev_switches::U_rev_switches_default_u = 0b00;
uint32_t I_rev_switches::I_rev_switches_default_u = 0b01111100100010000;

Switches::Switches() {
    //switches_names_v.reserve(SWITCHES_MAX_SIZE);
    switches_names_v.assign(SWITCHES_MAX_SIZE, "");

};

uint32_t Switches::get_switches() const {
    uint32_t res =  (this->S31 << 31) | (this->S30 << 30) | (this->S29 << 29) | (this->S28 << 28) |
                    (this->S27 << 27) | (this->S26 << 26) | (this->S25 << 25) | (this->S24 << 24) |
                    (this->S23 << 23) | (this->S22 << 22) | (this->S21 << 21) | (this->S20 << 20) |
                    (this->S19 << 19) | (this->S18 << 18) | (this->S17 << 17) | (this->S16 << 16) |
                    (this->S15 << 15) | (this->S14 << 14) | (this->S13 << 13) | (this->S12 << 12) |
                    (this->S11 << 11) | (this->S10 << 10) | (this->S9 << 9) | (this->S8 << 8) |
                    (this->S7 << 7) | (this->S6 << 6) | (this->S5 << 5) | (this->S4 << 4) |
                    (this->S3 << 3) | (this->S2 << 2) | (this->S1 << 1) | (this->S0 << 0);
    return res;
}

void Switches::update_switches_v() const {
    this->switches_v = {this->S0, this->S1, this->S2, this->S3, this->S4, this->S5, this->S6, this->S7,
                 this->S8, this->S9, this->S10, this->S11, this->S12, this->S13, this->S14, this->S15,
                 this->S16, this->S17, this->S18, this->S19, this->S20, this->S21, this->S22, this->S23,
                 this->S24, this->S25, this->S26, this->S27, this->S28, this->S29, this->S30, this->S31};
}

QDebug operator<<(QDebug os, const Switches& r) {
    std::stringstream ss;
    os << "\r\n";
    for (int i = 0; i < r._switches_size; ++i) {
        ss << r.switches_names_v.at(i) << " ";
    }
    os << ss.str();
    os << "\r\n";
    ss.str(std::string());
    r.update_switches_v();
    for (int i = 0; i < r._switches_size; ++i) {
        std::string spacer = "";
        if (r.switches_names_v.at(i).size() == 3) {
            spacer = "   ";
        } else if (r.switches_names_v.at(i).size() == 2) {
            spacer = "  ";
        }
        ss << r.switches_v.at(i) << spacer.c_str();
    }
    os << ss.str();
    os << "\r\n";
    return os;
}

std::ostream& operator<<(std::ostream &os, const Switches &r) {
    std::stringstream ss;
    os << "\r\n";
    for (int i = 0; i < r._switches_size; ++i) {
        ss << r.switches_names_v.at(i) << " ";
    }
    os << ss.str();
    os << "\r\n";
    ss.str(std::string());
    r.update_switches_v();
    for (int i = 0; i < r._switches_size; ++i) {
        std::string spacer = "";
        if (r.switches_names_v.at(i).size() == 3) {
            spacer = "   ";
        } else if (r.switches_names_v.at(i).size() == 2) {
            spacer = "  ";
        }
        ss << r.switches_v.at(i) << spacer.c_str();
    }
    os << ss.str();
    os << "\r\n";
    return os;
}

bool operator==(const Switches &lhs, const Switches &rhs) {
    return ((lhs.S0 == rhs.S0) && (lhs.S1 == rhs.S1) && (lhs.S2 == rhs.S2) && (lhs.S3 == rhs.S3) &&
            (lhs.S4 == rhs.S4) && (lhs.S5 == rhs.S5) && (lhs.S6 == rhs.S6) && (lhs.S7 == rhs.S7) &&
            (lhs.S8 == rhs.S8) && (lhs.S9 == rhs.S9) && (lhs.S10 == rhs.S10) && (lhs.S11 == rhs.S11) &&
            (lhs.S12 == rhs.S12) && (lhs.S13 == rhs.S13) && (lhs.S14 == rhs.S14) && (lhs.S15 == rhs.S15) &&
            (lhs.S16 == rhs.S16) && (lhs.S17 == rhs.S17) && (lhs.S18 == rhs.S18) && (lhs.S19 == rhs.S19) &&
            (lhs.S20 == rhs.S20) && (lhs.S21 == rhs.S21) && (lhs.S22 == rhs.S22) && (lhs.S23 == rhs.S23) &&
            (lhs.S24 == rhs.S24) && (lhs.S25 == rhs.S25) && (lhs.S26 == rhs.S26) && (lhs.S27 == rhs.S27) &&
            (lhs.S28 == rhs.S28) && (lhs.S29 == rhs.S29) && (lhs.S30 == rhs.S30) && (lhs.S31 == rhs.S31)
            );
};

bool operator!=(const Switches &lhs, const Switches &rhs) {
    return !(lhs == rhs);
};

U_for_switches::U_for_switches() :
    Switches()
{
    this->S0 = 1;
    this->S1 = 1;
    this->S2 = 1;
    this->S3 = 1;

//    switches_names_v = {
//        "C13",
//        "E3",
//        "D8"};

    switches_names_v = {
        "E6",
        "E2",
        "B5",
        "B3"};

    _switches_size = U_FOR_SWITCHES_SIZE;
}

I_for_switches::I_for_switches() :
    Switches()
{
    this->S0 = 1;
    this->S1 = 1;
    this->S2 = 1;
    this->S3 = 0;
    this->S4 = 0;

//    switches_names_v = {
//        "E5",
//        "D13",
//        "E4",
//        "E14",
//        "B2"};

    switches_names_v = {
        "E4",
        "D12",
        "E5",
        "E14",
        "E7"};

    _switches_size = I_FOR_SWITCHES_SIZE;
};

U_rev_switches::U_rev_switches() :
    Switches()
{
    this->S0 = 0;
    this->S1 = 0;

    switches_names_v = {
        "D9",
        "D8"
    };

    _switches_size = U_REV_SWITCHES_SIZE;
}

I_rev_switches::I_rev_switches() :
    Switches()
{
    this->S0 = 0;
    this->S1 = 0;
    this->S2 = 0;
    this->S3 = 0;

    this->S4 = 1;
    this->S5 = 0;
    this->S6 = 0;
    this->S7 = 0;

    this->S8 = 1;
    this->S9 = 0;
    this->S10 = 0;
    this->S11 = 1;

    this->S12 = 1;
    this->S13 = 1;
    this->S14 = 1;
    this->S15 = 1;

    this->S16 = 0;

//    switches_names_v = {
//        "B3",
//        "D5",
//        "E7",
//        "E9",

//        "D6",
//        "E12",
//        "E11",
//        "E13",

//        "E6",
//        "E10",
//        "E8",
//        "B5",

//        "E2",
//        "D1",
//        "D11",
//        "B6"};

    switches_names_v = {
        "D7",
        "D6",
        "B2",
        "E8",

        "D5",
        "E13",
        "E10",
        "E12",

        "C13",
        "E11",
        "E9",
        "B4",

        "E3",
        "D2",
        "D10",
        "B7",

        "D4"
    };

    _switches_size = I_REV_SWITCHES_SIZE;
};



#endif
