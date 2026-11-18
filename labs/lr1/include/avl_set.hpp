#pragma once

#include <algorithm>
#include <cstddef>
#include <cmath>
#include <functional>
#include <iomanip>
#include <istream>
#include <memory>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

#include "Exceptions.hpp"
#include "ICollection.hpp"
#include "IEnumerable.hpp"

template<class T>
class AvlSet;

template<class T>
class TreeEnumerator;

template<class T>
class AvlSet : public ICollection<T>, public IEnumerable<T> {
    template<class>
    friend class TreeEnumerator;

    struct Revision {
        std::size_t value = 0;
        bool alive = true;
    };

    struct Branch {
        explicit Branch(const T& item) : value(item) {
#ifdef AVLSET_TRACK_NODES
            ++alive;
#endif
        }
        explicit Branch(T&& item) : value(std::move(item)) {
#ifdef AVLSET_TRACK_NODES
            ++alive;
#endif
        }
        ~Branch() {
#ifdef AVLSET_TRACK_NODES
            --alive;
#endif
        }
        T value;
        std::unique_ptr<Branch> left;
        std::unique_ptr<Branch> right;
        int level = 1;
        std::size_t count = 1;
#ifdef AVLSET_TRACK_NODES
        static inline std::size_t alive = 0;
#endif
    };

    struct Section {
        char mark;
        char open;
        char close;
    };

    struct Format {
        Section sequence[3];
    };

    std::unique_ptr<Branch> top_;
    std::size_t count_ = 0;
    mutable std::shared_ptr<Revision> revision_ = std::make_shared<Revision>();

    void ensureRevision() const {
        if (!revision_) revision_ = std::make_shared<Revision>();
    }

    static int level(const std::unique_ptr<Branch>& branch) noexcept {
        return branch ? branch->level : 0;
    }

    static std::size_t nodeCount(const std::unique_ptr<Branch>& branch) noexcept {
        return branch ? branch->count : 0;
    }

    static void refresh(Branch& branch) noexcept {
        branch.level = 1 + std::max(level(branch.left), level(branch.right));
        branch.count = 1 + nodeCount(branch.left) + nodeCount(branch.right);
    }

    static int skew(const Branch& branch) noexcept {
        return level(branch.left) - level(branch.right);
    }

    static void turnRight(std::unique_ptr<Branch>& root) {
        auto pivot = std::move(root->left);
        root->left = std::move(pivot->right);
        refresh(*root);
        pivot->right = std::move(root);
        refresh(*pivot);
        root = std::move(pivot);
    }

    static void turnLeft(std::unique_ptr<Branch>& root) {
        auto pivot = std::move(root->right);
        root->right = std::move(pivot->left);
        refresh(*root);
        pivot->left = std::move(root);
        refresh(*pivot);
        root = std::move(pivot);
    }

    static void restoreBalance(std::unique_ptr<Branch>& root) {
        if (!root) return;
        refresh(*root);
        if (skew(*root) == 2) {
            if (skew(*root->left) < 0) turnLeft(root->left);
            turnRight(root);
        } else if (skew(*root) == -2) {
            if (skew(*root->right) > 0) turnRight(root->right);
            turnLeft(root);
        }
    }

    bool addAt(std::unique_ptr<Branch>& branch, const T& value) {
        if (!branch) {
            branch = std::make_unique<Branch>(value);
            return true;
        }
        bool changed;
        if (value < branch->value) changed = addAt(branch->left, value);
        else if (branch->value < value) changed = addAt(branch->right, value);
        else return false;
        if (changed) restoreBalance(branch);
        return changed;
    }

    bool locate(const std::unique_ptr<Branch>& branch, const T& value) const {
        if (!branch) return false;
        if (value < branch->value) return locate(branch->left, value);
        if (branch->value < value) return locate(branch->right, value);
        return true;
    }

    bool removeAt(std::unique_ptr<Branch>& branch, const T& value) {
        if (!branch) return false;
        bool changed;
        if (value < branch->value) changed = removeAt(branch->left, value);
        else if (branch->value < value) changed = removeAt(branch->right, value);
        else {
            changed = true;
            if (!branch->left) branch = std::move(branch->right);
            else if (!branch->right) branch = std::move(branch->left);
            else {
                T replacement = minimum(*branch->right);
                branch->value = replacement;
                removeAt(branch->right, replacement);
            }
        }
        if (changed && branch) restoreBalance(branch);
        return changed;
    }

