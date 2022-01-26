# GlideJson

## Introduction

We are all painfully aware of the ridiculous number of JSON parsers for C++. GlideJson was forced to exist to satisfy the following requirements in this order:

1. Pure C++11 compatibility and capabilities, with absolutely no external dependencies. STL containers are used for JSON arrays and objects. STL strings are used in favor of C strings where applicable. GlideJson objects are as fast and as easy to manipulate as their underlying STL objects.

2. Exhaustive support for UTF-8, namely [RFC 3629](https://tools.ietf.org/html/rfc3629).

3. String parsing and encoding are strictly limited to [FSMs](https://en.wikipedia.org/wiki/Finite-state_machine) composed solely of state tables and jump tables. No naive daisy-chaining of conditionals. No look-aheads, look-behinds, or recursion. No output sub-string mangling. Each byte of input is visited literally once, and each byte of output is appended permanently.

The idea is to more aggressively balance convenience and speed. You can see a rough comparison of GlideJson against popular parsers over [here](https://github.com/nzeid/glidejsonvs).

## Building/Embedding

It's recommended that you run the given makefile to get a quick readout of any individual compiler/linker commands that you'd like to embed in your own project.

The few files that make up GlideJson are the header:

    include/glidejson/GlideJson.hpp

The implementation:

    src/glidejson/GlideJson.cpp

Code fragments used by `src/glidejson/GlideJson.cpp`:

    src/glidejson/Array.inc
    src/glidejson/Encoder.inc
    src/glidejson/Object.inc
    src/glidejson/Parser.inc

And SipHash:

    include/siphash/halfsiphash.h
    include/siphash/siphash.h
    src/siphash/halfsiphash.c
    src/siphash/siphash.c

Note that SipHash is pure C. Remember that you can always just run the given makefile to create a shared library that will end up in `lib`. Whatever your use case may be, compiling should work on any platform.

## Usage

The point of entry for this entire project is the GlideJson class.

### Types
Specifically `GlideJson::Type` and `GlideJson::Whitespace`:

    enum Type { Error, Null, Boolean, Number, String, Array, Object };
    enum Whitespace { SpaceLf, TabLf, SpaceCrlf, TabCrlf };

### Constructors

    GlideJson();
    GlideJson(const GlideJson::Type &input);
    GlideJson(const GlideJson &input);
    GlideJson(GlideJson &&input);
    explicit GlideJson(const bool &input);
    explicit GlideJson(const int &input);
    explicit GlideJson(const unsigned int &input);
    explicit GlideJson(const long int &input);
    explicit GlideJson(const long unsigned int &input);
    explicit GlideJson(const size_t &count, const char &input);
    explicit GlideJson(const char * const &input);
    explicit GlideJson(const char * const &input, const size_t &size);
    explicit GlideJson(const std::string &input);
    explicit GlideJson(std::string &&input);

### Assignment

    GlideJson & operator=(const GlideJson::Type &input);
    GlideJson & operator=(const GlideJson &input);
    GlideJson & operator=(GlideJson &&input);
    GlideJson & operator=(const bool &input);
    GlideJson & operator=(const int &input);
    GlideJson & operator=(const unsigned int &input);
    GlideJson & operator=(const long int &input);
    GlideJson & operator=(const unsigned long int &input);
    bool setNumber(const std::string &input);
    bool setNumber(const char * const &input, const size_t &size);
    GlideJson & setString(const size_t &count, const char &input);
    GlideJson & operator=(const char * const &input);
    GlideJson & setString(const char * const &input, const size_t &size);
    GlideJson & operator=(const std::string &input);
    GlideJson & operator=(std::string &&input);

### Type checking

    GlideJson::Type getType() const;
    bool isError() const;
    bool isNull() const;
    bool isBoolean() const;
    bool isNumber() const;
    bool isString() const;
    bool isArray() const;
    bool isObject() const;
    bool notError() const;
    bool notNull() const;
    bool notBoolean() const;
    bool notNumber() const;
    bool notString() const;
    bool notArray() const;
    bool notObject() const;

### Access

    const std::string & error() const;
    const bool & boolean() const;
    const std::string & number() const;
    const std::string & string() const;
    const std::vector<GlideJson> & array() const;
    const GlideHashMap<GlideJson> & object() const;
    int toInt() const;
    unsigned int toUInt() const;
    long int toLong() const;
    unsigned long int toULong() const;

### Return-by-reference access and modification

    bool & boolean();
    std::string & string();
    std::vector<GlideJson> & array();
    GlideHashMap<GlideJson> & object();

### Parsing

    static GlideJson parse(const std::string &input);
    static GlideJson parse(const char * const &input, const size_t &size);

### JSON encoding

    std::string toJson() const;
    std::string toJson(const GlideJson::Whitespace &type) const;

There is some very basic support for pretty-printing through `GlideJson::Whitespace`:

|||
| :--- | :--- |
|`GlideJson::SpaceLf`|2 spaces and a newline|
|`GlideJson::TabLf`|a tab and a newline|
|`GlideJson::SpaceCrlf`|2 spaces, a carriage return, and a newline|
|`GlideJson::TabCrlf`|a tab, a carriage return, and a newline|

### JSON-encoding strings

    static std::string encodeString(const std::string &input);
    static std::string encodeString(const char * const &input, const size_t &size);

### Base64 and more

    static unsigned char getHex(const unsigned char &input);
    static std::string base64Encode(const std::string &input);
    static std::string base64Encode(const char * const &input, const size_t &size);
    static std::string base64Decode(const std::string &input);
    static std::string base64Decode(const char * const &input, const size_t &size);

### GlideHashMap&lt;class T&gt;
This class, which is used above as `GlideHashMap<GlideJson>`, behaves almost exactly like `std::unordered_map`, with the addition that insertion order is preserved and sorting functions `sort` and `rsort` are provided. If it wasn't already obvious, this is the structure used to represent JSON objects.

    GlideHashMap();
    GlideHashMap(const GlideHashMap &input);
    GlideHashMap(GlideHashMap &&input);
    ~GlideHashMap();
    GlideHashMap & operator=(const GlideHashMap &input);
    GlideHashMap & operator=(GlideHashMap &&input);
    size_t size() const;
    size_t count(const std::string &key) const;
    bool empty() const;
    const T & at(const std::string &key) const;
    T & at(const std::string &key);
    T & operator[](const std::string &key);
    T & operator[](std::string &&key);
    size_t erase(const std::string &key);
    void clear();
    void sort();
    void rsort();
    auto begin() const;
    auto end() const;
    auto begin();
    auto end();
    auto rbegin() const;
    auto rend() const;
    auto rbegin();
    auto rend();

## Example

More thorough examples can be found by taking a look at `GlideJsonTest.cpp`. But for good measure here's a quick one:

```cpp
// Parse:
std::string test("{\"first\":[null, 123],\"second\":[true, \"third\"]}");
GlideJson testGJ(GlideJson::parse(test));

// Encode and print:
std::cout << "\n" << testGJ.toJson(GlideJson::SpaceLf);

// Modify:
auto &someObject = testGJ.object();
auto &someArray = someObject["second"].array();
someArray[0] = 456;
testGJ.object()["first"].array().clear();

// Print the final result:
std::cout << "\n" << testGJ.toJson(GlideJson::SpaceLf) << "\n";
```

Which outputs:

```
{
  "first": [
    null,
    123
  ],
  "second": [
    true,
    "third"
  ]
}
{
  "first": [],
  "second": [
    456,
    "third"
  ]
}
```

## Contributing

Please communicate changes over this project's GitLab/GitHub pages. Bear in mind that the GitLab/GitHub pages are merely mirrors of a separate "official" repository. Any accepted changes will get due authorship and credit.

- [https://github.com/nzeid/glidejson](https://github.com/nzeid/glidejson)
- [https://gitlab.com/nzeid/glidejson](https://gitlab.com/nzeid/glidejson)

## License

Copyright (c) 2021 Nader G. Zeid

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see [https://www.gnu.org/licenses/gpl.html](https://www.gnu.org/licenses/gpl.html).
