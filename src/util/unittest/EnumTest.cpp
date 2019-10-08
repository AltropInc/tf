#define CATCH_CONFIG_MAIN

#include <util/Enum.h>
#include <util/EnumSet.h>
#include <catch2/catch.hpp>
#include <iostream>
#include <assert.h>
#include <cstring>


namespace tf {
ENUM(Digit, uint8_t, Zero, One, Two, Three, Four, Five, Six, Seven, Eight, Nine);
using Digits = EnumSet64<Digit>;
} // namespace tf

TEST_CASE( "Enum Set To Name", "[EnumSet]" ) {
    tf::Digits digits(tf::Digit::Two, tf::Digit::Five);
    std::string str = digits.toString();
	REQUIRE(str == "(Two,Five)");
}

TEST_CASE( "Enum Set Flip", "[EnumSet]" ) {
    tf::Digits evens(tf::Digit::Zero, tf::Digit::Two, tf::Digit::Four,
                       tf::Digit::Six, tf::Digit::Eight);
    tf::Digits odds(tf::Digit::One, tf::Digit::Three, tf::Digit::Five,
                       tf::Digit::Seven, tf::Digit::Nine);
	REQUIRE(evens.flip() == odds);
}

TEST_CASE( "Enum Set Count", "[EnumSet]" ) {
    tf::Digits digits(tf::Digit::Seven, tf::Digit::Nine, tf::Digit::Four);
	REQUIRE(digits.count() == 3);
}

TEST_CASE( "Enum Set Union", "[EnumSet]" ) {
    tf::Digits digit_set1(tf::Digit::Seven, tf::Digit::Nine, tf::Digit::Four);
    tf::Digits digit_set2(tf::Digit::Seven, tf::Digit::Nine, tf::Digit::Five);
	REQUIRE((digit_set1 | digit_set2).count() == 4);
    REQUIRE((digit_set1 & digit_set2).count() == 2);
}



