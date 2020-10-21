// Copyright (c) 2020 Nader G. Zeid
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

#ifndef GLIDE_JSON_HPP
#define GLIDE_JSON_HPP

#include <stdexcept>
#include <atomic>
#include <map>
#include <string>
#include <vector>

#define GLIDE_BYTE_SIZE 256
#define GLIDE_BYTE_WIDTH 8
#define GLIDE_BYTE_HALF_WIDTH 4
#define GLIDE_JSON_ENCODER_STATES 23
#define GLIDE_JSON_PARSER_STATES 69

class GlideError : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

// ========================================

template<class T1>
class GlideItem {
  protected:
    const T1 *item;
    std::atomic<size_t> *heldBy;
  public:
    GlideItem();
    GlideItem(const T1 * const &input);
    GlideItem(const T1 &input);
    GlideItem(T1 &&input);
    GlideItem(const GlideItem &input);
    GlideItem(GlideItem &&input);
    ~GlideItem();
    GlideItem & operator=(const GlideItem &input);
    GlideItem & operator=(GlideItem &&input);
    const T1 & value() const;
    T1 & value();
};

template<class T1>
GlideItem<T1>::GlideItem() : item(new T1()), heldBy(new std::atomic<size_t>(1)) {
}

template<class T1>
GlideItem<T1>::GlideItem(const T1 * const &input) : item(input), heldBy(NULL) {
}

template<class T1>
GlideItem<T1>::GlideItem(const T1 &input) : item(new T1(input)), heldBy(new std::atomic<size_t>(1)) {
}

template<class T1>
GlideItem<T1>::GlideItem(T1 &&input) : item(new T1(std::move(input))), heldBy(new std::atomic<size_t>(1)) {
}

template<class T1>
GlideItem<T1>::GlideItem(const GlideItem<T1> &input) : item(input.item), heldBy(input.heldBy) {
  heldBy && ++(*heldBy);
}

template<class T1>
GlideItem<T1>::GlideItem(GlideItem<T1> &&input) : item(input.item), heldBy(input.heldBy) {
  input.item = NULL;
  input.heldBy = NULL;
}

template<class T1>
GlideItem<T1>::~GlideItem() {
  if(heldBy && !(--(*heldBy))) {
    delete item;
    delete heldBy;
  }
}

template<class T1>
GlideItem<T1> & GlideItem<T1>::operator=(const GlideItem<T1> &input) {
  if(heldBy && !(--(*heldBy))) {
    delete item;
    delete heldBy;
  }
  item = input.item;
  heldBy = input.heldBy;
  heldBy && ++(*heldBy);
  return *this;
}

template<class T1>
GlideItem<T1> & GlideItem<T1>::operator=(GlideItem<T1> &&input) {
  if(heldBy && !(--(*heldBy))) {
    delete item;
    delete heldBy;
  }
  item = input.item;
  heldBy = input.heldBy;
  input.item = NULL;
  input.heldBy = NULL;
  return *this;
}

template<class T1>
T1 & GlideItem<T1>::value() {
  return *(const_cast<T1 *>(item));
}

template<class T1>
const T1 & GlideItem<T1>::value() const {
  return *item;
}

// ========================================

template<class T1>
class GlideSortItem : public GlideItem<T1> {
  using GlideItem<T1>::GlideItem;
  public:
    bool operator<(const GlideSortItem &input) const;
};

template<class T1>
bool GlideSortItem<T1>::operator<(const GlideSortItem<T1> &input) const {
  return *(GlideItem<T1>::item) < *(input.item);
}

// ========================================

template<class T1>
class GlideMapIterator {
  private:
    T1 iterator;
  public:
    GlideMapIterator();
    GlideMapIterator(const T1 &input);
    GlideMapIterator(const GlideMapIterator &input);
    ~GlideMapIterator();
    GlideMapIterator & operator=(const GlideMapIterator &input);
    bool operator==(const GlideMapIterator &input) const;
    bool operator!=(const GlideMapIterator &input) const;
    void next();
    void previous();
    const size_t & index() const;
    auto key() const -> decltype(iterator->second.first.value());
    auto value() const -> decltype(iterator->second.second.value());
};

template<class T1>
GlideMapIterator<T1>::GlideMapIterator() : iterator() {
  throw GlideError("GlideMapIterator<T1>::GlideMapIterator(): No default constructor!");
}

