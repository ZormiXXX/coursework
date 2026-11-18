#include "tree_console.hpp"

#include "avl_dictionary.hpp"
#include "avl_set.hpp"
#include "value_examples.hpp"

#include <cstdint>
#include <complex>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>
#include <string>

namespace {
bool colorsEnabled() {
    return std::getenv("NO_COLOR") == nullptr;
}

const char* style(const char* escape) {
    return colorsEnabled() ? escape : "";
}

void showRule() {
    std::cout << style("\033[90m")
              << "  ────────────────────────────────────────────────────────────────\n"
              << style("\033[0m");
}

void showHeader() {
    std::cout << '\n' << style("\033[36m")
              << "  ╭──────────────────────────────────────────────────────────────╮\n"
              << "  │  ЛАБОРАТОРНАЯ РАБОТА  /  AVL-МНОЖЕСТВО                       │\n"
              << "  │  бинарное дерево  ·  IDictionary  ·  C++20                   │\n"
              << "  ╰──────────────────────────────────────────────────────────────╯\n"
              << style("\033[0m");
}

void menuSection(const char* title) {
    std::cout << "  │\n  │  " << style("\033[32;1m") << title
              << style("\033[0m") << '\n';
}

void menuItem(int key, const char* title, const char* description) {
    std::cout << "  │  " << style("\033[33;1m") << '[' << key << ']'
              << style("\033[0m") << "  " << style("\033[1;97m") << title
              << style("\033[0m") << '\n'
              << "  │      " << style("\033[90m") << description
              << style("\033[0m") << '\n';
}

void showAction(const char* category, const char* title) {
    std::cout << '\n' << style("\033[36m") << "  ◆  " << category << "  /  "
              << style("\033[1;97m") << title << style("\033[0m") << '\n';
    showRule();
}

void showResult(const std::string& message, bool success = true) {
    std::cout << "  " << style(success ? "\033[32;1m" : "\033[33;1m")
              << (success ? "●  " : "○  ") << style("\033[0m") << message << '\n';
}

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

    AvlDictionary<int, std::complex<double>> complexValues;
    complexValues.AddOrUpdate(2, {1.0, 2.0}); complexValues.AddOrUpdate(1, {-1.0, 0.5});
    std::cout << "Комплексные числа по целочисленному ключу: ";
    complexValues.each([](int key, const auto& value) { std::cout << key << ':' << value << ' '; });
    std::cout << '\n';

    AvlSet<std::string> words;
    words.add("дерево"); words.add("поиск");
    std::cout << "Строки: ";
    words.each(AvlSet<std::string>::Walk::LKP, [](const auto& value) { std::cout << value << ' '; });
    std::cout << '\n';

    AvlDictionary<int, IntegerFunction> functions;
    functions.AddOrUpdate(1, &addOne); functions.AddOrUpdate(2, &doubleValue);
    std::cout << "Указатели на функции по ключам: f(5) = " << functions.Get(1)(5)
              << ", " << functions.Get(2)(5) << '\n';

    AvlDictionary<int, StudentRecord> students;
    students.AddOrUpdate(204, {204, "Ада Лавлейс"});
    students.AddOrUpdate(108, {108, "Алан Тьюринг"});
    std::cout << "Студенты по номеру: ";
    students.each([](int key, const StudentRecord& value) { std::cout << key << ':' << value.fullName << "; "; });
    std::cout << '\n';

    AvlDictionary<int, TeacherRecord> teachers;
    teachers.AddOrUpdate(902, {902, "Алгоритмы"}); teachers.AddOrUpdate(405, {405, "Математика"});
    std::cout << "Преподаватели по табельному номеру: ";
    teachers.each([](int key, const TeacherRecord& value) { std::cout << key << ':' << value.subject << "; "; });
    std::cout << '\n';
}

