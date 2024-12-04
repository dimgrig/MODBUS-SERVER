#ifndef SWITCHES_H
#define SWITCHES_H

#include "config_stm.h"

#ifdef QT_imit
#include <vector>

#include "Logger.h"
#include "TestRunner.h"

#include "Modules/RMO/enums.h"
#include "Modules/RMO/mixins.h"
#include "templates.h"

#else

#endif

class Switches : public CopyMoveBase {
public:
    Switches();
    virtual ~Switches() {};

    uint32_t get_switches() const;
    void update_switches_v() const;

#ifdef QT_imit
    friend QDebug operator<<(QDebug os, const Switches& r);
    friend std::ostream& operator<<(std::ostream &os, const Switches &r);
#endif
    friend bool operator==(const Switches &lhs, const Switches &rhs);
    friend bool operator!=(const Switches &lhs, const Switches &rhs);
public:
    unsigned int S0 = 0; //:1
    unsigned int S1 = 0;
    unsigned int S2 = 0;
    unsigned int S3 = 0;

    unsigned int S4 = 0;
    unsigned int S5 = 0;
    unsigned int S6 = 0;
    unsigned int S7 = 0;

    unsigned int S8 = 0;
    unsigned int S9 = 0;
    unsigned int S10 = 0;
    unsigned int S11 = 0;

    unsigned int S12 = 0;
    unsigned int S13 = 0;
    unsigned int S14 = 0;
    unsigned int S15 = 0;

    unsigned int S16 = 0;
    unsigned int S17 = 0;
    unsigned int S18 = 0;
    unsigned int S19 = 0;

    unsigned int S20 = 0;
    unsigned int S21 = 0;
    unsigned int S22 = 0;
    unsigned int S23 = 0;

    unsigned int S24 = 0;
    unsigned int S25 = 0;
    unsigned int S26 = 0;
    unsigned int S27 = 0;

    unsigned int S28 = 0;
    unsigned int S29 = 0;
    unsigned int S30 = 0;
    unsigned int S31 = 0;

    int _switches_size = SWITCHES_MAX_SIZE;

    std::vector<std::string> switches_names_v;
    mutable std::vector<unsigned int> switches_v;
};

//class ADC_switches : public Switches {
//public:
//    ADC_switches();

//    static uint32_t ADC_switches_default_u;
//};

//class DAC_switches : public Switches {
//public:
//    DAC_switches();

//    static uint32_t DAC_switches_default_u;
//};

//static ADC_switches ADC_switches_default;
//static DAC_switches DAC_switches_default;


class U_for_switches : public Switches {
public:
    U_for_switches();

    static uint32_t U_for_switches_default_u;
};

class I_for_switches : public Switches {
public:
    I_for_switches();

    static uint32_t I_for_switches_default_u;
};

class U_rev_switches : public Switches {
public:
    U_rev_switches();

    static uint32_t U_rev_switches_default_u;
};

class I_rev_switches : public Switches {
public:
    I_rev_switches();

    static uint32_t I_rev_switches_default_u;
};

static U_for_switches U_for_switches_default;
static I_for_switches I_for_switches_default;
static U_rev_switches U_rev_switches_default;
static I_rev_switches I_rev_switches_default;

#endif // SWITCHES_H