template<class T1>
GlideMapIterator<T1>::GlideMapIterator(const T1 &input) : iterator(input) {
}

template<class T1>
GlideMapIterator<T1>::GlideMapIterator(const GlideMapIterator<T1> &input) : iterator(input.iterator) {
}

template<class T1>
GlideMapIterator<T1>::~GlideMapIterator() {
}

template<class T1>
GlideMapIterator<T1> & GlideMapIterator<T1>::operator=(const GlideMapIterator<T1> &input) {
  iterator = input.iterator;
  return *this;
}

template<class T1>
bool GlideMapIterator<T1>::operator==(const GlideMapIterator<T1> &input) const {
  return iterator == input.iterator;
}

template<class T1>
bool GlideMapIterator<T1>::operator!=(const GlideMapIterator<T1> &input) const {
  return iterator != input.iterator;
}

template<class T1>
void GlideMapIterator<T1>::next() {
  ++iterator;
}

template<class T1>
void GlideMapIterator<T1>::previous() {
  --iterator;
}

template<class T1>
const size_t & GlideMapIterator<T1>::index() const {
  return iterator->first;
}

template<class T1>
auto GlideMapIterator<T1>::key() const -> decltype(iterator->second.first.value()) {
  return iterator->second.first.value();
}

template<class T1>
auto GlideMapIterator<T1>::value() const -> decltype(iterator->second.second.value()) {
  return iterator->second.second.value();
}

// ========================================

template<class T1, class T2>
class GlideMap {
  public:
    typedef std::map< GlideSortItem<T1>, std::pair< size_t, GlideItem<T2> > > KeyMap;
    typedef std::map< size_t, std::pair< GlideSortItem<T1>, GlideItem<T2> > > PositionMap;
  private:
    KeyMap keyMap;
    PositionMap positionMap;
    size_t counter;
  public:
    GlideMap();
    GlideMap(const GlideMap &input);
    GlideMap(GlideMap &&input);
    ~GlideMap();
    GlideMap & operator=(const GlideMap &input);
    GlideMap & operator=(GlideMap &&input);
    size_t size() const;
    size_t count(const T1 &key) const;
    bool empty() const;
    const T2 & at(const T1 &key) const;
    T2 & at(const T1 &key);
    T2 & operator[](const T1 &key);
    T2 & operator[](T1 &&key);
    size_t erase(const T1 &key);
    void clear();
    void sort();
    void rsort();
    auto begin() const -> GlideMapIterator<decltype(positionMap.begin())>;
    auto end() const -> GlideMapIterator<decltype(positionMap.end())>;
    auto begin() -> GlideMapIterator<decltype(positionMap.begin())>;
    auto end() -> GlideMapIterator<decltype(positionMap.end())>;
    auto rbegin() const -> GlideMapIterator<decltype(positionMap.rbegin())>;
    auto rend() const -> GlideMapIterator<decltype(positionMap.rend())>;
    auto rbegin() -> GlideMapIterator<decltype(positionMap.rbegin())>;
    auto rend() -> GlideMapIterator<decltype(positionMap.rend())>;
    auto find(const T1 &key) const -> GlideMapIterator<decltype(positionMap.find(0))>;
    auto find(const T1 &key) -> GlideMapIterator<decltype(positionMap.find(0))>;
};

template<class T1, class T2>
GlideMap<T1, T2>::GlideMap() : keyMap(), positionMap(), counter(0) {
}

template<class T1, class T2>
GlideMap<T1, T2>::GlideMap(const GlideMap<T1, T2> &input) : keyMap(input.keyMap), positionMap(input.positionMap), counter(input.counter) {
}

template<class T1, class T2>
GlideMap<T1, T2>::GlideMap(GlideMap<T1, T2> &&input) : keyMap(std::move(input.keyMap)), positionMap(std::move(input.positionMap)), counter(input.counter) {
}

template<class T1, class T2>
GlideMap<T1, T2>::~GlideMap() {
}

template<class T1, class T2>
GlideMap<T1, T2> & GlideMap<T1, T2>::operator=(const GlideMap<T1, T2> &input) {
  keyMap = input.keyMap;
  positionMap = input.positionMap;
  counter = input.counter;
  return *this;
}

template<class T1, class T2>
GlideMap<T1, T2> & GlideMap<T1, T2>::operator=(GlideMap<T1, T2> &&input) {
  keyMap = std::move(input.keyMap);
  positionMap = std::move(input.positionMap);
  counter = input.counter;
  return *this;
}

