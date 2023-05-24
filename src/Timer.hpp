//
// Created by ayrat on 24/05/16.
//

#ifndef SDF_TIMER_H
#define SDF_TIMER_H


#include <ctime>


class Timer {
public:
    Timer() {
        last = origin = std::clock();
    }

    /* (restarts the counter)
       returns the number of seconds since the last call to the function (or since the creation if just created).
    */
    clock_t sec_restart() {
        clock_t elapsed = (std::clock() - last) / CLOCKS_PER_SEC;
        last = clock();
        return elapsed;
    }

    clock_t sec_from_origin() const {
        return (std::clock() - origin) / CLOCKS_PER_SEC;
    }

private:
    std::clock_t last;
    std::clock_t origin;
};


#endif //SDF_TIMER_H
