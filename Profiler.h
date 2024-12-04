//
// Created by Dima on 14.05.2021.
//

#ifndef PROFILER_PROFILER_H
#define PROFILER_PROFILER_H

#include <chrono>
#include <iostream>
#include <string>

//using namespace std;
//using namespace std::chrono;

#define UNIQ_ID_IMPL(lineno) _a_local_var_##lineno
#define UNIQ_ID(lineno) UNIQ_ID_IMPL(lineno)

#define PROFILER(message) \
  Profiler UNIQ_ID(__LINE__){message};

class Profiler {
public:
    explicit Profiler(const std::string& msg = "")
            : message(msg + ": ")
            , start(std::chrono::steady_clock::now())
    {
    }

    ~Profiler() {
        auto finish = std::chrono::steady_clock::now();
        auto dur = finish - start;
        if (Profiler::_enable) {
            std::cerr << message
                 << std::chrono::duration_cast<std::chrono::microseconds>(dur).count()
                 << " mcs" << std::endl;
        }
    }

    static bool _enable;
private:
    std::string message;
    std::chrono::steady_clock::time_point start;
};



#endif //PROFILER_PROFILER_H