    static std::unique_ptr<Branch> duplicate(const std::unique_ptr<Branch>& source) {
        if (!source) return {};
        auto result = std::make_unique<Branch>(source->value);
        result->level = source->level;
        result->count = source->count;
        result->left = duplicate(source->left);
        result->right = duplicate(source->right);
        return result;
    }

    template<class Function>
    static void walk(const std::unique_ptr<Branch>& branch, int order, Function& function) {
        if (!branch) return;
        switch (order) {
            case 0: function(branch->value); walk(branch->left, order, function); walk(branch->right, order, function); break;
            case 1: function(branch->value); walk(branch->right, order, function); walk(branch->left, order, function); break;
            case 2: walk(branch->left, order, function); function(branch->value); walk(branch->right, order, function); break;
            case 3: walk(branch->left, order, function); walk(branch->right, order, function); function(branch->value); break;
            case 4: walk(branch->right, order, function); function(branch->value); walk(branch->left, order, function); break;
            default: walk(branch->right, order, function); walk(branch->left, order, function); function(branch->value); break;
        }
    }

    static const T& indexed(const Branch& branch, std::size_t index) {
        const auto leftCount = nodeCount(branch.left);
        if (index < leftCount) return indexed(*branch.left, index);
        if (index == leftCount) return branch.value;
        return indexed(*branch.right, index - leftCount - 1);
    }

    static std::unique_ptr<Branch> branchAt(const std::unique_ptr<Branch>& branch, const T& value) {
        if (!branch) return {};
        if (value < branch->value) return branchAt(branch->left, value);
        if (branch->value < value) return branchAt(branch->right, value);
        return duplicate(branch);
    }

    static bool sameBranch(const std::unique_ptr<Branch>& left, const std::unique_ptr<Branch>& right) {
        if (!left || !right) return !left && !right;
        const bool sameValue = !(left->value < right->value) && !(right->value < left->value);
        return sameValue && sameBranch(left->left, right->left)
            && sameBranch(left->right, right->right);
    }

    bool containsMatchingRoot(const std::unique_ptr<Branch>& here,
                              const std::unique_ptr<Branch>& pattern) const {
        if (!here) return false;
        if (pattern->value < here->value) return containsMatchingRoot(here->left, pattern);
        if (here->value < pattern->value) return containsMatchingRoot(here->right, pattern);
        return sameBranch(here, pattern);
    }

    static void writeValue(std::ostream& output, const T& value) {
        if constexpr (std::is_same_v<T, std::string>) output << std::quoted(value);
        else output << value;
    }

    static void writeBranch(std::ostream& output, const std::unique_ptr<Branch>& branch, const Format& format) {
        if (!branch) return;
        for (const auto& section : format.sequence) {
            output << section.open;
            if (section.mark == 'K') {
                writeValue(output, branch->value);
            } else {
                writeBranch(output, section.mark == 'L' ? branch->left : branch->right, format);
            }
            output << section.close;
        }
    }

    static T readValue(std::istream& input) {
        T value{};
        if constexpr (std::is_same_v<T, std::string>) input >> std::quoted(value);
        else input >> value;
        if (!input) throw InvalidArgument("tree text contains an invalid value");
        return value;
    }

    static std::unique_ptr<Branch> readBranch(std::istream& input, const Format& format) {
        std::optional<T> value;
        std::unique_ptr<Branch> left;
        std::unique_ptr<Branch> right;
        for (const auto& section : format.sequence) {
            expect(input, section.open);
            if (section.mark == 'K') {
                if (peek(input) != section.close) value = readValue(input);
            } else {
                std::unique_ptr<Branch> child;
                if (peek(input) != section.close) child = readBranch(input, format);
                if (section.mark == 'L') left = std::move(child);
                else right = std::move(child);
            }
            expect(input, section.close);
        }
        if (!value) throw InvalidArgument("tree text is missing the root value");
        auto branch = std::make_unique<Branch>(std::move(*value));
        branch->left = std::move(left);
        branch->right = std::move(right);
        refresh(*branch);
        if (branch->left && !(maximum(*branch->left) < branch->value))
            throw InvalidArgument("left branch violates search ordering");
        if (branch->right && !(branch->value < minimum(*branch->right)))
            throw InvalidArgument("right branch violates search ordering");
        if (std::abs(level(branch->left) - level(branch->right)) > 1)
            throw InvalidArgument("tree text describes an unbalanced tree");
        return branch;
    }

