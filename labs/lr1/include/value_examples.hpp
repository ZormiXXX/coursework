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

struct StudentOrder {
    bool operator()(const StudentRecord& left, const StudentRecord& right) const {
        return left.studentNumber < right.studentNumber;
    }
};

struct TeacherOrder {
    bool operator()(const TeacherRecord& left, const TeacherRecord& right) const {
        return left.personnelNumber < right.personnelNumber;
    }
};

struct ComplexOrder {
    bool operator()(const std::complex<double>& left, const std::complex<double>& right) const {
        if (left.real() != right.real()) return left.real() < right.real();
        return left.imag() < right.imag();
    }
};

using IntegerFunction = int(*)(int);

inline int addOne(int value) { return value + 1; }
inline int doubleValue(int value) { return value * 2; }
