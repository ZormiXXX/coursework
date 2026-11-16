#include "avl_set.hpp"

#include <cmath>
#include <complex>
#include <iostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {
int checks = 0;

void verify(bool condition, const std::string& label) {
    ++checks;
    if (!condition) throw std::runtime_error(label);
}

struct Student {
    int id;
    std::string name;
};

struct StudentOrder {
    bool operator()(const Student& left, const Student& right) const { return left.id < right.id; }
};

struct Teacher {
    int personnelNumber;
    std::string department;
};

struct TeacherOrder {
    bool operator()(const Teacher& left, const Teacher& right) const {
        return left.personnelNumber < right.personnelNumber;
    }
};

int ascending(int value) { return value + 1; }
int descending(int value) { return value - 1; }

void rotationsAndUpdates() {
    {
        AvlSet<int> tree;
        verify(tree.add(30), "right rotation setup 30");
        verify(tree.add(20), "right rotation setup 20");
        verify(tree.add(10), "right rotation setup 10");
        verify(tree.height() == 2 && tree.valid(), "LL correction");
        verify(tree.get(0) == 10 && tree.get(1) == 20 && tree.get(2) == 30, "LL order");
    }
    {
        AvlSet<int> tree;
        tree.add(10); tree.add(20); tree.add(30);
        verify(tree.height() == 2 && tree.valid(), "RR correction");
    }
    {
        AvlSet<int> tree;
        tree.add(30); tree.add(10); tree.add(20);
        verify(tree.height() == 2 && tree.valid(), "LR correction");
    }
    {
        AvlSet<int> tree;
        tree.add(10); tree.add(30); tree.add(20);
        verify(tree.height() == 2 && tree.valid(), "RL correction");
    }
    {
        AvlSet<int> tree;
        for (int item : {40, 20, 60, 10, 30, 50, 70, 25, 35}) tree.add(item);
        verify(tree.size() == 9, "size after inserts");
        verify(tree.has(35) && !tree.has(36), "search results");
        verify(!tree.add(35) && tree.size() == 9, "duplicate insertion");
        verify(tree.remove(40) && !tree.has(40), "remove two-child root");
        verify(tree.remove(10) && tree.remove(70), "remove leaves");
        verify(!tree.remove(999) && tree.valid(), "missing removal and invariant");
        tree.clear();
        verify(tree.empty() && tree.height() == 0 && tree.size() == 0, "clear state");
    }
}

void indexedAndFunctional() {
    AvlSet<int> tree;
    for (int value : {8, 3, 12, 1, 5, 10, 14, 4, 6}) tree.add(value);
    verify(tree.get(0) == 1 && tree.get(4) == 6 && tree.get(8) == 14, "rank access");
    bool rangeError = false;
    try { (void)tree.get(tree.size()); } catch (const IndexOutOfRange&) { rangeError = true; }
    verify(rangeError, "rank range error");
    auto mapped = tree.map([](int value) { return std::to_string(value); });
    verify(mapped.size() == tree.size() && mapped.has(std::string("10")), "map changes element type");
    auto filtered = tree.where([](int value) { return value % 2 == 0; });
    verify(filtered.size() == 6 && filtered.has(14) && !filtered.has(5), "where selection");
    verify(tree.reduce(0, [](int value, int sum) { return value + sum; }) == 63, "reduce order and result");
    auto merged = tree + filtered;
    verify(merged == tree && !(merged != tree), "set union operators");
    auto copy = tree;
    copy.remove(8);
    verify(tree.has(8) && !copy.has(8), "deep copy independence");
    verify(tree.valid() && copy.valid(), "cached metadata");
}

void walksAndText() {
    AvlSet<int> tree;
    for (int value : {4, 2, 6, 1, 3, 5, 7}) tree.add(value);
    const AvlSet<int>::Walk orders[] = {
        AvlSet<int>::Walk::KLP, AvlSet<int>::Walk::KPL,
        AvlSet<int>::Walk::LKP, AvlSet<int>::Walk::LPK,
        AvlSet<int>::Walk::PKL, AvlSet<int>::Walk::PLK
    };
    const std::string expected[] = {
        "4,2,1,3,6,5,7,", "4,6,7,5,2,3,1,", "1,2,3,4,5,6,7,",
        "1,3,2,5,7,6,4,", "7,6,5,4,3,2,1,", "7,5,6,3,1,2,4,"
    };
    for (int i = 0; i < 6; ++i) {
        std::ostringstream output;
        tree.each(orders[i], [&](int value) { output << value << ','; });
        verify(output.str() == expected[i], "traversal order " + std::to_string(i) + ": " + output.str());
    }
    const char* formats[] = {"{К}(Л)[П]", "{К}[П](Л)", "(Л){К}[П]", "(Л)[П]{К}", "[П]{К}(Л)", "[П](Л){К}"};
    for (const char* format : formats) {
        auto decoded = AvlSet<int>::from_text(tree.to_text(format), format);
        verify(decoded == tree && decoded.to_text(format) == tree.to_text(format), "format round trip");
    }
    AvlSet<int> blank;
    verify(AvlSet<int>::from_text(blank.to_text()).empty(), "empty tree round trip");
    bool badFormat = false;
    try { (void)tree.to_text("{К}(Л){К}"); } catch (const InvalidArgument&) { badFormat = true; }
    verify(badFormat, "invalid format rejection");
    bool badText = false;
    try { (void)AvlSet<int>::from_text("{8}(x)[ ]"); } catch (const InvalidArgument&) { badText = true; }
    verify(badText, "malformed text rejection");
    AvlSet<std::string> words;
    words.add("hello tree"); words.add("braces } and spaces");
    verify(AvlSet<std::string>::from_text(words.to_text()) == words, "quoted string values");
}