template<class T1, class T2>
size_t GlideMap<T1, T2>::size() const {
  return keyMap.size();
}

template<class T1, class T2>
size_t GlideMap<T1, T2>::count(const T1 &key) const {
  const GlideSortItem<T1> itemKey(&key);
  return keyMap.count(itemKey);
}

template<class T1, class T2>
bool GlideMap<T1, T2>::empty() const {
  return keyMap.empty();
}

template<class T1, class T2>
const T2 & GlideMap<T1, T2>::at(const T1 &key) const {
  const GlideSortItem<T1> itemKey(&key);
  return keyMap.at(itemKey).second.value();
}

template<class T1, class T2>
T2 & GlideMap<T1, T2>::at(const T1 &key) {
  const GlideSortItem<T1> itemKey(&key);
  return keyMap.at(itemKey).second.value();
}

template<class T1, class T2>
T2 & GlideMap<T1, T2>::operator[](const T1 &key) {
  GlideSortItem<T1> newItemKey(key);
  std::pair< size_t, GlideItem<T2> > *pair(&(keyMap[newItemKey]));
  if(pair->first == 0) {
    pair->first = ++counter;
    std::pair< GlideSortItem<T1>, GlideItem<T2> > insertPair(newItemKey, pair->second);
    positionMap[pair->first] = std::move(insertPair);
  }
  else {
    positionMap[pair->first].second = pair->second;
  }
  return pair->second.value();
}

template<class T1, class T2>
T2 & GlideMap<T1, T2>::operator[](T1 &&key) {
  GlideSortItem<T1> newItemKey(std::move(key));
  std::pair< size_t, GlideItem<T2> > *pair(&(keyMap[newItemKey]));
  if(pair->first == 0) {
    pair->first = ++counter;
    std::pair< GlideSortItem<T1>, GlideItem<T2> > insertPair(newItemKey, pair->second);
    positionMap[pair->first] = std::move(insertPair);
  }
  else {
    positionMap[pair->first].second = pair->second;
  }
  return pair->second.value();
}

template<class T1, class T2>
size_t GlideMap<T1, T2>::erase(const T1 &key) {
  const GlideSortItem<T1> itemKey(&key);
  if(keyMap.count(itemKey)) {
    positionMap.erase(keyMap[itemKey].first);
    return keyMap.erase(itemKey);
  }
  return 0;
}

template<class T1, class T2>
void GlideMap<T1, T2>::clear() {
  keyMap.clear();
  positionMap.clear();
  counter = 0;
}

template<class T1, class T2>
void GlideMap<T1, T2>::sort() {
  positionMap.clear();
  counter = 0;
  auto i(keyMap.begin());
  auto iEnd(keyMap.end());
  while(i != iEnd) {
    positionMap[++counter] = std::pair< GlideSortItem<T1>, GlideItem<T2> >(i->first, i->second.second);
    i->second.first = counter;
    ++i;
  }
}

template<class T1, class T2>
void GlideMap<T1, T2>::rsort() {
  positionMap.clear();
  counter = 0;
  auto i(keyMap.rbegin());
  auto iEnd(keyMap.rend());
  while(i != iEnd) {
    positionMap[++counter] = std::pair< GlideSortItem<T1>, GlideItem<T2> >(i->first, i->second.second);
    i->second.first = counter;
    ++i;
  }
}

template<class T1, class T2>
auto GlideMap<T1, T2>::begin() const -> GlideMapIterator<decltype(positionMap.begin())> {
  return GlideMapIterator<decltype(positionMap.begin())>(positionMap.begin());
}

template<class T1, class T2>
auto GlideMap<T1, T2>::end() const -> GlideMapIterator<decltype(positionMap.end())> {
  return GlideMapIterator<decltype(positionMap.end())>(positionMap.end());
}

template<class T1, class T2>
auto GlideMap<T1, T2>::begin() -> GlideMapIterator<decltype(positionMap.begin())> {
  return GlideMapIterator<decltype(positionMap.begin())>(positionMap.begin());
}

template<class T1, class T2>
auto GlideMap<T1, T2>::end() -> GlideMapIterator<decltype(positionMap.end())> {
  return GlideMapIterator<decltype(positionMap.end())>(positionMap.end());
}

