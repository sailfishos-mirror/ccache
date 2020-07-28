// Copyright (C) 2019-2020 Joel Rosdahl and other contributors
//
// See doc/AUTHORS.adoc for a complete list of contributors.
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 3 of the License, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc., 51
// Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

#include "../src/Config.hpp"
#include "../src/Fd.hpp"
#include "../src/Util.hpp"
#include "TestUtil.hpp"

#include "third_party/doctest.h"

using TestUtil::TestContext;

TEST_SUITE_BEGIN("Util");

TEST_CASE("Util::base_name")
{
  CHECK(Util::base_name("") == "");
  CHECK(Util::base_name(".") == ".");
  CHECK(Util::base_name("foo") == "foo");
  CHECK(Util::base_name("/") == "");
  CHECK(Util::base_name("/foo") == "foo");
  CHECK(Util::base_name("/foo/bar/f.txt") == "f.txt");
}

TEST_CASE("Util::big_endian_to_int")
{
  uint8_t bytes[8] = {0x70, 0x9e, 0x9a, 0xbc, 0xd6, 0x54, 0x4b, 0xca};

  uint8_t uint8;
  Util::big_endian_to_int(bytes, uint8);
  CHECK(uint8 == 0x70);

  int8_t int8;
  Util::big_endian_to_int(bytes, int8);
  CHECK(int8 == 0x70);

  uint16_t uint16;
  Util::big_endian_to_int(bytes, uint16);
  CHECK(uint16 == 0x709e);

  int16_t int16;
  Util::big_endian_to_int(bytes, int16);
  CHECK(int16 == 0x709e);

  uint32_t uint32;
  Util::big_endian_to_int(bytes, uint32);
  CHECK(uint32 == 0x709e9abc);

  int32_t int32;
  Util::big_endian_to_int(bytes, int32);
  CHECK(int32 == 0x709e9abc);

  uint64_t uint64;
  Util::big_endian_to_int(bytes, uint64);
  CHECK(uint64 == 0x709e9abcd6544bca);

  int64_t int64;
  Util::big_endian_to_int(bytes, int64);
  CHECK(int64 == 0x709e9abcd6544bca);
}

TEST_CASE("Util::change_extension")
{
  CHECK(Util::change_extension("", "") == "");
  CHECK(Util::change_extension("x", "") == "x");
  CHECK(Util::change_extension("", "x") == "x");
  CHECK(Util::change_extension("", ".") == ".");
  CHECK(Util::change_extension(".", "") == "");
  CHECK(Util::change_extension("...", "x") == "..x");
  CHECK(Util::change_extension("abc", "def") == "abcdef");
  CHECK(Util::change_extension("dot.", ".dot") == "dot.dot");
  CHECK(Util::change_extension("foo.ext", "e2") == "fooe2");
  CHECK(Util::change_extension("bar.txt", ".o") == "bar.o");
  CHECK(Util::change_extension("foo.bar.txt", ".o") == "foo.bar.o");
}

TEST_CASE("Util::common_dir_prefix_length")
{
  CHECK(Util::common_dir_prefix_length("", "") == 0);
  CHECK(Util::common_dir_prefix_length("/", "") == 0);
  CHECK(Util::common_dir_prefix_length("", "/") == 0);
  CHECK(Util::common_dir_prefix_length("/", "/") == 0);
  CHECK(Util::common_dir_prefix_length("/", "/b") == 0);
  CHECK(Util::common_dir_prefix_length("/a", "/") == 0);
  CHECK(Util::common_dir_prefix_length("/a", "/b") == 0);
  CHECK(Util::common_dir_prefix_length("/a", "/a") == 2);
  CHECK(Util::common_dir_prefix_length("/a", "/a/b") == 2);
  CHECK(Util::common_dir_prefix_length("/a/b", "/a") == 2);
  CHECK(Util::common_dir_prefix_length("/a/b", "/a/c") == 2);
  CHECK(Util::common_dir_prefix_length("/a/b", "/a/b") == 4);
  CHECK(Util::common_dir_prefix_length("/a/bc", "/a/b") == 2);
  CHECK(Util::common_dir_prefix_length("/a/b", "/a/bc") == 2);
}

TEST_CASE("Util::create_dir")
{
  TestContext test_context;

  CHECK(Util::create_dir("/"));

  CHECK(Util::create_dir("create/dir"));
  CHECK(Stat::stat("create/dir").is_directory());

  Util::write_file("create/dir/file", "");
  CHECK(!Util::create_dir("create/dir/file"));
}

TEST_CASE("Util::dir_name")
{
  CHECK(Util::dir_name("") == ".");
  CHECK(Util::dir_name(".") == ".");
  CHECK(Util::dir_name("foo") == ".");
  CHECK(Util::dir_name("/") == "/");
  CHECK(Util::dir_name("/foo") == "/");
  CHECK(Util::dir_name("/foo/bar/f.txt") == "/foo/bar");
}