void subtreeAndGenericValues() {
    AvlSet<int> tree;
    for (int value : {20, 10, 30, 5, 15, 25, 35}) tree.add(value);
    auto piece = tree.subtree(10);
    verify(piece.size() == 3 && piece.has(5) && piece.has(15), "subtree extraction");
    verify(tree.contains_subtree(piece), "subtree containment");
    AvlSet<int> unrelatedShape;
    unrelatedShape.add(25); unrelatedShape.add(30);
    verify(!tree.contains_subtree(unrelatedShape), "shape-sensitive containment");
    verify(tree.subtree(500).empty(), "missing subtree yields empty tree");
    verify(tree.contains_subtree(AvlSet<int>{}), "empty tree containment");

    AvlSet<double> decimals;
    decimals.add(2.25); decimals.add(-1.5);
    verify(decimals.has(2.25) && decimals.get(0) == -1.5, "real values");
    struct ComplexOrder {
        bool operator()(const std::complex<double>& a, const std::complex<double>& b) const {
            return a.real() < b.real() || (a.real() == b.real() && a.imag() < b.imag());
        }
    };
    AvlSet<std::complex<double>, ComplexOrder> complex;
    complex.add({1.0, 2.0}); complex.add({-1.0, 0.5});
    verify(complex.size() == 2 && complex.get(0).real() == -1.0, "complex values by explicit order");
    AvlSet<Student, StudentOrder> students;
    students.add({12, "Ada"}); students.add({5, "Lin"});
    verify(students.has({12, "different name"}) && students.get(0).id == 5, "student records by identifier");
    AvlSet<Teacher, TeacherOrder> teachers;
    teachers.add({901, "math"}); teachers.add({104, "physics"});
    verify(teachers.has({901, "other department"}) && teachers.get(0).personnelNumber == 104,
           "teacher records by personnel number");
    using Function = int(*)(int);
    AvlSet<Function> functions;
    functions.add(&ascending); functions.add(&descending);
    verify(functions.size() == 2 && functions.has(&ascending), "function pointer values");
}

void randomizedReference() {
    constexpr int low = -1000;
    constexpr int width = 2001;
    int sorted[width]{};
    int length = 0;
    AvlSet<int> tree;
    std::mt19937 generator(74123);
    std::uniform_int_distribution<int> key(low, low + width - 1);
    std::uniform_int_distribution<int> action(0, 1);
    for (int step = 0; step < 20000; ++step) {
        const int value = key(generator);
        int position = 0;
        while (position < length && sorted[position] < value) ++position;
        const bool exists = position < length && sorted[position] == value;
        if (action(generator) == 0) {
            verify(tree.add(value) == !exists, "random add result");
            if (!exists) {
                for (int i = length; i > position; --i) sorted[i] = sorted[i - 1];
                sorted[position] = value;
                ++length;
            }
        } else {
            verify(tree.remove(value) == exists, "random remove result");
            if (exists) {
                for (int i = position; i + 1 < length; ++i) sorted[i] = sorted[i + 1];
                --length;
            }
        }
        if ((step % 200) == 0) {
            verify(tree.valid() && tree.size() == static_cast<std::size_t>(length), "random invariant checkpoint");
            for (int i = 0; i < length; ++i)
                verify(tree.get(static_cast<std::size_t>(i)) == sorted[i], "random model comparison");
        }
    }
    verify(tree.valid() && tree.size() == static_cast<std::size_t>(length), "random final state");
}

void allocationAccounting() {
    const auto initial = AvlSet<int>{}.live_nodes();
    {
        AvlSet<int> tree;
        for (int value = 0; value < 500; ++value) tree.add(value);
        auto copy = tree;
        auto part = tree.subtree(250);
        verify(tree.live_nodes() >= 500, "node allocation count increases");
        copy.clear();
        verify(copy.empty(), "copy releases nodes on clear");
    }
    verify(AvlSet<int>{}.live_nodes() == initial, "all owned nodes released");
}
}

int main() {
    try {
        rotationsAndUpdates();
        indexedAndFunctional();
        walksAndText();
        subtreeAndGenericValues();
        randomizedReference();
        allocationAccounting();
        std::cout << "Passed " << checks << " checks across 6 groups, including 20,000 randomized operations.\n";
    } catch (const std::exception& error) {
        std::cerr << "FAILED after " << checks << " checks: " << error.what() << '\n';
        return 1;
    }
}
