#pragma once

#include <cstddef> // size_t
#include <iterator> // input_iterator_tag
#include <string> // string, to_string
#include <tuple> // tuple_size, get, tuple_element

#include "../meta/type_traits.hpp"
#include "../value_t.hpp"

namespace nlohmann
{
namespace detail
{
template <typename IteratorType> class iteration_proxy_value
{
  public:
    using difference_type = std::ptrdiff_t;
    using value_type = iteration_proxy_value;
    using pointer = value_type * ;
    using reference = value_type & ;
    using iterator_category = std::input_iterator_tag;

  private:
        IteratorType anchor;
        std::size_t array_index = 0;
        mutable std::size_t array_index_last = 0;
        mutable std::string array_index_str = "0";
        const std::string empty_str = "";

  public:
    explicit iteration_proxy_value(IteratorType it) noexcept : anchor(it) {}

        iteration_proxy_value& operator*()
    {
        return *this;
    }

        iteration_proxy_value& operator++()
    {
        ++anchor;
        ++array_index;

        return *this;
    }

        bool operator==(const iteration_proxy_value& o) const
    {
        return anchor == o.anchor;
    }

        bool operator!=(const iteration_proxy_value& o) const
    {
        return anchor != o.anchor;
    }

        const std::string& key() const
    {
        assert(anchor.m_object != nullptr);

        switch (anchor.m_object->type())
        {
            // use integer array index as key
            case value_t::array:
            {
                if (array_index != array_index_last)
                {
                    array_index_str = std::to_string(array_index);
                    array_index_last = array_index;
                }
                return array_index_str;
            }

            // use key from the object
            case value_t::object:
                return anchor.key();

            // use an empty key for all primitive types
            default:
                return empty_str;
        }
    }

        typename IteratorType::reference value() const
    {
        return anchor.value();
    }
};

template<typename IteratorType> class iteration_proxy
{
  private:
        typename IteratorType::reference container;

  public:
        explicit iteration_proxy(typename IteratorType::reference cont) noexcept
        : container(cont) {}

        iteration_proxy_value<IteratorType> begin() noexcept
    {
        return iteration_proxy_value<IteratorType>(container.begin());
    }

        iteration_proxy_value<IteratorType> end() noexcept
    {
        return iteration_proxy_value<IteratorType>(container.end());
    }
};
// Structured Bindings Support
// For further reference see https://blog.tartanllama.xyz/structured-bindings/
// And see https://github.com/nlohmann/json/pull/1391
template <std::size_t N, typename IteratorType, enable_if_t<N == 0, int> = 0>
auto get(const nlohmann::detail::iteration_proxy_value<IteratorType>& i) -> decltype(i.key())
{
    return i.key();
}
// Structured Bindings Support
// For further reference see https://blog.tartanllama.xyz/structured-bindings/
// And see https://github.com/nlohmann/json/pull/1391
template <std::size_t N, typename IteratorType, enable_if_t<N == 1, int> = 0>
auto get(const nlohmann::detail::iteration_proxy_value<IteratorType>& i) -> decltype(i.value())
{
    return i.value();
}
}  // namespace detail
}  // namespace nlohmann

// The Addition to the STD Namespace is required to add
// Structured Bindings Support to the iteration_proxy_value class
// For further reference see https://blog.tartanllama.xyz/structured-bindings/
// And see https://github.com/nlohmann/json/pull/1391
namespace std
{
#if defined(__clang__)
    // Fix: https://github.com/nlohmann/json/issues/1401
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wmismatched-tags"
#endif
template <typename IteratorType>
class tuple_size<::nlohmann::detail::iteration_proxy_value<IteratorType>>
            : public std::integral_constant<std::size_t, 2> {};

template <std::size_t N, typename IteratorType>
class tuple_element<N, ::nlohmann::detail::iteration_proxy_value<IteratorType >>
{
  public:
    using type = decltype(
                     get<N>(std::declval <
                            ::nlohmann::detail::iteration_proxy_value<IteratorType >> ()));
};
#if defined(__clang__)
    #pragma clang diagnostic pop
#endif
} // namespace std
