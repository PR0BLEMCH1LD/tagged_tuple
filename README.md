# tagged_tuple
Tuple but with tags.


Example code:

```
#include <iostream>
#include "tagged_tuple.hpp"

int main() {
    // defines binding between types (names/tags) 
    // easier to maintain than just going with tuple<tag, type, tag, type, etc...>
    // no chance of you accidently removing a only a tag and setting the whole list off balance
    // also they are re-usable so you can always bind a certain type to a name
    using ptnk::type_pair;
    using ptnk::tagged_tuple;
    // bring in our get function for ADL
    using ptnk::get;
    // can co-exist with std::get 
    using std::get;
    // define a tagged_tuple
    using user_t = tagged_tuple<type_pair<class name, std::string>, type_pair<class age, int>>;
    // it's initialized the same way as a tuple created with the value types of the type pairs (so tuple<std::string, int> in this case)
    user_t user{ "zyper", 15 };
    std::cout << "Name: " << get<name>(user) << std::endl;
    std::cout << "Age: " << get<age>(user) << std::endl;
    ++get<age>(user);
    std::cout << "Age: " << get<age>(user) << std::endl;
    // you can still access properties via numeric indexes as if the class was defined as tuple<string, int>
    std::cout << "user[0] = " << get<0>(user) << std::endl;
    // tagged_tuple is derives from tuple<value_types<TagPairs>...> (in this example tuple<std::string, int>)
    // so it's implicitly convertible
    std::tuple<std::string, int> regular_tuple{ user };
    user_t another_user{ regular_tuple };
    // if you don't like this you just need to make tagged_tuple privately derive from std::tuple instead of publicly
    // I don't think splicing is an issue because it adds no data members. Just keeps track of the TagType -> Index mapping
    // The mapping is done statically so there's no allocations or memory overhead. Should be just as fast as using a normal tuple with proper inlining
}
```
