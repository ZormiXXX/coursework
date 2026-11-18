#pragma once

#include <complex>
#include <string>

struct StudentRecord {
    int studentNumber;
    std::string fullName;
};

struct TeacherRecord {
    int personnelNumber;
    std::string subject;
};

using IntegerFunction = int(*)(int);

inline int addOne(int value) { return value + 1; }
inline int doubleValue(int value) { return value * 2; }
