#include "tree_console.hpp"

#include "avl_set.hpp"
#include "value_examples.hpp"

#include <cstdint>
#include <complex>
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
    throw InputError("введено значение неверного типа");
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

AvlSet<int>::Walk readWalk() {
    int option = 0;
    if (!readValue("Порядок (1 КЛП, 2 КПЛ, 3 ЛКП, 4 ЛПК, 5 ПКЛ, 6 ПЛК): ", option))
        throw InputError("не задан порядок обхода");
    switch (option) {
        case 1: return AvlSet<int>::Walk::KLP;
        case 2: return AvlSet<int>::Walk::KPL;
        case 3: return AvlSet<int>::Walk::LKP;
        case 4: return AvlSet<int>::Walk::LPK;
        case 5: return AvlSet<int>::Walk::PKL;
        case 6: return AvlSet<int>::Walk::PLK;
        default: throw InvalidArgument("номер обхода должен быть от 1 до 6");
    }
}

void fillRandom(AvlSet<int>& tree) {
    std::size_t amount = 0;
    int low = 0;
    int high = 0;
    std::uint64_t seed = 0;
    if (!readValue("Количество значений: ", amount)) return;
    if (!readValue("Минимум: ", low)) return;
    if (!readValue("Максимум: ", high)) return;
    if (!readValue("Seed: ", seed)) return;
    if (low > high) throw InvalidArgument("минимум больше максимума");
    std::mt19937_64 engine(seed);
    std::uniform_int_distribution<int> distribution(low, high);
    std::size_t inserted = 0;
    for (std::size_t i = 0; i < amount; ++i) inserted += tree.add(distribution(engine));
    std::cout << "Сгенерировано: " << amount << ", добавлено уникальных: " << inserted
              << ", seed: " << seed << '\n';
}

void showOtherTypes() {
    AvlSet<double> realValues;
    realValues.add(3.5); realValues.add(-2.25);
    std::cout << "Вещественные числа: " << realValues.get(0) << ' ' << realValues.get(1) << '\n';

    AvlSet<std::complex<double>, ComplexOrder> complexValues;
    complexValues.add({1.0, 2.0}); complexValues.add({-1.0, 0.5});
    std::cout << "Комплексные числа с лексикографическим порядком: ";
    complexValues.each(AvlSet<std::complex<double>, ComplexOrder>::Walk::LKP,
        [](const auto& value) { std::cout << value << ' '; });
    std::cout << '\n';

    AvlSet<std::string> words;
    words.add("дерево"); words.add("поиск");
    std::cout << "Строки: ";
    words.each(AvlSet<std::string>::Walk::LKP, [](const auto& value) { std::cout << value << ' '; });
    std::cout << '\n';

    AvlSet<IntegerFunction> functions;
    functions.add(&addOne); functions.add(&doubleValue);
    std::cout << "Указатели на функции: f(5) = " << functions.get(0)(5)
              << ", " << functions.get(1)(5) << '\n';

    AvlSet<StudentRecord, StudentOrder> students;
    students.add({204, "Ада Лавлейс"}); students.add({108, "Алан Тьюринг"});
    std::cout << "Студенты по номеру: ";
    students.each(AvlSet<StudentRecord, StudentOrder>::Walk::LKP,
        [](const StudentRecord& value) { std::cout << value.studentNumber << ':' << value.fullName << "; "; });
    std::cout << '\n';

    AvlSet<TeacherRecord, TeacherOrder> teachers;
    teachers.add({902, "Алгоритмы"}); teachers.add({405, "Математика"});
    std::cout << "Преподаватели по табельному номеру: ";
    teachers.each(AvlSet<TeacherRecord, TeacherOrder>::Walk::LKP,
        [](const TeacherRecord& value) { std::cout << value.personnelNumber << ':' << value.subject << "; "; });
    std::cout << '\n';
}

void showMenu() {
    std::cout << "\nAVL-множество\n"
              << "1 добавить   2 найти   3 удалить   4 выбрать обход и вывести\n"
              << "5 получить элемент по индексу   6 map: квадрат   7 where: чётные\n"
              << "8 reduce: сумма   9 извлечь поддерево   10 проверить вхождение поддерева\n"
              << "11 слить с другим деревом   12 сохранить   13 загрузить\n"
              << "14 заполнить случайными значениями   15 схема дерева   16 другие типы\n"
              << "0 выход\n> ";
}
}

int run_console() {
    AvlSet<int> tree;
    for (;;) {
        showMenu();
        int command = 0;
        try {
            if (!readValue("", command)) return 0;
            if (command == 0) return 0;
            if (command >= 1 && command <= 3) {
                int value = 0;
                if (!readValue("Значение: ", value)) return 0;
                if (command == 1) std::cout << (tree.add(value) ? "Добавлено\n" : "Уже есть\n");
                if (command == 2) std::cout << (tree.has(value) ? "Найдено\n" : "Не найдено\n");
                if (command == 3) std::cout << (tree.remove(value) ? "Удалено\n" : "Не найдено\n");
            } else if (command == 4) printValues(tree, readWalk());
            else if (command == 5) {
                std::size_t index = 0;
                if (!readValue("Индекс: ", index)) return 0;
                std::cout << tree.get(index) << '\n';
            } else if (command == 6) {
                auto squared = tree.map([](int value) { return value * value; });
                printValues(squared, AvlSet<int>::Walk::LKP);
            } else if (command == 7) {
                auto evens = tree.where([](int value) { return value % 2 == 0; });
                printValues(evens, AvlSet<int>::Walk::LKP);
            } else if (command == 8) {
                std::cout << "Сумма: " << tree.reduce(0LL, [](int value, long long total) { return total + value; }) << '\n';
            } else if (command == 9) {
                int value = 0;
                if (!readValue("Значение корня: ", value)) return 0;
                std::cout << tree.subtree(value).diagram();
            } else if (command == 10) {
                std::string notation;
                std::string text;
                std::cout << "Формат (например {К}(Л)[П]): ";
                std::getline(std::cin >> std::ws, notation);
                std::cout << "Текст дерева-кандидата: ";
                std::getline(std::cin, text);
                auto candidate = AvlSet<int>::from_text(text, notation);
                std::cout << (tree.contains_subtree(candidate) ? "Поддерево входит\n" : "Не входит\n");
            } else if (command == 11) {
                AvlSet<int> another;
                std::size_t amount = 0;
                if (!readValue("Количество значений второго дерева: ", amount)) return 0;
                for (std::size_t i = 0; i < amount; ++i) {
                    int value = 0;
                    if (!readValue("Значение: ", value)) return 0;
                    another.add(value);
                }
                tree = tree.merge(another);
                std::cout << "Объединение содержит " << tree.size() << " ключей\n";
            } else if (command == 12 || command == 13) {
                std::string notation;
                std::cout << "Формат (например {К}(Л)[П]): ";
                std::getline(std::cin >> std::ws, notation);
                if (command == 12) std::cout << tree.to_text(notation) << '\n';
                else {
                    std::string text;
                    std::cout << "Текст дерева: ";
                    std::getline(std::cin, text);
                    tree = AvlSet<int>::from_text(text, notation);
                    std::cout << "Загружено " << tree.size() << " ключей\n";
                }
            } else if (command == 14) fillRandom(tree);
            else if (command == 15) std::cout << tree.diagram();
            else if (command == 16) showOtherTypes();
            else std::cout << "Нет такого пункта меню\n";
        } catch (const std::exception& error) {
            std::cout << "Ошибка: " << error.what() << '\n';
        }
    }
}