template<class T1, class T2>
auto GlideMap<T1, T2>::rbegin() const -> GlideMapIterator<decltype(positionMap.rbegin())> {
  return GlideMapIterator<decltype(positionMap.rbegin())>(positionMap.rbegin());
}

template<class T1, class T2>
auto GlideMap<T1, T2>::rend() const -> GlideMapIterator<decltype(positionMap.rend())> {
  return GlideMapIterator<decltype(positionMap.rend())>(positionMap.rend());
}

template<class T1, class T2>
auto GlideMap<T1, T2>::rbegin() -> GlideMapIterator<decltype(positionMap.rbegin())> {
  return GlideMapIterator<decltype(positionMap.rbegin())>(positionMap.rbegin());
}

template<class T1, class T2>
auto GlideMap<T1, T2>::rend() -> GlideMapIterator<decltype(positionMap.rend())> {
  return GlideMapIterator<decltype(positionMap.rend())>(positionMap.rend());
}

template<class T1, class T2>
auto GlideMap<T1, T2>::find(const T1 &key) const -> GlideMapIterator<decltype(positionMap.find(0))> {
  const GlideSortItem<T1> itemKey(&key);
  return GlideMapIterator<decltype(positionMap.find(0))>(positionMap.find(keyMap.at(itemKey).first));
}

template<class T1, class T2>
auto GlideMap<T1, T2>::find(const T1 &key) -> GlideMapIterator<decltype(positionMap.find(0))> {
  const GlideSortItem<T1> itemKey(&key);
  return GlideMapIterator<decltype(positionMap.find(0))>(positionMap.find(keyMap.at(itemKey).first));
}

// ========================================

class GlideCheck {
  private:
    GlideCheck();
    GlideCheck(const GlideCheck &input);
  public:
    ~GlideCheck();
    GlideCheck & operator=(const GlideCheck &input);
    static const GlideCheck check;
};

// ========================================

class GlideString {
  public:
    static constexpr size_t wordBisector = sizeof(size_t) * GLIDE_BYTE_HALF_WIDTH;
    static constexpr size_t initialCapacity = 1 << 5;
  private:
    static inline void nearestPower(const size_t &inputSize, size_t &inputCapacity);
  public:
    static inline void initialize(const size_t &inputSize, size_t &inputCapacity, std::string &input);
    static inline void append(const unsigned char &inputChar, size_t &inputSize, size_t &inputCapacity, std::string &input);
};

// ========================================

namespace GlideJsonScheme {
  class Encoder;
  class Base;
  class Error;
  class Null;
  class Boolean;
  class Number;
  class String;
  class Array;
  class Object;
  class Parser;
}

class GlideJson {
  friend class GlideJsonScheme::Number;
  friend class GlideJsonScheme::Array;
  friend class GlideJsonScheme::Object;
  friend class GlideJsonScheme::Parser;
  public:
    enum Type { Error, Null, Boolean, Number, String, Array, Object };
    enum Whitespace { SpaceLf, TabLf, SpaceCrlf, TabCrlf };
  private:
    GlideJsonScheme::Base *content;
    inline void initialize(const GlideJson::Type &input);
    inline void reinitialize(const GlideJson::Type &input);
  public:
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
    ~GlideJson();
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
  private:
    std::string toJson(const GlideJson::Whitespace &type, const size_t &depth) const;
  public:
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
    std::string toJson() const;
    std::string toJson(const GlideJson::Whitespace &type) const;
    const std::string & error() const;
    const bool & boolean() const;
    const std::string & number() const;
    const std::string & string() const;
    const std::vector<GlideJson> & array() const;
    const GlideMap<std::string, GlideJson> & object() const;
    int toInt() const;
    unsigned int toUInt() const;
    long int toLong() const;
    unsigned long int toULong() const;
    bool & boolean();
    std::string & string();
    std::vector<GlideJson> & array();
    GlideMap<std::string, GlideJson> & object();
    static unsigned char getHex(const unsigned char &input);
    static GlideJson parse(const std::string &input);
    static GlideJson parse(const char * const &input, const size_t &size);
    static std::string encodeString(const std::string &input);
    static std::string encodeString(const char * const &input, const size_t &size);
    static std::string base64Encode(const std::string &input);
    static std::string base64Encode(const char * const &input, const size_t &size);
    static std::string base64Decode(const std::string &input);
    static std::string base64Decode(const char * const &input, const size_t &size);
};

namespace GlideJsonScheme {

  class EncoderInitializer;

