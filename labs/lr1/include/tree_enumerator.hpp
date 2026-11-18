#pragma once

#include "avl_set.hpp"

#include <memory>

template<class T>
class TreeEnumerator final : public IEnumerator<T> {
    using Tree = AvlSet<T>;
    using Branch = typename Tree::Branch;
    using Revision = typename Tree::Revision;

    struct Link {
        Link(const Branch* value, std::unique_ptr<Link> nextLink)
            : branch(value), next(std::move(nextLink)) {}
        const Branch* branch;
        std::unique_ptr<Link> next;
    };

    const Branch* root_;
    const Branch* current_ = nullptr;
    std::unique_ptr<Link> pending_;
    std::weak_ptr<Revision> revision_;
    std::size_t observed_;

    void checkRevision() const {
        const auto revision = revision_.lock();
        if (!revision || !revision->alive || revision->value != observed_)
            throw InvalidState("tree changed or was destroyed during enumeration");
    }

    void pushLeft(const Branch* branch) {
        while (branch) {
            pending_ = std::make_unique<Link>(branch, std::move(pending_));
            branch = branch->left.get();
        }
    }

public:
    explicit TreeEnumerator(const Tree& tree)
        : root_(tree.top_.get()), revision_(tree.revision_), observed_(tree.revision_->value) {
        pushLeft(root_);
    }

    bool MoveNext() override {
        checkRevision();
        if (!pending_) {
            current_ = nullptr;
            return false;
        }
        auto next = std::move(pending_);
        pending_ = std::move(next->next);
        current_ = next->branch;
        pushLeft(current_->right.get());
        return true;
    }

    const T& GetCurrent() const override {
        checkRevision();
        if (!current_) throw InvalidState("enumerator has no current value");
        return current_->value;
    }

    void Reset() override {
        checkRevision();
        pending_.reset();
        current_ = nullptr;
        pushLeft(root_);
    }
};

template<class T>
std::unique_ptr<IEnumerator<T>> AvlSet<T>::GetEnumerator() const {
    ensureRevision();
    return std::make_unique<TreeEnumerator<T>>(*this);
}

template<class T>
bool AvlSet<T>::equals(const AvlSet& other) const {
    if (count_ != other.count_) return false;
    auto left = GetEnumerator();
    auto right = other.GetEnumerator();
    while (left->MoveNext()) {
        if (!right->MoveNext()) return false;
        const auto& a = left->GetCurrent();
        const auto& b = right->GetCurrent();
        if (a < b || b < a) return false;
    }
    return !right->MoveNext();
}

#include "avl_set.hpp"