TEST_CASE("Util::strip_ansi_csi_seqs")
{
  const char input[] =
    "Normal,"
    " \x1B[K\x1B[1mbold\x1B[m,"
    " \x1B[31mred\x1B[m,"
    " \x1B[1;32mbold green\x1B[m.\n";

  CHECK(Util::strip_ansi_csi_seqs(input) == "Normal, bold, red, bold green.\n");
}

TEST_CASE("Util::ends_with")
{
  CHECK(Util::ends_with("", ""));
  CHECK(Util::ends_with("x", ""));
  CHECK(Util::ends_with("x", "x"));
  CHECK(Util::ends_with("xy", ""));
  CHECK(Util::ends_with("xy", "y"));
  CHECK(Util::ends_with("xy", "xy"));
  CHECK(Util::ends_with("xyz", ""));
  CHECK(Util::ends_with("xyz", "z"));
  CHECK(Util::ends_with("xyz", "yz"));
  CHECK(Util::ends_with("xyz", "xyz"));

  CHECK_FALSE(Util::ends_with("", "x"));
  CHECK_FALSE(Util::ends_with("x", "y"));
  CHECK_FALSE(Util::ends_with("x", "xy"));
}

TEST_CASE("Util::expand_environment_variables")
{
  x_setenv("FOO", "bar");

  CHECK(Util::expand_environment_variables("") == "");
  CHECK(Util::expand_environment_variables("$FOO") == "bar");
  CHECK(Util::expand_environment_variables("$") == "$");
  CHECK(Util::expand_environment_variables("$FOO $FOO:$FOO") == "bar bar:bar");
  CHECK(Util::expand_environment_variables("x$FOO") == "xbar");
  CHECK(Util::expand_environment_variables("${FOO}x") == "barx");

  DOCTEST_GCC_SUPPRESS_WARNING_PUSH
  DOCTEST_GCC_SUPPRESS_WARNING("-Wunused-result")
  CHECK_THROWS_WITH(
    (void)Util::expand_environment_variables("$surelydoesntexist"),
    "environment variable \"surelydoesntexist\" not set");
  CHECK_THROWS_WITH((void)Util::expand_environment_variables("${FOO"),
                    "syntax error: missing '}' after \"FOO\"");
  DOCTEST_GCC_SUPPRESS_WARNING_POP
}

TEST_CASE("Util::fallocate")
{
  TestContext test_context;

  const char filename[] = "test-file";

  CHECK(Util::fallocate(Fd(creat(filename, S_IRUSR | S_IWUSR)).get(), 10000)
        == 0);
  CHECK(Stat::stat(filename).size() == 10000);

  CHECK(
    Util::fallocate(Fd(open(filename, O_RDWR, S_IRUSR | S_IWUSR)).get(), 5000)
    == 0);
  CHECK(Stat::stat(filename).size() == 10000);

  CHECK(
    Util::fallocate(Fd(open(filename, O_RDWR, S_IRUSR | S_IWUSR)).get(), 20000)
    == 0);
  CHECK(Stat::stat(filename).size() == 20000);
}

TEST_CASE("Util::for_each_level_1_subdir")
{
  std::vector<std::string> actual;
  Util::for_each_level_1_subdir(
    "cache_dir",
    [&](const std::string& subdir, const Util::ProgressReceiver&) {
      actual.push_back(subdir);
    },
    [](double) {});

  std::vector<std::string> expected = {
    "cache_dir/0",
    "cache_dir/1",
    "cache_dir/2",
    "cache_dir/3",
    "cache_dir/4",
    "cache_dir/5",
    "cache_dir/6",
    "cache_dir/7",
    "cache_dir/8",
    "cache_dir/9",
    "cache_dir/a",
    "cache_dir/b",
    "cache_dir/c",
    "cache_dir/d",
    "cache_dir/e",
    "cache_dir/f",
  };
  CHECK(actual == expected);
}

TEST_CASE("format_hex")
{
  uint8_t none[] = "";
  uint8_t text[4] = "foo"; // incl. NUL
  uint8_t data[4] = {0, 1, 2, 3};

  CHECK(Util::format_hex(none, 0) == "");
  CHECK(Util::format_hex(text, sizeof(text)) == "666f6f00");
  CHECK(Util::format_hex(data, sizeof(data)) == "00010203");
}

TEST_CASE("format_human_readable_size")
{
  CHECK(Util::format_human_readable_size(0) == "0.0 MB");
  CHECK(Util::format_human_readable_size(49) == "0.0 MB");
  CHECK(Util::format_human_readable_size(420 * 1000) == "0.4 MB");
  CHECK(Util::format_human_readable_size(1000 * 1000) == "1.0 MB");
  CHECK(Util::format_human_readable_size(1234 * 1000) == "1.2 MB");
  CHECK(Util::format_human_readable_size(438.5 * 1000 * 1000) == "438.5 MB");
  CHECK(Util::format_human_readable_size(1000 * 1000 * 1000) == "1.0 GB");
  CHECK(Util::format_human_readable_size(17.11 * 1000 * 1000 * 1000)
        == "17.1 GB");
}

