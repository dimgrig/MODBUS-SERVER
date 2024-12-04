#ifndef UNITTEST_TESTRUNNER_H
#define UNITTEST_TESTRUNNER_H

#include <sstream>
#include <stdexcept>
#include <iostream>
#include <functional>

#include "templates_cout.h"

#define ASSERT_EQUAL(x, y) {            \
  std::ostringstream os;                     \
  os << #x << " != " << #y << ", "      \
    << __FILE__ << ":" << __LINE__;     \
  AssertEqual(x, y, os.str());          \
}

#define ASSERT(x) {                     \
  std::ostringstream os;                     \
  os << #x << " is false, "             \
    << __FILE__ << ":" << __LINE__;     \
  Assert(x, os.str());                  \
}

#define RUN_TEST(tr, func) \
  tr.RunTest(func, #func)

//#define GET_NAME(f) get_name(f, #f)
//
//std::string get_name(std::string name) {
//    cout << name << endl;
//    return name;
//}
//
//std::string get_name(std::function<void()> func, std::string name) {
//    return name;
//}

template<class T, class U>
void AssertEqual(const T& t, const U& u, const std::string& hint = {}) {
    if (!(t == u)) {
        std::ostringstream os;
        os << "Assertion failed: " << t << " != " << u;
        if (!hint.empty()) {
            os << " hint: " << hint;
        }
        throw std::runtime_error(os.str());
    }
}

inline void Assert(bool b, const std::string& hint) {
    AssertEqual(b, true, hint);
}

class TestRunner {
public:
    template <class TestFunc>  //function<void()>
    void RunTest(TestFunc func, std::string test_name = "") {
        //if (test_name.empty()) {
            //test_name = (*func.target<void()>()); //GET_NAME(func);
            //test_name = GET_NAME(func);
        //}
        if (TestRunner::_enable) {
            try {
                func();
                std::cerr << test_name << " OK" << std::endl;
            } catch (std::exception &e) {
                ++fail_count;
                std::cerr << test_name << " fail: " << e.what() << std::endl;
            } catch (...) {
                ++fail_count;
                std::cerr << "Unknown exception caught" << std::endl;
            }
        }
    }

    ~TestRunner() {
        if (fail_count > 0) {
            std::cerr << fail_count << " unit tests failed. Terminate" << std::endl;
            exit(1);
        }
    }
    static bool _enable;
private:
    int fail_count = 0;
};

#endif //UNITTEST_TESTRUNNER_H
