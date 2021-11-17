// tagged_tuple header

// Code taken from https://gist.github.com/tarqd/dd91285d50197d6345f9
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

// helper alias, turns a list of TypePairs supplied to tagged_tuple to a list of key/tag/name types
template <class T>
using name_tag_t = typename name_tag_traits<T>::tag_type;

// same as above but returns a list of value types
template <class T>
using name_tag_value_t = typename name_tag_traits<T>::value_type;

template <class T> struct Base {};

template <class... Types>
struct TypeSet : Base<Types>... {
    template <class T>
    constexpr auto operator + (Base<T>) noexcept {
        if constexpr (std::is_base_of_v<Base<T>, TypeSet>) { 
			return TypeSet{};
		}
        else { 
			return TypeSet<Types..., T>{};
		}
    }
    constexpr std::size_t size() const noexcept { return sizeof...(Types); }
};

// checks if name tags are unique
template <class... TypePairs>
constexpr bool are_name_tags_unique() noexcept {
    constexpr auto tagset = (TypeSet<>{} + ... + Base<name_tag_t<TypePairs>>{});
    return tagset.size() == sizeof...(TypePairs);
}

template <class Needle>
constexpr std::size_t index_of_impl(size_t index, size_t end) noexcept {
    return end;
};

template <class Needle, class T, class... Haystack>
constexpr std::size_t index_of_impl(size_t index, size_t end) noexcept {
    return std::is_same<Needle, T>::value
        ? index
        : index_of_impl<Needle, Haystack...>(index + 1, end);
};

// find the index of T in a type list 
// returns sizeof...(Haystack) + 1 on failure (think std::end())
template <class Needle, class... Haystack>
static constexpr std::size_t index_of() noexcept {
    return index_of_impl<Needle, Haystack...>(0, sizeof...(Haystack) + 1);
};

}; // namespace detail

// and here's our little wrapper class that enables tagged tuples
template <class... TypePairs>
class tagged_tuple : public std::tuple<detail::name_tag_value_t<TypePairs>...> {
public:
    // throws an error if name tags are not unique
    static_assert(detail::are_name_tags_unique<TypePairs...>(), "Duplicated name tags!");
	
    // not really needed for now but if we switch to private inheritance it'll come in handy
    using tag_type = std::tuple<detail::name_tag_t<TypePairs>...>;
    using value_type = std::tuple<detail::name_tag_value_t<TypePairs>...>;
    using value_type::value_type;
    using value_type::swap;
    using value_type::operator =;
    
    // copy and move constructors
    template <class... Types>
    tagged_tuple(std::tuple<Types...>& tuple) noexcept : value_type(tuple) {}
    
    template <class... Types>
    tagged_tuple(const std::tuple<Types...>& tuple) noexcept : value_type(tuple) {} 
    
    template <class... Types>
    tagged_tuple(std::tuple<Types...>&& tuple) noexcept : value_type(std::move(tuple)) {}
};

// our special get functions
template <class Name, class... TypePairs>
constexpr std::tuple_element_t<
	detail::index_of<Name, detail::name_tag_t<TypePairs>...>(), 
	typename tagged_tuple<TypePairs...>::value_type>& get(tagged_tuple<TypePairs...>& tpl) noexcept {
    return std::get<detail::index_of<Name, detail::name_tag_t<TypePairs>...>()>(
        tpl);
};

template <class Name, class... TypePairs>
constexpr const std::tuple_element_t<
	detail::index_of<Name, detail::name_tag_t<TypePairs>...>(), 
	typename tagged_tuple<TypePairs...>::value_type>& get(const tagged_tuple<TypePairs...>& tpl) noexcept {
    return std::get<detail::index_of<Name, detail::name_tag_t<TypePairs>...>()>(
        tpl);
};

template <class Name, class... TypePairs>
constexpr std::tuple_element_t<
	detail::index_of<Name, detail::name_tag_t<TypePairs>...>(), 
	typename tagged_tuple<TypePairs...>::value_type>&& get(tagged_tuple<TypePairs...>&& tpl) noexcept {
    return std::get<detail::index_of<Name, detail::name_tag_t<TypePairs>...>()>(
        std::move(tpl));
};

} // namespace ptnk

#endif