void showMenu(const AvlSet<int>& tree) {
    std::cout << '\n' << style("\033[36m")
              << "  ╭──────────────────────────────────────────────────────────────╮\n"
              << "  │  СОСТОЯНИЕ  ·  " << style("\033[1;97m") << tree.size() << " элементов"
              << style("\033[0m") << style("\033[36m") << "  ·  высота " << tree.height();
    if (tree.empty()) std::cout << "  ·  пусто";
    std::cout << "\n  ├──────────────────────────────────────────────────────────────┤\n"
              << style("\033[0m");

    menuSection("ОПЕРАЦИИ С МНОЖЕСТВОМ");
    menuItem(1, "Добавить", "вставить уникальный ключ");
    menuItem(2, "Найти", "проверить наличие ключа");
    menuItem(3, "Удалить", "удалить ключ из дерева");
    menuItem(5, "Элемент по индексу", "получить ключ в порядке возрастания");

    menuSection("ОБХОД И ПРЕОБРАЗОВАНИЯ");
    menuItem(4, "Обход", "выбрать один из шести порядков");
    menuItem(6, "map", "построить дерево квадратов");
    menuItem(7, "where", "оставить только чётные ключи");
    menuItem(8, "reduce", "вычислить сумму ключей");
    menuItem(11, "Объединить", "добавить элементы второго дерева");

    menuSection("ПОДДЕРЕВЬЯ И ТЕКСТ");
    menuItem(9, "Извлечь поддерево", "скопировать ветвь по ключу-корню");
    menuItem(10, "Проверить вхождение", "сравнить форму и ключи поддерева");
    menuItem(12, "Сохранить", "записать дерево по выбранному формату");
    menuItem(13, "Загрузить", "восстановить дерево из текста");

    menuSection("ДАННЫЕ И ДЕМОНСТРАЦИИ");
    menuItem(14, "Случайное заполнение", "задать объём, диапазон и seed");
    menuItem(15, "Показать дерево", "вывести схему ветвей");
    menuItem(16, "Другие типы и IDictionary", "комплексные числа, функции и записи");

    std::cout << "  │\n  │  " << style("\033[90m") << "[0] Выход"
              << style("\033[0m") << "\n" << style("\033[36m")
              << "  ╰──────────────────────────────────────────────────────────────╯\n"
              << style("\033[0m") << "\n  Ваш выбор: ";
}
}

