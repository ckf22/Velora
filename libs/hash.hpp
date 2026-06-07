#pragma once

#include <algorithm>
#include <functional>

namespace hash{

    template<typename T, typename _return_type = std::size_t>
    _return_type hash(const T& hash_src) noexcept {
        std::byte * it = (std::byte*)&hash_src;
        _return_type result = sizeof(T);
        std::hash<std::byte> hash_obj{};
        for(u_int16_t i = 0; i < sizeof(T); ++i)
            result = result * 31 + hash_obj(it[i]);
        return result;
    }

    template<typename T, typename _return_type = std::size_t>
    _return_type hash_iterable(const T& hash_src) noexcept {
        _return_type result = sizeof(T);
        for(auto& it : hash_src)
            result = result * 31 + hash(it);
        return result;
    }

    template<typename T, typename _return_type = std::size_t>
    struct hash_struct{
        _return_type operator()(const T& hash_src) const noexcept {
            return hash(hash_src);
        }
    };
}
