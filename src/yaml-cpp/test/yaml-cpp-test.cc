//
// Created by caesar kekxv on 2021/3/7.
//

#include "gtest/gtest.h"
#include "yaml-cpp/yaml.h"

using namespace std;
using namespace YAML;

TEST(YamlCppLoadNodeTest, NumericConversion) {
    EXPECT_EQ(1.5f, Load("1.5").as<float>());
    EXPECT_EQ(1.5, Load("1.5").as<double>());
    EXPECT_THROW(Load("1.5").as<int>(), TypedBadConversion<int>);
    EXPECT_EQ(1, Load("1").as<int>());
    EXPECT_EQ(1.0f, Load("1").as<float>());
    EXPECT_NE(Load(".nan").as<float>(), Load(".nan").as<float>());
    EXPECT_EQ(std::numeric_limits<float>::infinity(), Load(".inf").as<float>());
    EXPECT_EQ(-std::numeric_limits<float>::infinity(), Load("-.inf").as<float>());
    EXPECT_EQ(21, Load("0x15").as<int>());
    EXPECT_EQ(13, Load("015").as<int>());
    EXPECT_EQ(-128, +Load("-128").as<int8_t>());
    EXPECT_EQ(127, +Load("127").as<int8_t>());
    EXPECT_THROW(Load("128").as<int8_t>(), TypedBadConversion<signed char>);
    EXPECT_EQ(255, +Load("255").as<uint8_t>());
    EXPECT_THROW(Load("256").as<uint8_t>(), TypedBadConversion<unsigned char>);
    // test as<char>/as<uint8_t> with ‘a’,"ab",'1',"127"
    EXPECT_EQ('a', Load("a").as<char>());
    EXPECT_THROW(Load("ab").as<char>(), TypedBadConversion<char>);
    EXPECT_EQ('1', Load("1").as<char>());
    EXPECT_THROW(Load("127").as<char>(), TypedBadConversion<char>);
    EXPECT_THROW(Load("a").as<uint8_t>(), TypedBadConversion<unsigned char>);
    EXPECT_THROW(Load("ab").as<uint8_t>(), TypedBadConversion<unsigned char>);
    EXPECT_EQ(1, +Load("1").as<uint8_t>());
    // Throw exception: convert a negative number to an unsigned number.
    EXPECT_THROW(Load("-128").as<unsigned>(), TypedBadConversion<unsigned int>);
    EXPECT_THROW(Load("-128").as<unsigned short>(), TypedBadConversion<unsigned short>);
    EXPECT_THROW(Load("-128").as<unsigned long>(), TypedBadConversion<unsigned long>);
    EXPECT_THROW(Load("-128").as<unsigned long long>(), TypedBadConversion<unsigned long long>);
    EXPECT_THROW(Load("-128").as<uint8_t>(), TypedBadConversion<unsigned char>);
}
TEST(YamlCppLoadNodeTest, Binary) {
    Node node = Load(
            "[!!binary \"SGVsbG8sIFdvcmxkIQ==\", !!binary "
            "\"TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIGJ1dCBieS"
            "B0aGlzIHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxzLCB3aGljaCBpcyBhIG"
            "x1c3Qgb2YgdGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZXZlcmFuY2Ugb2YgZGVsaWdodCBpbi"
            "B0aGUgY29udGludWVkIGFuZCBpbmRlZmF0aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZG"
            "dlLCBleGNlZWRzIHRoZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS"
            "4K\"]");
    EXPECT_EQ(Binary(reinterpret_cast<const unsigned char*>("Hello, World!"), 13),
              node[0].as<Binary>());
    EXPECT_EQ(Binary(reinterpret_cast<const unsigned char*>(
                             "Man is distinguished, not only by his reason, "
                             "but by this singular passion from other "
                             "animals, which is a lust of the mind, that by "
                             "a perseverance of delight in the continued and "
                             "indefatigable generation of knowledge, exceeds "
                             "the short vehemence of any carnal pleasure.\n"),
                     270),
              node[1].as<Binary>());
}

TEST(YamlCppLoadNodeTest, BinaryWithWhitespaces) {
    Node node = Load(
            "binaryText: !binary |-\n"
            "  TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIGJ1dCBieS\n"
            "  B0aGlzIHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxzLCB3aGljaCBpcyBhIG\n"
            "  x1c3Qgb2YgdGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZXZlcmFuY2Ugb2YgZGVsaWdodCBpbi\n"
            "  B0aGUgY29udGludWVkIGFuZCBpbmRlZmF0aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZG\n"
            "  dlLCBleGNlZWRzIHRoZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS\n"
            "  4K");
    EXPECT_EQ(Binary(reinterpret_cast<const unsigned char*>(
                             "Man is distinguished, not only by his reason, "
                             "but by this singular passion from other "
                             "animals, which is a lust of the mind, that by "
                             "a perseverance of delight in the continued and "
                             "indefatigable generation of knowledge, exceeds "
                             "the short vehemence of any carnal pleasure.\n"),
                     270),
              node["binaryText"].as<Binary>());
}

