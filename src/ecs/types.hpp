#pragma once

#include <cstdint>
#include <limits>
#include <unordered_set>
#include <random>
#include <cassert>

namespace ecs{
    using Entity = std::uint64_t;
    
    using Component = std::uint16_t;

    using SystemID = std::uint16_t;

    template<typename T>
    struct BaseIDLimitsDefault{
        static constexpr T _MIN = std::numeric_limits<T>::min();
        static constexpr T _MAX = std::numeric_limits<T>::max();
    };

    template<typename T, T _MIN = std::numeric_limits<T>::min(), T _MAX = std::numeric_limits<T>::max(), typename _HASH_T = std::hash<T>>
    class BaseID{
      public:
        struct _HashStruct{
            [[nodiscard]] inline std::size_t operator()(const ::ecs::BaseID<T,_MIN,_MAX,_HASH_T>& v) const noexcept {
                return v.hash();
            }
        };
        //using ::std::hash<::ecs::BaseID<T,_MIN,_MAX,_HASH_T>> = _HashStruct;
        BaseID(_HASH_T _hash_obj = _HASH_T{}) : hash_obj{_hash_obj} {
            assert(std::__is_integer<T>().__value && "T must be an integer type");

            assert( _MIN >= std::numeric_limits<T>::min() && _MAX <= std::numeric_limits<T>::max()
                         && "'_MIN' and '_MAX' must be in range of specified type T" );

            assert( _MIN < _MAX && "_MIN must be smaller than _MAX" );
            
            assert( typeid(this->hash_obj(T{})) == typeid(std::size_t) && "Specified _HASH_T must have operator() taking T and returning std::size_t" );

            T buffer = static_cast<T>(random_distribution(random_twister));
            while( this->used_ids.find(this->value) != used_ids.end() )
                buffer = static_cast<T>(random_distribution(random_twister));

            this->value = buffer;
            used_ids.insert(std::move(buffer));
        }

        [[nodiscard]] inline std::size_t hash() const noexcept {
            return this->hash_obj(this->value);
        }

        [[nodiscard]] inline T get() const noexcept {
            return value;
        }

        [[nodiscard]] inline T operator()() const noexcept {
            return value;
        }
      private:
        T value{};

        _HASH_T hash_obj;
        static std::unordered_set<T, _HASH_T> used_ids;

        static std::random_device random_device;
        static std::mt19937_64 random_twister;
        static std::uniform_int_distribution<std::mt19937_64::result_type> random_distribution;
    };

    template<typename T, T min, T max, typename _HASH_T>
    std::unordered_set<T, _HASH_T> BaseID<T,min,max, _HASH_T>::used_ids{};

    template<typename T, T min, T max, typename _HASH_T>
    std::random_device BaseID<T,min,max, _HASH_T>::random_device{};

    template<typename T, T min, T max, typename _HASH_T>
    std::mt19937_64 BaseID<T,min,max, _HASH_T>::random_twister{BaseID<T,min,max, _HASH_T>::random_device()};

    template<typename T, T min, T max, typename _HASH_T>
    std::uniform_int_distribution<std::mt19937_64::result_type> BaseID<T,min,max, _HASH_T>::random_distribution{
        static_cast<std::mt19937_64::result_type>(min), static_cast<std::mt19937_64::result_type>(max)
    };
}
