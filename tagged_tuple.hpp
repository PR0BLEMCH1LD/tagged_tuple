// tagged_tuple header

// Code taken from https://gist.github.com/ilsken/dd91285d50197d6345f9
// Modified by https://github.com/legendaryzyper

#ifndef _TAGGED_TUPLE_
#define _TAGGED_TUPLE_

#include <tuple>
#include <type_traits>

namespace ptnk {
// simple class to hold two types
// works with incomplete types, important for our use case
template <class First, class Second> struct type_pair {
    using first = First;
    using second = Second;
};

namespace detail {
// gets info about a classes name tag (key -> value type mapping) from the class
template <class T>
struct name_tag_traits {
    // aliases for our specific use case
    using tag_type = typename T::first;
    using value_type = typename T::second;
};

// helper alias, turns a list of TypePairs supplied to tagged_tuple to a list of
// key/tag/name types
template <class T>
using name_tag_t = typename name_tag_traits<T>::tag_type;

// same as above but returns a list of value types
template <class T>
using name_tag_value_t = typename name_tag_traits<T>::value_type;

template <class T> struct Base {};
template <class... Ts>
struct TypeSet : Base<Ts>... {
    template <class T>
    constexpr auto operator + (Base<T>) {
        if constexpr (std::is_base_of_v<Base<T>, TypeSet>) {
            return TypeSet{};
        }
        else {
            return TypeSet<Ts..., T>{};
        }
    }
    constexpr auto size() const -> std::size_t { return sizeof...(Ts); }
};

// checks if tags are unique
template <class... TypePairs>
constexpr auto are_tags_unique() -> bool {
    constexpr auto tagset = (TypeSet<>{} + ... + Base<name_tag_t<TypePairs>>{});
    return tagset.size() == sizeof...(TypePairs);
}

template <class Needle>
constexpr auto index_of_impl(size_t index, size_t end) -> std::size_t {
    return end;
};

template <class Needle, class T, class... Haystack>
constexpr auto index_of_impl(size_t index, size_t end) -> std::size_t {
    return std::is_same<Needle, T>::value
        ? index
        : index_of_impl<Needle, Haystack...>(index + 1, end);
};

// find the index of T in a type list, returns sizeof...(Haystack) + 1 on failure (think std::end())
template <class Needle, class... Haystack>
static constexpr auto index_of() -> std::size_t {
    return index_of_impl<Needle, Haystack...>(0, sizeof...(Haystack) + 1);
};

}; // namespace detail

// and here's our little wrapper class that enables tagged tuples
template <class... TypePairs>
class tagged_tuple : public std::tuple<detail::name_tag_value_t<TypePairs>...> {
public:
    // throws an error if tags are not unique
    static_assert(detail::are_tags_unique<TypePairs...>(), "Duplicated tags!");
    // not really needed for now but if we switch to private inheritance it'll come in handy
    using tag_type = std::tuple<detail::name_tag_t<TypePairs>...>;
    using value_type = std::tuple<detail::name_tag_value_t<TypePairs>...>;
    using value_type::value_type;
    using value_type::swap;
    using value_type::operator =;

    // copy and move constructors
    template <class... Types>
    tagged_tuple(std::tuple<Types...> &tuple) : value_type(tuple) {}

    template <class... Types>
    tagged_tuple(const std::tuple<Types...> &tuple) : value_type(tuple) {}

    template <class... Types>
    tagged_tuple(std::tuple<Types...> &&tuple) : value_type(std::move(tuple)) {}
};

// our special get functions
template <class Name, class... TypePairs>
auto get(tagged_tuple<TypePairs...> &tuple) -> typename std::tuple_element<
    detail::index_of<Name, detail::name_tag_t<TypePairs>...>(),
    typename tagged_tuple<TypePairs...>::value_type>::type &{
    return std::get<detail::index_of<Name, detail::name_tag_t<TypePairs>...>()>(
        tuple);
};

template <class Name, class... TypePairs>
auto get(const tagged_tuple<TypePairs...> &tuple) -> const typename std::tuple_element<
    detail::index_of<Name, detail::name_tag_t<TypePairs>...>(),
    typename tagged_tuple<TypePairs...>::value_type>::type &{
    return std::get<detail::index_of<Name, detail::name_tag_t<TypePairs>...>()>(
        tuple);
};

template <class Name, class... TypePairs>
auto get(tagged_tuple<TypePairs...> &&tuple) -> typename std::tuple_element<
    detail::index_of<Name, detail::name_tag_t<TypePairs>...>(),
    typename tagged_tuple<TypePairs...>::value_type>::type &&{
    return std::get<detail::index_of<Name, detail::name_tag_t<TypePairs>...>()>(
        std::move(tuple));
};

} // namespace ptnk

#endif