  class Encoder {
    friend class EncoderInitializer;
    friend unsigned char GlideJson::getHex(const unsigned char &input);
    private:
      static unsigned char hexMap[16];
      static unsigned char stateMap[GLIDE_BYTE_SIZE * GLIDE_JSON_ENCODER_STATES];
      static unsigned char b64eMap[64];
      static unsigned char b64dMap[GLIDE_BYTE_SIZE];
      static void setEscapable(const size_t &state);
      static void copyTransitions(const size_t &from, const size_t &to);
      static void initialize();
      Encoder();
      Encoder(const Encoder &input);
    public:
      ~Encoder();
      Encoder & operator=(const Encoder &input);
      static std::string encode(const std::string &input);
      static std::string encode(const char * const &cInput, const size_t &size);
      static std::string base64Encode(const std::string &input);
      static std::string base64Encode(const char * const &cInput, const size_t &length);
      static std::string base64Decode(const std::string &input);
      static std::string base64Decode(const char * const &cInput, const size_t &length);
  };

  class EncoderInitializer {
    private:
      EncoderInitializer();
      EncoderInitializer(const EncoderInitializer &input);
    public:
      ~EncoderInitializer();
      EncoderInitializer & operator=(const EncoderInitializer &input);
      static const EncoderInitializer & initializer();
  };

  class Base {
    protected:
      Base();
      Base(const Base &input);
    public:
      virtual ~Base();
      Base & operator=(const Base &input);
      virtual GlideJson::Type getType() const;
      virtual std::string toJson() const;
      virtual std::string toJson(const GlideJson::Whitespace &type, const size_t &depth) const;
      virtual Base * clone() const;
      virtual void selfDelete();
      virtual Base * toError();
      virtual Base * toNull();
      virtual Base * toBoolean();
      virtual Base * toNumber(const unsigned char &input);
      virtual Base * toString();
      virtual Base * toArray();
      virtual Base * toObject();
      virtual Base * quickCopy(Base * const &input);
      virtual Base * copyToError(Error * const &input);
      virtual Base * copyToNull(Null * const &input);
      virtual Base * copyToBoolean(Boolean * const &input);
      virtual Base * copyToNumber(Number * const &input);
      virtual Base * copyToString(String * const &input);
      virtual Base * copyToArray(Array * const &input);
      virtual Base * copyToObject(Object * const &input);
      virtual const std::string & theError() const;
      virtual const bool & theBoolean() const;
      virtual bool & theBoolean();
      virtual const std::string & theNumber() const;
      virtual const std::string & theString() const;
      virtual std::string & theString();
      virtual const std::vector<GlideJson> & theArray() const;
      virtual std::vector<GlideJson> & theArray();
      virtual const GlideMap<std::string, GlideJson> & theObject() const;
      virtual GlideMap<std::string, GlideJson> & theObject();
  };

  class Error : public Base {
    friend class Parser;
    protected:
      std::string error;
    public:
      Error();
      Error(const std::string &input);
      Error(const Error &input);
      Error(Error &&input);
      virtual ~Error();
      Error & operator=(const Error &input);
      Error & operator=(Error &&input);
      virtual GlideJson::Type getType() const;
      virtual std::string toJson() const;
      virtual std::string toJson(const GlideJson::Whitespace &type, const size_t &depth) const;
      virtual Error * clone() const;
      virtual void selfDelete();
      virtual Base * toError();
      virtual Base * toNull();
      virtual Base * toBoolean();
      virtual Base * toNumber(const unsigned char &input);
      virtual Base * toString();
      virtual Base * toArray();
      virtual Base * toObject();
      virtual Base * quickCopy(Base * const &input);
      virtual Base * copyToError(Error * const &input);
      virtual Base * copyToNull(Null * const &input);
      virtual Base * copyToBoolean(Boolean * const &input);
      virtual Base * copyToNumber(Number * const &input);
      virtual Base * copyToString(String * const &input);
      virtual Base * copyToArray(Array * const &input);
      virtual Base * copyToObject(Object * const &input);
      const std::string & theError() const;
  };