TEST_CASE("format_parsable_size_with_suffix")
{
  CHECK(Util::format_parsable_size_with_suffix(0) == "0");
  CHECK(Util::format_parsable_size_with_suffix(42 * 1000) == "42000");
  CHECK(Util::format_parsable_size_with_suffix(1000 * 1000) == "1.0M");
  CHECK(Util::format_parsable_size_with_suffix(1234 * 1000) == "1.2M");
  CHECK(Util::format_parsable_size_with_suffix(438.5 * 1000 * 1000)
        == "438.5M");
  CHECK(Util::format_parsable_size_with_suffix(1000 * 1000 * 1000) == "1.0G");
  CHECK(Util::format_parsable_size_with_suffix(17.11 * 1000 * 1000 * 1000)
        == "17.1G");
}

TEST_CASE("Util::get_extension")
{
  CHECK(Util::get_extension("") == "");
  CHECK(Util::get_extension(".") == ".");
  CHECK(Util::get_extension("...") == ".");
  CHECK(Util::get_extension("foo") == "");
  CHECK(Util::get_extension("/") == "");
  CHECK(Util::get_extension("/foo") == "");
  CHECK(Util::get_extension("/foo/bar/f") == "");
  CHECK(Util::get_extension("f.txt") == ".txt");
  CHECK(Util::get_extension("f.abc.txt") == ".txt");
  CHECK(Util::get_extension("/foo/bar/f.txt") == ".txt");
  CHECK(Util::get_extension("/foo/bar/f.abc.txt") == ".txt");
}

TEST_CASE("Util::get_level_1_files")
{
  TestContext test_context;

  Util::create_dir("e/m/p/t/y");

  Util::create_dir("0/1");
  Util::create_dir("0/f/c");
  Util::write_file("0/file_a", "");
  Util::write_file("0/1/file_b", "1");
  Util::write_file("0/1/file_c", "12");
  Util::write_file("0/f/c/file_d", "123");

  std::vector<std::shared_ptr<CacheFile>> files;
  auto null_receiver = [](double) {};

  SUBCASE("nonexistent subdirectory")
  {
    Util::get_level_1_files("2", null_receiver, files);
    CHECK(files.empty());
  }

  SUBCASE("empty subdirectory")
  {
    Util::get_level_1_files("e", null_receiver, files);
    CHECK(files.empty());
  }

  SUBCASE("simple case")
  {
    Util::get_level_1_files("0", null_receiver, files);
    REQUIRE(files.size() == 4);

    // Files within a level are in arbitrary order, sort them to be able to
    // verify them.
    std::sort(files.begin(),
              files.end(),
              [](const std::shared_ptr<CacheFile>& f1,
                 const std::shared_ptr<CacheFile>& f2) {
                return f1->path() < f2->path();
              });

    CHECK(files[0]->path() == "0/1/file_b");
    CHECK(files[0]->lstat().size() == 1);
    CHECK(files[1]->path() == "0/1/file_c");
    CHECK(files[1]->lstat().size() == 2);
    CHECK(files[2]->path() == "0/f/c/file_d");
    CHECK(files[2]->lstat().size() == 3);
    CHECK(files[3]->path() == "0/file_a");
    CHECK(files[3]->lstat().size() == 0);
  }
}

TEST_CASE("Util::get_relative_path")
{
#ifdef _WIN32
  CHECK(Util::get_relative_path("C:/a", "C:/a") == ".");
  CHECK(Util::get_relative_path("C:/a", "Z:/a") == "Z:/a");
  CHECK(Util::get_relative_path("C:/a/b", "C:/a") == "..");
  CHECK(Util::get_relative_path("C:/a", "C:/a/b") == "b");
  CHECK(Util::get_relative_path("C:/a", "C:/a/b/c") == "b/c");
  CHECK(Util::get_relative_path("C:/a/b", "C:/a/c") == "../c");
  CHECK(Util::get_relative_path("C:/a/b", "C:/a/c/d") == "../c/d");
  CHECK(Util::get_relative_path("C:/a/b/c", "C:/a/c/d") == "../../c/d");
  CHECK(Util::get_relative_path("C:/a/b", "C:/") == "../..");
  CHECK(Util::get_relative_path("C:/a/b", "C:/c") == "../../c");
  CHECK(Util::get_relative_path("C:/", "C:/a/b") == "a/b");
  CHECK(Util::get_relative_path("C:/a", "D:/a/b") == "D:/a/b");
#else
  CHECK(Util::get_relative_path("/a", "/a") == ".");
  CHECK(Util::get_relative_path("/a/b", "/a") == "..");
  CHECK(Util::get_relative_path("/a", "/a/b") == "b");
  CHECK(Util::get_relative_path("/a", "/a/b/c") == "b/c");
  CHECK(Util::get_relative_path("/a/b", "/a/c") == "../c");
  CHECK(Util::get_relative_path("/a/b", "/a/c/d") == "../c/d");
  CHECK(Util::get_relative_path("/a/b/c", "/a/c/d") == "../../c/d");
  CHECK(Util::get_relative_path("/a/b", "/") == "../..");
  CHECK(Util::get_relative_path("/a/b", "/c") == "../../c");
  CHECK(Util::get_relative_path("/", "/a/b") == "a/b");
#endif
}