    static char peek(std::istream& input) {
        input >> std::ws;
        const int value = input.peek();
        if (value == std::char_traits<char>::eof()) throw InvalidArgument("unexpected end of tree text");
        return static_cast<char>(value);
    }

    static void expect(std::istream& input, char expected) {
        input >> std::ws;
        char actual = 0;
        if (!(input >> actual) || actual != expected) throw InvalidArgument("tree text does not match its format");
    }

    static const T& minimum(const Branch& branch) {
        return branch.left ? minimum(*branch.left) : branch.value;
    }

    static const T& maximum(const Branch& branch) {
        return branch.right ? maximum(*branch.right) : branch.value;
    }

    static bool validate(const std::unique_ptr<Branch>& branch,
                         const std::optional<T>& lower, const std::optional<T>& upper) {
        if (!branch) return true;
        if (lower && !(*lower < branch->value)) return false;
        if (upper && !(branch->value < *upper)) return false;
        const int lh = level(branch->left), rh = level(branch->right);
        return branch->level == 1 + std::max(lh, rh)
            && branch->count == 1 + nodeCount(branch->left) + nodeCount(branch->right)
            && std::abs(lh - rh) <= 1
            && validate(branch->left, lower, branch->value)
            && validate(branch->right, branch->value, upper);
    }

    static void writeDiagram(std::ostream& output, const Branch& branch,
                             const std::string& prefix, bool last, bool root = false) {
        if (!root) output << prefix << (last ? "└── " : "├── ");
        writeValue(output, branch.value);
        output << '\n';
        const std::string childPrefix = root ? std::string{} : prefix + (last ? "    " : "│   ");
        if (branch.right) writeDiagram(output, *branch.right, childPrefix, false);
        if (branch.left) writeDiagram(output, *branch.left, childPrefix, true);
    }

    static Format parseFormat(const std::string& text) {
        Format result{{{'\0', 0, 0}, {'\0', 0, 0}, {'\0', 0, 0}}};
        std::size_t found = 0;
        for (std::size_t i = 0; i < text.size();) {
            const char open = text[i++];
            if (i >= text.size()) throw InvalidArgument("incomplete tree format");
            const bool unicode = static_cast<unsigned char>(text[i]) >= 0x80;
            const std::string marker = text.substr(i, unicode ? 2 : 1);
            i += unicode ? 2 : 1;
            if (i >= text.size()) throw InvalidArgument("incomplete tree format");
            const char close = text[i++];
            char mark = marker == "К" || marker == "K" ? 'K'
                : marker == "Л" || marker == "L" ? 'L'
                : marker == "П" || marker == "R" ? 'R' : '\0';
            const bool delimitersMatch = (open == '{' && close == '}')
                || (open == '(' && close == ')') || (open == '[' && close == ']')
                || (open == '<' && close == '>');
            if ((mark != 'K' && mark != 'L' && mark != 'R') || !delimitersMatch)
                throw InvalidArgument("tree format uses К, Л, П sections");
            for (std::size_t j = 0; j < found; ++j)
                if (result.sequence[j].mark == mark) throw InvalidArgument("tree format repeats a section");
            if (found == 3) throw InvalidArgument("tree format has extra sections");
            result.sequence[found++] = {mark, open, close};
        }
        if (found != 3) throw InvalidArgument("tree format must contain K, L and R sections");
        return result;
    }

public:
    enum class Walk { KLP, KPL, LKP, LPK, PKL, PLK };

    AvlSet() = default;
    AvlSet(const AvlSet& other) : top_(duplicate(other.top_)), count_(other.count_) {}
    AvlSet(AvlSet&& other) noexcept
        : top_(std::move(other.top_)), count_(std::exchange(other.count_, 0)),
          revision_(std::move(other.revision_)) {
        if (revision_) ++revision_->value;
    }

    ~AvlSet() { if (revision_) revision_->alive = false; }

    AvlSet& operator=(AvlSet other) noexcept(noexcept(std::declval<AvlSet&>().swap(other))) {
        swap(other);
        return *this;
    }

    void swap(AvlSet& other) noexcept {
        if (revision_) ++revision_->value;
        if (other.revision_) ++other.revision_->value;
        top_.swap(other.top_);
        std::swap(count_, other.count_);
        revision_.swap(other.revision_);
    }