  class Null : public Base {
    public:
      Null();
      Null(const Null &input);
      virtual ~Null();
      Null & operator=(const Null &input);
      virtual GlideJson::Type getType() const;
      virtual std::string toJson() const;
      virtual std::string toJson(const GlideJson::Whitespace &type, const size_t &depth) const;
      virtual Null * clone() const;
      virtual void selfDelete();
      virtual Base * toError();
      virtual Base * toNull();
      virtual Base * toBoolean();
      virtual Base * toNumber(const unsigned char &input);
      virtual Base * toString();
      virtual Base * toArray();
      virtual Base * toObject();
      virtual Base * quickCopy(Base * const &input);
      virtual Base * copyToError(Error * const &input);
      virtual Base * copyToNull(Null * const &input);
      virtual Base * copyToBoolean(Boolean * const &input);
      virtual Base * copyToNumber(Number * const &input);
      virtual Base * copyToString(String * const &input);
      virtual Base * copyToArray(Array * const &input);
      virtual Base * copyToObject(Object * const &input);
      static Null * soleNull();
  };

  class Boolean : public Base {
    public:
      bool boolean;
      Boolean();
      Boolean(const bool &input);
      Boolean(const Boolean &input);
      virtual ~Boolean();
      Boolean & operator=(const Boolean &input);
      virtual GlideJson::Type getType() const;
      virtual std::string toJson() const;
      virtual std::string toJson(const GlideJson::Whitespace &type, const size_t &depth) const;
      virtual Boolean * clone() const;
      virtual void selfDelete();
      virtual Base * toError();
      virtual Base * toNull();
      virtual Base * toBoolean();
      virtual Base * toNumber(const unsigned char &input);
      virtual Base * toString();
      virtual Base * toArray();
      virtual Base * toObject();
      virtual Base * quickCopy(Base * const &input);
      virtual Base * copyToError(Error * const &input);
      virtual Base * copyToNull(Null * const &input);
      virtual Base * copyToBoolean(Boolean * const &input);
      virtual Base * copyToNumber(Number * const &input);
      virtual Base * copyToString(String * const &input);
      virtual Base * copyToArray(Array * const &input);
      virtual Base * copyToObject(Object * const &input);
      virtual const bool & theBoolean() const;
      virtual bool & theBoolean();
  };

  class Number : public Base {
    friend class Parser;
    friend class Error;
    friend class Null;
    friend class Boolean;
    friend class String;
    friend class Array;
    friend class Object;
    protected:
      std::string number;
    public:
      Number();
    protected:
      explicit Number(const unsigned char &input);
      explicit Number(const std::string &input, const unsigned char &placeholder);
    public:
      Number(const std::string &input);
      Number(const int &input);
      Number(const unsigned int &input);
      Number(const long int &input);
      Number(const unsigned long int &input);
      Number(const Number &input);
      Number(Number &&input);
      virtual ~Number();
      Number & operator=(const int &input);
      Number & operator=(const unsigned int &input);
      Number & operator=(const long int &input);
      Number & operator=(const unsigned long int &input);
      Number & operator=(const Number &input);
      Number & operator=(Number &&input);
      virtual GlideJson::Type getType() const;
      virtual std::string toJson() const;
      virtual std::string toJson(const GlideJson::Whitespace &type, const size_t &depth) const;
      virtual Number * clone() const;
      virtual void selfDelete();
      virtual Base * toError();
      virtual Base * toNull();
      virtual Base * toBoolean();
      virtual Base * toNumber(const unsigned char &input);
      virtual Base * toString();
      virtual Base * toArray();
      virtual Base * toObject();
      virtual Base * quickCopy(Base * const &input);
      virtual Base * copyToError(Error * const &input);
      virtual Base * copyToNull(Null * const &input);
      virtual Base * copyToBoolean(Boolean * const &input);
      virtual Base * copyToNumber(Number * const &input);
      virtual Base * copyToString(String * const &input);
      virtual Base * copyToArray(Array * const &input);
      virtual Base * copyToObject(Object * const &input);
      virtual const std::string & theNumber() const;
      bool set(const std::string &input);
      bool set(const char * const &input, const size_t &size);
  };