TEST_CASE("Util::get_path_in_cache")
{
  CHECK(Util::get_path_in_cache("/zz/ccache", 1, "ABCDEF", ".suffix")
        == "/zz/ccache/A/BCDEF.suffix");
  CHECK(Util::get_path_in_cache("/zz/ccache", 4, "ABCDEF", ".suffix")
        == "/zz/ccache/A/B/C/D/EF.suffix");
}

TEST_CASE("Util::int_to_big_endian")
{
  uint8_t bytes[8];

  uint8_t uint8 = 0x70;
  Util::int_to_big_endian(uint8, bytes);
  CHECK(bytes[0] == 0x70);

  int8_t int8 = 0x70;
  Util::int_to_big_endian(int8, bytes);
  CHECK(bytes[0] == 0x70);

  uint16_t uint16 = 0x709e;
  Util::int_to_big_endian(uint16, bytes);
  CHECK(bytes[0] == 0x70);
  CHECK(bytes[1] == 0x9e);

  int16_t int16 = 0x709e;
  Util::int_to_big_endian(int16, bytes);
  CHECK(bytes[0] == 0x70);
  CHECK(bytes[1] == 0x9e);

  uint32_t uint32 = 0x709e9abc;
  Util::int_to_big_endian(uint32, bytes);
  CHECK(bytes[0] == 0x70);
  CHECK(bytes[1] == 0x9e);
  CHECK(bytes[2] == 0x9a);
  CHECK(bytes[3] == 0xbc);

  int32_t int32 = 0x709e9abc;
  Util::int_to_big_endian(int32, bytes);
  CHECK(bytes[0] == 0x70);
  CHECK(bytes[1] == 0x9e);
  CHECK(bytes[2] == 0x9a);
  CHECK(bytes[3] == 0xbc);

  uint64_t uint64 = 0x709e9abcd6544bca;
  Util::int_to_big_endian(uint64, bytes);
  CHECK(bytes[0] == 0x70);
  CHECK(bytes[1] == 0x9e);
  CHECK(bytes[2] == 0x9a);
  CHECK(bytes[3] == 0xbc);
  CHECK(bytes[4] == 0xd6);
  CHECK(bytes[5] == 0x54);
  CHECK(bytes[6] == 0x4b);
  CHECK(bytes[7] == 0xca);

  int64_t int64 = 0x709e9abcd6544bca;
  Util::int_to_big_endian(int64, bytes);
  CHECK(bytes[0] == 0x70);
  CHECK(bytes[1] == 0x9e);
  CHECK(bytes[2] == 0x9a);
  CHECK(bytes[3] == 0xbc);
  CHECK(bytes[4] == 0xd6);
  CHECK(bytes[5] == 0x54);
  CHECK(bytes[6] == 0x4b);
  CHECK(bytes[7] == 0xca);
}

TEST_CASE("Util::is_absolute_path")
{
#ifdef _WIN32
  CHECK(Util::is_absolute_path("C:/"));
  CHECK(Util::is_absolute_path("C:\\foo/fie"));
  CHECK(Util::is_absolute_path("/C:\\foo/fie")); // MSYS/Cygwin path
  CHECK(!Util::is_absolute_path(""));
  CHECK(!Util::is_absolute_path("foo\\fie/fum"));
  CHECK(!Util::is_absolute_path("C:foo/fie"));
#endif
  CHECK(Util::is_absolute_path("/"));
  CHECK(Util::is_absolute_path("/foo/fie"));
  CHECK(!Util::is_absolute_path(""));
  CHECK(!Util::is_absolute_path("foo/fie"));
}

TEST_CASE("Util::is_dir_separator")
{
  CHECK(!Util::is_dir_separator('x'));
  CHECK(Util::is_dir_separator('/'));
#ifdef _WIN32
  CHECK(Util::is_dir_separator('\\'));
#else
  CHECK(!Util::is_dir_separator('\\'));
#endif
}