    bool add(const T& value) {
        ensureRevision();
        const bool inserted = addAt(top_, value);
        count_ += inserted;
        if (inserted) ++revision_->value;
        return inserted;
    }

    bool remove(const T& value) {
        const bool erased = removeAt(top_, value);
        count_ -= erased;
        if (erased) ++revision_->value;
        return erased;
    }

    bool has(const T& value) const { return locate(top_, value); }
    const T* find(const T& value) const {
        const Branch* branch = top_.get();
        while (branch) {
            if (value < branch->value) branch = branch->left.get();
            else if (branch->value < value) branch = branch->right.get();
            else return &branch->value;
        }
        return nullptr;
    }
    bool Contains(const T& value) const override { return has(value); }
    bool empty() const noexcept { return !top_; }
    std::size_t size() const noexcept { return count_; }
    std::size_t GetCount() const noexcept override { return count_; }
    int height() const noexcept { return level(top_); }
    const T& get(std::size_t index) const {
        if (index >= count_) throw IndexOutOfRange(static_cast<int>(index), static_cast<int>(count_));
        return indexed(*top_, index);
    }
    const T& Get(std::size_t index) const override { return get(index); }

    void clear() noexcept {
        if (top_) ++revision_->value;
        top_.reset();
        count_ = 0;
    }

    std::unique_ptr<IEnumerator<T>> GetEnumerator() const override;

    template<class Function>
    void each(Walk order, Function function) const {
        walk(top_, static_cast<int>(order), function);
    }

    template<class Function>
    auto map(Function transform) const -> AvlSet<std::decay_t<std::invoke_result_t<Function, const T&>>> {
        using Result = std::decay_t<std::invoke_result_t<Function, const T&>>;
        AvlSet<Result> output;
        each(Walk::LKP, [&](const T& value) { output.add(std::invoke(transform, value)); });
        return output;
    }

    template<class Predicate>
    AvlSet where(Predicate predicate) const {
        AvlSet output;
        each(Walk::LKP, [&](const T& value) { if (std::invoke(predicate, value)) output.add(value); });
        return output;
    }

    template<class Accumulator, class Function>
    Accumulator reduce(Accumulator result, Function combine) const {
        each(Walk::LKP, [&](const T& value) { result = std::invoke(combine, value, std::move(result)); });
        return result;
    }

    AvlSet subtree(const T& rootValue) const {
        AvlSet output;
        output.top_ = branchAt(top_, rootValue);
        output.count_ = nodeCount(output.top_);
        return output;
    }

    bool contains_subtree(const AvlSet& candidate) const {
        return candidate.empty() || containsMatchingRoot(top_, candidate.top_);
    }

    AvlSet merge(const AvlSet& other) const {
        AvlSet output(*this);
        other.each(Walk::LKP, [&](const T& value) { output.add(value); });
        return output;
    }

    bool valid() const {
        return count_ == nodeCount(top_) && validate(top_, {}, {});
    }

    std::string diagram() const {
        std::ostringstream output;
        if (top_) writeDiagram(output, *top_, {}, true, true);
        return output.str();
    }

    std::string to_text(const std::string& notation = "{К}(Л)[П]") const {
        const auto format = parseFormat(notation);
        std::ostringstream output;
        writeBranch(output, top_, format);
        return output.str();
    }

    static AvlSet from_text(const std::string& text, const std::string& notation = "{К}(Л)[П]") {
        const auto format = parseFormat(notation);
        std::istringstream input(text);
        AvlSet result;
        input >> std::ws;
        if (input.peek() != std::char_traits<char>::eof())
            result.top_ = readBranch(input, format);
        input >> std::ws;
        if (input.peek() != std::char_traits<char>::eof()) throw InvalidArgument("trailing data after tree");
        result.count_ = nodeCount(result.top_);
        return result;
    }

    std::size_t live_nodes() const noexcept {
#ifdef AVLSET_TRACK_NODES
        return Branch::alive;
#else
        return 0;
#endif
    }

    bool equals(const AvlSet& other) const;

    friend bool operator==(const AvlSet& left, const AvlSet& right) { return left.equals(right); }

    friend bool operator!=(const AvlSet& left, const AvlSet& right) { return !(left == right); }
    friend AvlSet operator+(const AvlSet& left, const AvlSet& right) { return left.merge(right); }
    friend std::ostream& operator<<(std::ostream& output, const AvlSet& tree) { return output << tree.to_text(); }
};

#include "tree_enumerator.hpp"
