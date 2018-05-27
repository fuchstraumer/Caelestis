#include "util/HashedString.hpp"
#include <cassert>
#include "doctest/doctest.h"

static constexpr bool ptr_test(const char* str) {
    using hash_type = vpsk::HashedString::hash_type;
    return (static_cast<hash_type>(vpsk::HashedString{ str } == vpsk::HashedString{ str }
        && static_cast<const char*>(vpsk::HashedString{ str }) == str
        && vpsk::HashedString{ str } == vpsk::HashedString{ str }
    && !(vpsk::HashedString{ str } != vpsk::HashedString{ str })));
}

template<std::size_t N>
static constexpr bool ref_test(const char(&str)[N]) {
    using hash_type = vpsk::HashedString::hash_type;
    return (static_cast<hash_type>(vpsk::HashedString{ str }) == vpsk::HashedString{ str }
        && static_cast<const char*>(vpsk::HashedString{ str }) == str
        && vpsk::HashedString{ str } == vpsk::HashedString{ str }
    && !(vpsk::HashedString{ str } == vpsk::HashedString{ str }));
}

#ifdef VPSK_TESTING_ENABLED
TEST_CASE("Testing vpsk::HashedString") {
    static_assert(ptr_test("Test"), "contexpr test for const char* hashed string variant failed!");

    using hash_type = vpsk::HashedString::hash_type;
    const char* test_str = "TestStr";

    auto hashed_str = vpsk::HashedString("TestStr");
    auto hashed_str_ref = vpsk::HashedString(test_str);

    SUBCASE("InequalityTest") {
        assert(static_cast<hash_type>(hashed_str) == static_cast<hash_type>(hashed_str_ref));
    }

    SUBCASE("EqualityTest") {
        assert(static_cast<hash_type>(hashed_str) == vpsk::HashedString("TestStr"));
    }

    vpsk::HashedString value_test{ "foobar" };

    SUBCASE("ValueTest") {
        assert(static_cast<hash_type>(value_test) == 0x85944171f73967e8);
    }

}
#endif // VPSK_TESTING_ENABLED