TEST_CASE("Util::matches_dir_prefix_or_file")
{
  CHECK(!Util::matches_dir_prefix_or_file("", ""));
  CHECK(!Util::matches_dir_prefix_or_file("/", ""));
  CHECK(!Util::matches_dir_prefix_or_file("", "/"));

  CHECK(Util::matches_dir_prefix_or_file("aa", "aa"));
  CHECK(!Util::matches_dir_prefix_or_file("aaa", "aa"));
  CHECK(!Util::matches_dir_prefix_or_file("aa", "aaa"));
  CHECK(!Util::matches_dir_prefix_or_file("aa/", "aa"));

  CHECK(Util::matches_dir_prefix_or_file("/aa/bb", "/aa/bb"));
  CHECK(!Util::matches_dir_prefix_or_file("/aa/b", "/aa/bb"));
  CHECK(!Util::matches_dir_prefix_or_file("/aa/bbb", "/aa/bb"));

  CHECK(Util::matches_dir_prefix_or_file("/aa", "/aa/bb"));
  CHECK(Util::matches_dir_prefix_or_file("/aa/", "/aa/bb"));
  CHECK(!Util::matches_dir_prefix_or_file("/aa/bb", "/aa"));
  CHECK(!Util::matches_dir_prefix_or_file("/aa/bb", "/aa/"));

#ifdef _WIN32
  CHECK(Util::matches_dir_prefix_or_file("\\aa", "\\aa\\bb"));
  CHECK(Util::matches_dir_prefix_or_file("\\aa\\", "\\aa\\bb"));
#else
  CHECK(!Util::matches_dir_prefix_or_file("\\aa", "\\aa\\bb"));
  CHECK(!Util::matches_dir_prefix_or_file("\\aa\\", "\\aa\\bb"));
#endif
}

TEST_CASE("Util::normalize_absolute_path")
{
  CHECK(Util::normalize_absolute_path("") == "");
  CHECK(Util::normalize_absolute_path(".") == ".");
  CHECK(Util::normalize_absolute_path("..") == "..");
  CHECK(Util::normalize_absolute_path("...") == "...");
  CHECK(Util::normalize_absolute_path("x/./") == "x/./");

#ifdef _WIN32
  CHECK(Util::normalize_absolute_path("c:/") == "c:/");
  CHECK(Util::normalize_absolute_path("c:\\") == "c:/");
  CHECK(Util::normalize_absolute_path("c:/.") == "c:/");
  CHECK(Util::normalize_absolute_path("c:\\..") == "c:/");
  CHECK(Util::normalize_absolute_path("c:\\x/..") == "c:/");
  CHECK(Util::normalize_absolute_path("c:\\x/./y\\..\\\\z") == "c:/x/z");
#else
  CHECK(Util::normalize_absolute_path("/") == "/");
  CHECK(Util::normalize_absolute_path("/.") == "/");
  CHECK(Util::normalize_absolute_path("/..") == "/");
  CHECK(Util::normalize_absolute_path("/./") == "/");
  CHECK(Util::normalize_absolute_path("//") == "/");
  CHECK(Util::normalize_absolute_path("/../x") == "/x");
  CHECK(Util::normalize_absolute_path("/x/./y/z") == "/x/y/z");
  CHECK(Util::normalize_absolute_path("/x/../y/z/") == "/y/z");
  CHECK(Util::normalize_absolute_path("/x/.../y/z") == "/x/.../y/z");
  CHECK(Util::normalize_absolute_path("/x/yyy/../zz") == "/x/zz");
  CHECK(Util::normalize_absolute_path("//x/yyy///.././zz") == "/x/zz");
#endif
}

TEST_CASE("Util::parse_duration")
{
  CHECK(Util::parse_duration("0s") == 0);
  CHECK(Util::parse_duration("2s") == 2);
  CHECK(Util::parse_duration("1d") == 3600 * 24);
  CHECK(Util::parse_duration("2d") == 2 * 3600 * 24);
  CHECK_THROWS_WITH(
    Util::parse_duration("-2"),
    "invalid suffix (supported: d (day) and s (second)): \"-2\"");
  CHECK_THROWS_WITH(
    Util::parse_duration("2x"),
    "invalid suffix (supported: d (day) and s (second)): \"2x\"");
  CHECK_THROWS_WITH(
    Util::parse_duration("2"),
    "invalid suffix (supported: d (day) and s (second)): \"2\"");
}

