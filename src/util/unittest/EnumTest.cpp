#define CATCH_CONFIG_MAIN

#include <util/Enum.h>
#include <util/EnumSet.h>
#include <catch2/catch.hpp>
#include <iostream>
#include <assert.h>
#include <cstring>


namespace tf {
ENUM(Color, uint8_t, Red, Green, Blue);
using ColorSet = EnumSet64<Color>;
} // namespace tf

TEST_CASE( "Enum Set To Name", "[EnumSet]" ) {
    tf::ColorSet colors(tf::Color::Green, tf::Color::Red);
    std::string str = colors.toString();
	REQUIRE(str == "(Red,Green)");
}

TEST_CASE( "Enum Set Flip", "[EnumSet]" ) {
    tf::ColorSet colors(tf::Color::Green, tf::Color::Red);
    colors.flip();
	REQUIRE(colors == tf::ColorSet(tf::Color::Blue));
}

