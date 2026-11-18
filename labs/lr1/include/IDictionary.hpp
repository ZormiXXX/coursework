#pragma once

#include <cstddef>

template<class Key, class Value>
class IDictionary {
public:
    virtual ~IDictionary() = default;
    virtual bool AddOrUpdate(const Key& key, const Value& value) = 0;
    virtual const Value& Get(const Key& key) const = 0;
    virtual bool ContainsKey(const Key& key) const = 0;
    virtual bool Remove(const Key& key) = 0;
    virtual std::size_t GetCount() const noexcept = 0;
};