TEST_CASE("Util::parse_int")
{
  CHECK(Util::parse_int("0") == 0);
  CHECK(Util::parse_int("2") == 2);
  CHECK(Util::parse_int("-17") == -17);
  CHECK(Util::parse_int("42") == 42);
  CHECK(Util::parse_int("0666") == 666);
  CHECK(Util::parse_int(" 777") == 777);

  CHECK_THROWS_WITH(Util::parse_int(""), "invalid integer: \"\"");
  CHECK_THROWS_WITH(Util::parse_int("x"), "invalid integer: \"x\"");
  CHECK_THROWS_WITH(Util::parse_int("0x"), "invalid integer: \"0x\"");
  CHECK_THROWS_WITH(Util::parse_int("0x4"), "invalid integer: \"0x4\"");
  CHECK_THROWS_WITH(Util::parse_int("0 "), "invalid integer: \"0 \"");

  // check boundary values
  if (sizeof(int) == 2) {
    CHECK(Util::parse_int("-32768") == -32768);
    CHECK(Util::parse_int("32767") == 32767);
    CHECK_THROWS_WITH(Util::parse_int("-32768"), "invalid integer: \"-32768\"");
    CHECK_THROWS_WITH(Util::parse_int("32768"), "invalid integer: \"32768\"");
  }
  if (sizeof(int) == 4) {
    CHECK(Util::parse_int("-2147483648") == -2147483648);
    CHECK(Util::parse_int("2147483647") == 2147483647);
    CHECK_THROWS_WITH(Util::parse_int("-2147483649"),
                      "invalid integer: \"-2147483649\"");
    CHECK_THROWS_WITH(Util::parse_int("2147483648"),
                      "invalid integer: \"2147483648\"");
  }
}

TEST_CASE("Util::parse_size")
{
  CHECK(Util::parse_size("0") == 0);
  CHECK(Util::parse_size("42") // Default suffix: G
        == static_cast<uint64_t>(42) * 1000 * 1000 * 1000);
  CHECK(Util::parse_size("78k") == 78 * 1000);
  CHECK(Util::parse_size("78K") == 78 * 1000);
  CHECK(Util::parse_size("1.1 M") == (int64_t(1.1 * 1000 * 1000)));
  CHECK(Util::parse_size("438.55M") == (int64_t(438.55 * 1000 * 1000)));
  CHECK(Util::parse_size("1 G") == 1 * 1000 * 1000 * 1000);
  CHECK(Util::parse_size("2T")
        == static_cast<uint64_t>(2) * 1000 * 1000 * 1000 * 1000);
  CHECK(Util::parse_size("78 Ki") == 78 * 1024);
  CHECK(Util::parse_size("1.1Mi") == (int64_t(1.1 * 1024 * 1024)));
  CHECK(Util::parse_size("438.55 Mi") == (int64_t(438.55 * 1024 * 1024)));
  CHECK(Util::parse_size("1Gi") == 1 * 1024 * 1024 * 1024);
  CHECK(Util::parse_size("2 Ti")
        == static_cast<uint64_t>(2) * 1024 * 1024 * 1024 * 1024);

  CHECK_THROWS_WITH(Util::parse_size(""), "invalid size: \"\"");
  CHECK_THROWS_WITH(Util::parse_size("x"), "invalid size: \"x\"");
  CHECK_THROWS_WITH(Util::parse_size("10x"), "invalid size: \"10x\"");
}

TEST_CASE("Util::parse_uint32")
{
  CHECK(Util::parse_uint32("0") == 0);
  CHECK(Util::parse_uint32("2") == 2);
  CHECK(Util::parse_uint32("42") == 42);
  CHECK(Util::parse_uint32("0666") == 666);
  CHECK(Util::parse_uint32(" 777") == 777);

  CHECK_THROWS_WITH(Util::parse_uint32(""),
                    "invalid 32-bit unsigned integer: \"\"");
  CHECK_THROWS_WITH(Util::parse_uint32("x"),
                    "invalid 32-bit unsigned integer: \"x\"");
  CHECK_THROWS_WITH(Util::parse_uint32("0x"),
                    "invalid 32-bit unsigned integer: \"0x\"");
  CHECK_THROWS_WITH(Util::parse_uint32("0x4"),
                    "invalid 32-bit unsigned integer: \"0x4\"");
  CHECK_THROWS_WITH(Util::parse_uint32("0 "),
                    "invalid 32-bit unsigned integer: \"0 \"");

  // check boundary values
  CHECK(Util::parse_uint32("4294967295") == 4294967295);
  CHECK_THROWS_WITH(Util::parse_uint32("-1"),
                    "invalid 32-bit unsigned integer: \"-1\"");
  CHECK_THROWS_WITH(Util::parse_uint32("4294967296"),
                    "invalid 32-bit unsigned integer: \"4294967296\"");
}

