#pragma once

#include "IDictionary.hpp"
#include "avl_set.hpp"

#include <optional>
#include <utility>

template<class Key, class Value>
class AvlDictionary final : public IDictionary<Key, Value> {
    struct Entry {
        explicit Entry(const Key& entryKey) : key(entryKey) {}
        Entry(const Key& entryKey, const Value& entryValue) : key(entryKey), value(entryValue) {}

        bool operator<(const Entry& other) const { return key < other.key; }

        Key key;
        std::optional<Value> value;
    };

    AvlSet<Entry> entries_;

public:
    bool AddOrUpdate(const Key& key, const Value& value) override {
        const Entry lookup(key);
        const bool inserted = !entries_.has(lookup);
        if (!inserted) entries_.remove(lookup);
        entries_.add(Entry(key, value));
        return inserted;
    }

    const Value& Get(const Key& key) const override {
        const Entry lookup(key);
        const Entry* entry = entries_.find(lookup);
        if (!entry || !entry->value) throw ElementNotFound();
        return *entry->value;
    }

    bool ContainsKey(const Key& key) const override {
        return entries_.has(Entry(key));
    }

    bool Remove(const Key& key) override {
        return entries_.remove(Entry(key));
    }

    std::size_t GetCount() const noexcept override { return entries_.size(); }
    bool empty() const noexcept { return entries_.empty(); }

    template<class Function>
    void each(Function function) const {
        entries_.each(AvlSet<Entry>::Walk::LKP, [&](const Entry& entry) {
            function(entry.key, *entry.value);
        });
    }
};