int run_console() {
    AvlSet<int> tree;
    showHeader();
    for (;;) {
        showMenu(tree);
        int command = 0;
        try {
            if (!readValue("", command)) return 0;
            if (command == 0) {
                std::cout << '\n' << style("\033[36m") << "  Спасибо за работу. До встречи!"
                          << style("\033[0m") << '\n';
                return 0;
            }
            if (command >= 1 && command <= 3) {
                int value = 0;
                if (!readValue("Значение: ", value)) return 0;
                if (command == 1) {
                    showAction("МНОЖЕСТВО", "ДОБАВЛЕНИЕ");
                    const bool inserted = tree.add(value);
                    showResult(inserted ? "Ключ добавлен в дерево." : "Такой ключ уже есть.", inserted);
                }
                if (command == 2) {
                    showAction("МНОЖЕСТВО", "ПОИСК");
                    showResult(tree.has(value) ? "Ключ найден." : "Ключ не найден.", tree.has(value));
                }
                if (command == 3) {
                    showAction("МНОЖЕСТВО", "УДАЛЕНИЕ");
                    const bool removed = tree.remove(value);
                    showResult(removed ? "Ключ удалён." : "Такого ключа в дереве нет.", removed);
                }
            } else if (command == 4) {
                showAction("ОБХОД", "ПОРЯДОК УЗЛОВ");
                printValues(tree, readWalk());
            }
            else if (command == 5) {
                std::size_t index = 0;
                if (!readValue("Индекс: ", index)) return 0;
                showAction("МНОЖЕСТВО", "ЭЛЕМЕНТ ПО ИНДЕКСУ");
                std::cout << "  tree[" << index << "] = " << tree.get(index) << '\n';
            } else if (command == 6) {
                showAction("ПРЕОБРАЗОВАНИЕ", "MAP · КВАДРАТЫ");
                auto squared = tree.map([](int value) { return value * value; });
                printValues(squared, AvlSet<int>::Walk::LKP);
            } else if (command == 7) {
                showAction("ПРЕОБРАЗОВАНИЕ", "WHERE · ЧЁТНЫЕ КЛЮЧИ");
                auto evens = tree.where([](int value) { return value % 2 == 0; });
                printValues(evens, AvlSet<int>::Walk::LKP);
            } else if (command == 8) {
                showAction("ПРЕОБРАЗОВАНИЕ", "REDUCE · СУММА");
                std::cout << "  Результат: "
                          << tree.reduce(0LL, [](int value, long long total) { return total + value; }) << '\n';
            } else if (command == 9) {
                int value = 0;
                if (!readValue("Значение корня: ", value)) return 0;
                showAction("ПОДДЕРЕВО", "КОПИЯ ВЕТВИ");
                const auto extracted = tree.subtree(value);
                if (extracted.empty()) showResult("Ключ не найден; поддерево пустое.", false);
                else std::cout << extracted.diagram();
            } else if (command == 10) {
                std::string notation;
                std::string text;
                std::cout << "Формат (например {К}(Л)[П]): ";
                std::getline(std::cin >> std::ws, notation);
                std::cout << "Текст дерева-кандидата: ";
                std::getline(std::cin, text);
                auto candidate = AvlSet<int>::from_text(text, notation);
                showAction("ПОДДЕРЕВО", "ПРОВЕРКА ВХОЖДЕНИЯ");
                const bool contained = tree.contains_subtree(candidate);
                showResult(contained ? "Поддерево входит в дерево." : "Такого поддерева нет.", contained);
            } else if (command == 11) {
                showAction("МНОЖЕСТВО", "ОБЪЕДИНЕНИЕ");
                AvlSet<int> another;
                std::size_t amount = 0;
                if (!readValue("Количество значений второго дерева: ", amount)) return 0;
                for (std::size_t i = 0; i < amount; ++i) {
                    int value = 0;
                    if (!readValue("Значение: ", value)) return 0;
                    another.add(value);
                }
                tree = tree.merge(another);
                showResult("В дереве теперь " + std::to_string(tree.size()) + " уникальных ключей.");
            } else if (command == 12 || command == 13) {
                std::string notation;
                std::cout << "Формат (например {К}(Л)[П]): ";
                std::getline(std::cin >> std::ws, notation);
                if (command == 12) {
                    showAction("ТЕКСТОВОЕ ПРЕДСТАВЛЕНИЕ", "СОХРАНЕНИЕ");
                    std::cout << "  " << tree.to_text(notation) << '\n';
                }
                else {
                    std::string text;
                    std::cout << "Текст дерева: ";
                    std::getline(std::cin, text);
                    tree = AvlSet<int>::from_text(text, notation);
                    showAction("ТЕКСТОВОЕ ПРЕДСТАВЛЕНИЕ", "ЗАГРУЗКА");
                    showResult("Загружено ключей: " + std::to_string(tree.size()) + ".");
                }
            } else if (command == 14) {
                showAction("ДАННЫЕ", "СЛУЧАЙНОЕ ЗАПОЛНЕНИЕ");
                fillRandom(tree);
            } else if (command == 15) {
                showAction("ДЕРЕВО", "СХЕМА ВЕТВЕЙ");
                if (tree.empty()) showResult("Пока нет узлов для отображения.", false);
                else std::cout << tree.diagram();
            } else if (command == 16) {
                showAction("ДЕМОНСТРАЦИЯ", "ТИПЫ И IDICTIONARY");
                showOtherTypes();
            } else showResult("Нет такого пункта меню.", false);
        } catch (const std::exception& error) {
            std::cout << "  " << style("\033[31;1m") << "Ошибка: " << style("\033[0m")
                      << error.what() << '\n';
        }
    }
}
