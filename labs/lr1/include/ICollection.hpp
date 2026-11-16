#pragma once

#include <cstddef>

template<class T>
class ICollection {
public:
    virtual ~ICollection() = default;
    virtual std::size_t GetCount() const noexcept = 0;
    virtual bool Contains(const T& value) const = 0;
    virtual const T& Get(std::size_t index) const = 0;
};
