// Copyright (c) 2021 Nader G. Zeid
//
// This file is part of GlideJson.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GlideJson. If not, see <https://www.gnu.org/licenses/gpl.html>.

#ifndef GLIDE_JSON_TEST_HPP
#define GLIDE_JSON_TEST_HPP

#include "GlideJson.hpp"

#include <vector>
#include <string>
#include <ctime>
#include <chrono>
#include <iostream>
#include <random>
#include <thread>

class GlideJsonTest {
  private:
    static std::vector<std::string> errors;
  public:
    static std::string hashMapToString(const GlideHashMap<int> &input);
    static std::string vectorToString(const std::vector< std::pair<std::string, int> > &input);
    static int vectorGet(const std::vector< std::pair<std::string, int> > &input, const std::string &key);
    static void testGlideHashMap();
    static void escapeByte(const unsigned char &input, std::string &output);
    static void generateJson(const int &mode, std::string * const &output, std::string * const &outputWithWs);
    static std::string generateJson(const int &mode);
    static std::pair<std::string, std::string> generateJsonWithWs();
    static void injectBadUtf8(const bool &lenient, std::string &input, std::string &output);
    static std::string generateBadUtf8();
    static std::pair<std::string, std::string> generateBadUtf8Pair();
    static std::string breakContainer(const unsigned char &input);
    static size_t locateSymbol(const std::string &input, const size_t &skip, const char &target);
    static std::string generateBadContainer(const int &mode);
    static std::string generateBadContainer();
    static void testCodepoints();
    static void testBase64();
    static void testJson();
    static void testJsonUtf8();
    static void testJsonContainers();
    static void testJsonInParallel();
    static int run();
};

#endif