TEST_CASE("Util::read_file and Util::write_file")
{
  TestContext test_context;

  Util::write_file("test", "foo\nbar\n");
  std::string data = Util::read_file("test");
  CHECK(data == "foo\nbar\n");

  Util::write_file("test", "car");
  data = Util::read_file("test");
  CHECK(data == "car");

  Util::write_file("test", "pet", std::ios::app);
  data = Util::read_file("test");
  CHECK(data == "carpet");

  Util::write_file("test", "\n", std::ios::app | std::ios::binary);
  data = Util::read_file("test");
  CHECK(data == "carpet\n");

  Util::write_file("test", "\n", std::ios::app); // text mode
  data = Util::read_file("test");
#ifdef _WIN32
  CHECK(data == "carpet\n\r\n");
#else
  CHECK(data == "carpet\n\n");
#endif
  CHECK_THROWS_WITH(Util::read_file("does/not/exist"),
                    "No such file or directory");

  CHECK_THROWS_WITH(Util::write_file("", "does/not/exist"),
                    "No such file or directory");
}

TEST_CASE("Util::remove_extension")
{
  CHECK(Util::remove_extension("") == "");
  CHECK(Util::remove_extension(".") == "");
  CHECK(Util::remove_extension("...") == "..");
  CHECK(Util::remove_extension("foo") == "foo");
  CHECK(Util::remove_extension("/") == "/");
  CHECK(Util::remove_extension("/foo") == "/foo");
  CHECK(Util::remove_extension("/foo/bar/f") == "/foo/bar/f");
  CHECK(Util::remove_extension("f.txt") == "f");
  CHECK(Util::remove_extension("f.abc.txt") == "f.abc");
  CHECK(Util::remove_extension("/foo/bar/f.txt") == "/foo/bar/f");
  CHECK(Util::remove_extension("/foo/bar/f.abc.txt") == "/foo/bar/f.abc");
}

TEST_CASE("Util::same_program_name")
{
  CHECK(Util::same_program_name("foo", "foo"));
#ifdef _WIN32
  CHECK(Util::same_program_name("FOO", "foo"));
  CHECK(Util::same_program_name("FOO.exe", "foo"));
#else
  CHECK(!Util::same_program_name("FOO", "foo"));
  CHECK(!Util::same_program_name("FOO.exe", "foo"));
#endif
}

TEST_CASE("Util::split_into_views")
{
  {
    CHECK(Util::split_into_views("", "/").empty());
  }
  {
    CHECK(Util::split_into_views("///", "/").empty());
  }
  {
    auto s = Util::split_into_views("a/b", "/");
    REQUIRE(s.size() == 2);
    CHECK(s.at(0) == "a");
    CHECK(s.at(1) == "b");
  }
  {
    auto s = Util::split_into_views("a/b", "x");
    REQUIRE(s.size() == 1);
    CHECK(s.at(0) == "a/b");
  }
  {
    auto s = Util::split_into_views("a/b:c", "/:");
    REQUIRE(s.size() == 3);
    CHECK(s.at(0) == "a");
    CHECK(s.at(1) == "b");
    CHECK(s.at(2) == "c");
  }
  {
    auto s = Util::split_into_views(":a//b..:.c/:/.", "/:.");
    REQUIRE(s.size() == 3);
    CHECK(s.at(0) == "a");
    CHECK(s.at(1) == "b");
    CHECK(s.at(2) == "c");
  }
  {
    auto s = Util::split_into_views(".0.1.2.3.4.5.6.7.8.9.", "/:.+_abcdef");
    REQUIRE(s.size() == 10);
    CHECK(s.at(0) == "0");
    CHECK(s.at(9) == "9");
  }
}

TEST_CASE("Util::split_into_strings")
{
  {
    CHECK(Util::split_into_strings("", "/").empty());
  }
  {
    CHECK(Util::split_into_strings("///", "/").empty());
  }
  {
    auto s = Util::split_into_strings("a/b", "/");
    REQUIRE(s.size() == 2);
    CHECK(s.at(0) == "a");
    CHECK(s.at(1) == "b");
  }
  {
    auto s = Util::split_into_strings("a/b", "x");
    REQUIRE(s.size() == 1);
    CHECK(s.at(0) == "a/b");
  }
  {
    auto s = Util::split_into_strings("a/b:c", "/:");
    REQUIRE(s.size() == 3);
    CHECK(s.at(0) == "a");
    CHECK(s.at(1) == "b");
    CHECK(s.at(2) == "c");
  }
  {
    auto s = Util::split_into_strings(":a//b..:.c/:/.", "/:.");
    REQUIRE(s.size() == 3);
    CHECK(s.at(0) == "a");
    CHECK(s.at(1) == "b");
    CHECK(s.at(2) == "c");
  }
  {
    auto s = Util::split_into_strings(".0.1.2.3.4.5.6.7.8.9.", "/:.+_abcdef");
    REQUIRE(s.size() == 10);
    CHECK(s.at(0) == "0");
    CHECK(s.at(9) == "9");
  }
}

