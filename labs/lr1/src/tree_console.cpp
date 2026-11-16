#include "tree_console.hpp"

#include "avl_set.hpp"

#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <string>

namespace {
template<class Value>
bool readValue(const std::string& prompt, Value& value) {
    std::cout << prompt;
    if (std::cin >> value) return true;
    if (std::cin.eof()) return false;
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    throw InputError("value has the wrong format");
}

void printValues(const AvlSet<int>& tree, AvlSet<int>::Walk order) {
    bool first = true;
    tree.each(order, [&](int value) {
        if (!first) std::cout << ' ';
        first = false;
        std::cout << value;
    });
    std::cout << '\n';
}

void fillRandom(AvlSet<int>& tree) {
    std::size_t amount = 0;
    int low = 0;
    int high = 0;
    std::uint64_t seed = 0;
    if (!readValue("How many values: ", amount)) return;
    if (!readValue("Minimum value: ", low)) return;
    if (!readValue("Maximum value: ", high)) return;
    if (!readValue("Seed: ", seed)) return;
    if (low > high) throw InvalidArgument("minimum value exceeds maximum value");
    std::mt19937_64 engine(seed);
    std::uniform_int_distribution<int> distribution(low, high);
    std::size_t inserted = 0;
    for (std::size_t i = 0; i < amount; ++i) inserted += tree.add(distribution(engine));
    std::cout << "Generated " << amount << " values; inserted " << inserted
              << " unique keys. Seed: " << seed << '\n';
}

void showMenu() {
    std::cout << "\nAVL set\n"
              << "1 add   2 search   3 remove   4 print inorder\n"
              << "5 print preorder   6 print postorder   7 map: square values\n"
              << "8 where: even values   9 reduce: sum   10 get subtree\n"
              << "11 save with format   12 load with format   13 random fill\n"
              << "0 exit\n> ";
}
}

int run_console() {
    AvlSet<int> tree;
    for (;;) {
        showMenu();
        int command = 0;
        if (!readValue("", command)) return 0;
        try {
            if (command == 0) return 0;
            if (command >= 1 && command <= 3) {
                int value = 0;
                if (!readValue("Value: ", value)) return 0;
                if (command == 1) std::cout << (tree.add(value) ? "Added\n" : "Already present\n");
                if (command == 2) std::cout << (tree.has(value) ? "Found\n" : "Not found\n");
                if (command == 3) std::cout << (tree.remove(value) ? "Removed\n" : "Not found\n");
            } else if (command == 4) printValues(tree, AvlSet<int>::Walk::LKP);
            else if (command == 5) printValues(tree, AvlSet<int>::Walk::KLP);
            else if (command == 6) printValues(tree, AvlSet<int>::Walk::LPK);
            else if (command == 7) {
                auto squared = tree.map([](int value) { return value * value; });
                printValues(squared, AvlSet<int>::Walk::LKP);
            } else if (command == 8) {
                auto evens = tree.where([](int value) { return value % 2 == 0; });
                printValues(evens, AvlSet<int>::Walk::LKP);
            } else if (command == 9) {
                std::cout << "Sum: " << tree.reduce(0LL, [](int value, long long total) { return total + value; }) << '\n';
            } else if (command == 10) {
                int value = 0;
                if (!readValue("Root value: ", value)) return 0;
                auto part = tree.subtree(value);
                printValues(part, AvlSet<int>::Walk::KLP);
            } else if (command == 11 || command == 12) {
                std::string format;
                std::string text;
                std::cout << "Format (for example {К}(Л)[П]): ";
                std::getline(std::cin >> std::ws, format);
                if (command == 11) std::cout << tree.to_text(format) << '\n';
                else {
                    std::cout << "Tree text: ";
                    std::getline(std::cin, text);
                    tree = AvlSet<int>::from_text(text, format);
                    std::cout << "Loaded " << tree.size() << " values\n";
                }
            } else if (command == 13) fillRandom(tree);
            else std::cout << "Unknown menu item\n";
        } catch (const std::exception& error) {
            std::cout << "Error: " << error.what() << '\n';
        }
    }
}