  class String : public Base {
    public:
      std::string string;
      String();
      String(const std::string &input);
      String(std::string &&input);
      String(const String &input);
      String(String &&input);
      virtual ~String();
      String & operator=(const String &input);
      String & operator=(String &&input);
      virtual GlideJson::Type getType() const;
      virtual std::string toJson() const;
      virtual std::string toJson(const GlideJson::Whitespace &type, const size_t &depth) const;
      virtual String * clone() const;
      virtual void selfDelete();
      virtual Base * toError();
      virtual Base * toNull();
      virtual Base * toBoolean();
      virtual Base * toNumber(const unsigned char &input);
      virtual Base * toString();
      virtual Base * toArray();
      virtual Base * toObject();
      virtual Base * quickCopy(Base * const &input);
      virtual Base * copyToError(Error * const &input);
      virtual Base * copyToNull(Null * const &input);
      virtual Base * copyToBoolean(Boolean * const &input);
      virtual Base * copyToNumber(Number * const &input);
      virtual Base * copyToString(String * const &input);
      virtual Base * copyToArray(Array * const &input);
      virtual Base * copyToObject(Object * const &input);
      virtual const std::string & theString() const;
      virtual std::string & theString();
  };

  class Array : public Base {
    public:
      std::vector<GlideJson> array;
      Array();
      Array(const std::vector<GlideJson> &input);
      Array(const Array &input);
      Array(Array &&input);
      virtual ~Array();
      Array & operator=(const Array &input);
      Array & operator=(Array &&input);
      virtual GlideJson::Type getType() const;
      virtual std::string toJson() const;
      virtual std::string toJson(const GlideJson::Whitespace &type, const size_t &depth) const;
      virtual Array * clone() const;
      virtual void selfDelete();
      virtual Base * toError();
      virtual Base * toNull();
      virtual Base * toBoolean();
      virtual Base * toNumber(const unsigned char &input);
      virtual Base * toString();
      virtual Base * toArray();
      virtual Base * toObject();
      virtual Base * quickCopy(Base * const &input);
      virtual Base * copyToError(Error * const &input);
      virtual Base * copyToNull(Null * const &input);
      virtual Base * copyToBoolean(Boolean * const &input);
      virtual Base * copyToNumber(Number * const &input);
      virtual Base * copyToString(String * const &input);
      virtual Base * copyToArray(Array * const &input);
      virtual Base * copyToObject(Object * const &input);
      virtual const std::vector<GlideJson> & theArray() const;
      virtual std::vector<GlideJson> & theArray();
  };

  class Object : public Base {
    public:
      GlideMap<std::string, GlideJson> object;
      Object();
      Object(const GlideMap<std::string, GlideJson> &input);
      Object(const Object &input);
      Object(Object &&input);
      virtual ~Object();
      Object & operator=(const Object &input);
      Object & operator=(Object &&input);
      virtual GlideJson::Type getType() const;
      virtual std::string toJson() const;
      virtual std::string toJson(const GlideJson::Whitespace &type, const size_t &depth) const;
      virtual Object * clone() const;
      virtual void selfDelete();
      virtual Base * toError();
      virtual Base * toNull();
      virtual Base * toBoolean();
      virtual Base * toNumber(const unsigned char &input);
      virtual Base * toString();
      virtual Base * toArray();
      virtual Base * toObject();
      virtual Base * quickCopy(Base * const &input);
      virtual Base * copyToError(Error * const &input);
      virtual Base * copyToNull(Null * const &input);
      virtual Base * copyToBoolean(Boolean * const &input);
      virtual Base * copyToNumber(Number * const &input);
      virtual Base * copyToString(String * const &input);
      virtual Base * copyToArray(Array * const &input);
      virtual Base * copyToObject(Object * const &input);
      virtual const GlideMap<std::string, GlideJson> & theObject() const;
      virtual GlideMap<std::string, GlideJson> & theObject();
  };

  class ParserInitializer;

  class Parser {
    friend class ParserInitializer;
    private:
      static unsigned char hexMap[GLIDE_BYTE_SIZE];
      static unsigned char stateMap[GLIDE_BYTE_SIZE * GLIDE_JSON_PARSER_STATES];
      static bool incompleteMap[GLIDE_JSON_PARSER_STATES];
      static void copyTransitions(const size_t &from, const size_t &to);
      static void setWhitespace(const size_t &at, const size_t &next);
      static void initialize();
      Parser();
      Parser(const Parser &input);
      ~Parser();
      Parser & operator=(const Parser &input);
    public:
      static GlideJson parse(const std::string &input);
      static GlideJson parse(const char * const &cInput, const size_t &size);
  };

  class ParserInitializer {
    friend class Parser;
    private:
      ParserInitializer();
      ParserInitializer(const ParserInitializer &input);
    public:
      ~ParserInitializer();
      ParserInitializer & operator=(const ParserInitializer &input);
  };

}

#endif