TEST_CASE("Util::starts_with")
{
  CHECK(Util::starts_with("", ""));
  CHECK(Util::starts_with("x", ""));
  CHECK(Util::starts_with("x", "x"));
  CHECK(Util::starts_with("xy", ""));
  CHECK(Util::starts_with("xy", "x"));
  CHECK(Util::starts_with("xy", "xy"));
  CHECK(Util::starts_with("xyz", ""));
  CHECK(Util::starts_with("xyz", "x"));
  CHECK(Util::starts_with("xyz", "xy"));
  CHECK(Util::starts_with("xyz", "xyz"));

  CHECK_FALSE(Util::starts_with("", "x"));
  CHECK_FALSE(Util::starts_with("x", "y"));
  CHECK_FALSE(Util::starts_with("x", "xy"));
}

TEST_CASE("Util::strip_whitespace")
{
  CHECK(Util::strip_whitespace("") == "");
  CHECK(Util::strip_whitespace("x") == "x");
  CHECK(Util::strip_whitespace(" x") == "x");
  CHECK(Util::strip_whitespace("x ") == "x");
  CHECK(Util::strip_whitespace(" x ") == "x");
  CHECK(Util::strip_whitespace(" \n\tx \n\t") == "x");
  CHECK(Util::strip_whitespace("  x  y  ") == "x  y");
}

TEST_CASE("Util::to_lowercase")
{
  CHECK(Util::to_lowercase("") == "");
  CHECK(Util::to_lowercase("x") == "x");
  CHECK(Util::to_lowercase("X") == "x");
  CHECK(Util::to_lowercase(" x_X@") == " x_x@");
}

TEST_CASE("Util::traverse")
{
  TestContext test_context;

  REQUIRE(Util::create_dir("dir-with-subdir-and-file/subdir"));
  Util::write_file("dir-with-subdir-and-file/subdir/f", "");
  REQUIRE(Util::create_dir("dir-with-files"));
  Util::write_file("dir-with-files/f1", "");
  Util::write_file("dir-with-files/f2", "");
  REQUIRE(Util::create_dir("empty-dir"));

  std::vector<std::string> visited;
  auto visitor = [&visited](const std::string& path, bool is_dir) {
    visited.push_back(fmt::format("[{}] {}", is_dir ? 'd' : 'f', path));
  };

  SUBCASE("traverse nonexistent path")
  {
    CHECK_THROWS_WITH(
      Util::traverse("nonexistent", visitor),
      "failed to open directory nonexistent: No such file or directory");
  }

  SUBCASE("traverse file")
  {
    CHECK_NOTHROW(Util::traverse("dir-with-subdir-and-file/subdir/f", visitor));
    REQUIRE(visited.size() == 1);
    CHECK(visited[0] == "[f] dir-with-subdir-and-file/subdir/f");
  }

  SUBCASE("traverse empty directory")
  {
    CHECK_NOTHROW(Util::traverse("empty-dir", visitor));
    REQUIRE(visited.size() == 1);
    CHECK(visited[0] == "[d] empty-dir");
  }

  SUBCASE("traverse directory with files")
  {
    CHECK_NOTHROW(Util::traverse("dir-with-files", visitor));
    REQUIRE(visited.size() == 3);
    std::string f1 = "[f] dir-with-files/f1";
    std::string f2 = "[f] dir-with-files/f2";
    CHECK(((visited[0] == f1 && visited[1] == f2)
           || (visited[0] == f2 && visited[1] == f1)));
    CHECK(visited[2] == "[d] dir-with-files");
  }

  SUBCASE("traverse directory hierarchy")
  {
    CHECK_NOTHROW(Util::traverse("dir-with-subdir-and-file", visitor));
    REQUIRE(visited.size() == 3);
    CHECK(visited[0] == "[f] dir-with-subdir-and-file/subdir/f");
    CHECK(visited[1] == "[d] dir-with-subdir-and-file/subdir");
    CHECK(visited[2] == "[d] dir-with-subdir-and-file");
  }
}

TEST_CASE("Util::wipe_path")
{
  TestContext test_context;

  SUBCASE("Wipe non-existing path")
  {
    CHECK_NOTHROW(Util::wipe_path("a"));
  }

  SUBCASE("Wipe file")
  {
    Util::write_file("a", "");
    CHECK_NOTHROW(Util::wipe_path("a"));
    CHECK(!Stat::stat("a"));
  }

  SUBCASE("Wipe directory")
  {
    REQUIRE(Util::create_dir("a/b"));
    Util::write_file("a/1", "");
    Util::write_file("a/b/1", "");
    CHECK_NOTHROW(Util::wipe_path("a"));
    CHECK(!Stat::stat("a"));
  }

  SUBCASE("Wipe bad path")
  {
#ifdef _WIN32
    const char error[] = "failed to rmdir .: Permission denied";
#else
    const char error[] = "failed to rmdir .: Invalid argument";
#endif
    CHECK_THROWS_WITH(Util::wipe_path("."), error);
  }
}

TEST_SUITE_END();
