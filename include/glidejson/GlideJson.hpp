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

#ifndef GLIDE_JSON_HPP
#define GLIDE_JSON_HPP

extern "C" {
  #include "siphash.h"
  #include "halfsiphash.h"
}

#include <stdexcept>
#include <atomic>
#include <random>
#include <unordered_map>
#include <list>
#include <map>
#include <string>
#include <vector>

#define SIP_HASH_SECRET_SIZE 16
#define GLIDE_BYTE_SIZE 256
#define GLIDE_BYTE_WIDTH 8
#define GLIDE_BYTE_HALF_WIDTH 4
#define GLIDE_JSON_ENCODER_STATES 23
#define GLIDE_JSON_PARSER_STATES 69

class GlideError : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

// ========================================

class GlideLfs;

class GlideLfsNode {
  friend class GlideLfs;
  private:
    GlideLfsNode *below;
  public:
    GlideLfsNode();
    GlideLfsNode(const GlideLfsNode &input);
    virtual ~GlideLfsNode();
    GlideLfsNode & operator=(const GlideLfsNode &input);
};

// ========================================

class GlideLfs {
  protected:
    std::atomic<GlideLfsNode *> top;
  public:
    GlideLfs();
    GlideLfs(const GlideLfs &input);
    ~GlideLfs();
    GlideLfs & operator=(const GlideLfs &input);
    void push(GlideLfsNode *input);
    GlideLfsNode * pop();
};

// ========================================

template<class T>
class GlideLfsItem : public GlideLfsNode {
  private:
    static GlideLfs itemCache;
    static const T blankItem;
  public:
    T item;
    std::atomic<size_t> heldBy;
    static GlideLfsItem * make();
    void dispose();
};

template<class T>
GlideLfs GlideLfsItem<T>::itemCache;

template<class T>
const T GlideLfsItem<T>::blankItem = T();

template<class T>
GlideLfsItem<T> * GlideLfsItem<T>::make() {
  GlideLfsItem<T> *output((GlideLfsItem<T> *)(itemCache.pop()));
  if(output == NULL) {
    output = new GlideLfsItem<T>();
  }
  return output;
}

template<class T>
void GlideLfsItem<T>::dispose() {
  item = blankItem;
  itemCache.push(this);
}

// ========================================

template<class T>
class GlideItem {
  protected:
    GlideLfsItem<T> *lfsItem;
    const T *item;
  public:
    GlideItem();
    GlideItem(const T *input);
    GlideItem(const T &input);
    GlideItem(T &&input);
    GlideItem(const GlideItem &input);
    GlideItem(GlideItem &&input);
    ~GlideItem();
    GlideItem & operator=(const GlideItem &input);
    GlideItem & operator=(GlideItem &&input);
    const T & value() const;
    T & value();
};

template<class T>
GlideItem<T>::GlideItem() : lfsItem(GlideLfsItem<T>::make()) {
  lfsItem->heldBy = 1;
  item = &(lfsItem->item);
}

template<class T>
GlideItem<T>::GlideItem(const T *input) : lfsItem(NULL), item(input) {
}

template<class T>
GlideItem<T>::GlideItem(const T &input) : lfsItem(GlideLfsItem<T>::make()) {
  lfsItem->item = input;
  lfsItem->heldBy = 1;
  item = &(lfsItem->item);
}

template<class T>
GlideItem<T>::GlideItem(T &&input) : lfsItem(GlideLfsItem<T>::make()) {
  lfsItem->item = std::move(input);
  lfsItem->heldBy = 1;
  item = &(lfsItem->item);
}

template<class T>
GlideItem<T>::GlideItem(const GlideItem<T> &input) : lfsItem(input.lfsItem), item(input.item) {
  lfsItem && ++(lfsItem->heldBy);
}

template<class T>
GlideItem<T>::GlideItem(GlideItem<T> &&input) : lfsItem(input.lfsItem), item(input.item) {
  input.lfsItem = NULL;
  input.item = NULL;
}

template<class T>
GlideItem<T>::~GlideItem() {
  if(lfsItem && !(--(lfsItem->heldBy))) {
    lfsItem->dispose();
  }
}

template<class T>
GlideItem<T> & GlideItem<T>::operator=(const GlideItem<T> &input) {
  if(lfsItem && !(--(lfsItem->heldBy))) {
    lfsItem->dispose();
  }
  lfsItem = input.lfsItem;
  item = input.item;
  lfsItem && ++(lfsItem->heldBy);
  return *this;
}

template<class T>
GlideItem<T> & GlideItem<T>::operator=(GlideItem<T> &&input) {
  if(lfsItem && !(--(lfsItem->heldBy))) {
    lfsItem->dispose();
  }
  lfsItem = input.lfsItem;
  item = input.item;
  input.lfsItem = NULL;
  input.item = NULL;
  return *this;
}

template<class T>
T & GlideItem<T>::value() {
  return *(const_cast<T *>(item));
}

template<class T>
const T & GlideItem<T>::value() const {
  return *item;
}

// ========================================

template<class T>
class GlideSortItem : public GlideItem<T> {
  using GlideItem<T>::GlideItem;
  public:
    bool operator==(const GlideSortItem &input) const;
    bool operator<(const GlideSortItem &input) const;
    bool operator>(const GlideSortItem &input) const;
};

template<class T>
bool GlideSortItem<T>::operator==(const GlideSortItem<T> &input) const {
  return *(GlideItem<T>::item) == *(input.item);
}

template<class T>
bool GlideSortItem<T>::operator<(const GlideSortItem<T> &input) const {
  return *(GlideItem<T>::item) < *(input.item);
}

template<class T>
bool GlideSortItem<T>::operator>(const GlideSortItem<T> &input) const {
  return *(GlideItem<T>::item) > *(input.item);
}

// ========================================

struct GlideErrorHasher {
  size_t operator()(const GlideSortItem<std::string> &key) const;
};

class Glide32Hasher {
  protected:
    std::vector<unsigned char> secret;
  public:
    Glide32Hasher();
    Glide32Hasher(const Glide32Hasher &input);
    Glide32Hasher(Glide32Hasher &&input);
    ~Glide32Hasher();
    Glide32Hasher & operator=(const Glide32Hasher &input);
    Glide32Hasher & operator=(Glide32Hasher &&input);
    size_t operator()(const GlideSortItem<std::string> &key) const;
};

class Glide64Hasher : public Glide32Hasher {
  using Glide32Hasher::Glide32Hasher;
  public:
    size_t operator()(const GlideSortItem<std::string> &key) const;
};

typedef std::conditional<
  sizeof(size_t) == 8,
  Glide64Hasher,
  std::conditional<
    sizeof(size_t) == 4,
    Glide32Hasher,
    GlideErrorHasher
  >::type
>::type GlideHasher;

// ========================================

template<class T>
class GlideHashMapIterator {
  private:
    T iterator;
  public:
    GlideHashMapIterator();
    GlideHashMapIterator(const T &input);
    GlideHashMapIterator(const GlideHashMapIterator &input);
    ~GlideHashMapIterator();
    GlideHashMapIterator & operator=(const GlideHashMapIterator &input);
    bool operator==(const GlideHashMapIterator &input) const;
    bool operator!=(const GlideHashMapIterator &input) const;
    void next();
    void previous();
    auto key() const -> decltype(iterator->first.value());
    auto value() const -> decltype(iterator->second.value());
};

template<class T>
GlideHashMapIterator<T>::GlideHashMapIterator() : iterator() {
}

template<class T>
GlideHashMapIterator<T>::GlideHashMapIterator(const T &input) : iterator(input) {
}

template<class T>
GlideHashMapIterator<T>::GlideHashMapIterator(const GlideHashMapIterator<T> &input) : iterator(input.iterator) {
}

template<class T>
GlideHashMapIterator<T>::~GlideHashMapIterator() {
}

template<class T>
GlideHashMapIterator<T> & GlideHashMapIterator<T>::operator=(const GlideHashMapIterator<T> &input) {
  iterator = input.iterator;
  return *this;
}

template<class T>
bool GlideHashMapIterator<T>::operator==(const GlideHashMapIterator<T> &input) const {
  return iterator == input.iterator;
}

template<class T>
bool GlideHashMapIterator<T>::operator!=(const GlideHashMapIterator<T> &input) const {
  return iterator != input.iterator;
}

template<class T>
void GlideHashMapIterator<T>::next() {
  ++iterator;
}

template<class T>
void GlideHashMapIterator<T>::previous() {
  --iterator;
}

template<class T>
auto GlideHashMapIterator<T>::key() const -> decltype(iterator->first.value()) {
  return iterator->first.value();
}

template<class T>
auto GlideHashMapIterator<T>::value() const -> decltype(iterator->second.value()) {
  return iterator->second.value();
}

// ========================================

template<class T>
class GlideHashMap {
  public:
    typedef GlideSortItem<std::string> Key;
    typedef std::pair< Key, GlideItem<T> > KeyPair;
  private:
    std::list<KeyPair> positionList;
  public:
    typedef std::pair<bool, decltype(positionList.begin())> Position;
    typedef std::pair< Position, GlideItem<T> > ValuePair;
    typedef std::unordered_map<Key, ValuePair, GlideHasher> HashMap;
  private:
    HashMap hashMap;
    inline void reindex();
    static bool lessThan(const KeyPair &x, const KeyPair &y);
    static bool greaterThan(const KeyPair &x, const KeyPair &y);
  public:
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
    auto begin() const -> GlideHashMapIterator<decltype(positionList.begin())>;
    auto end() const -> GlideHashMapIterator<decltype(positionList.end())>;
    auto begin() -> GlideHashMapIterator<decltype(positionList.begin())>;
    auto end() -> GlideHashMapIterator<decltype(positionList.end())>;
    auto rbegin() const -> GlideHashMapIterator<decltype(positionList.rbegin())>;
    auto rend() const -> GlideHashMapIterator<decltype(positionList.rend())>;
    auto rbegin() -> GlideHashMapIterator<decltype(positionList.rbegin())>;
    auto rend() -> GlideHashMapIterator<decltype(positionList.rend())>;
};

template<class T>
inline void GlideHashMap<T>::reindex() {
  auto i(positionList.begin());
  auto iEnd(positionList.end());
  while(i != iEnd) {
    hashMap[i->first.value()].first.second = i++;
  }
}

template<class T>
bool GlideHashMap<T>::lessThan(const KeyPair &x, const KeyPair &y) {
  return x.first < y.first;
}

template<class T>
bool GlideHashMap<T>::greaterThan(const KeyPair &x, const KeyPair &y) {
  return x.first > y.first;
}

template<class T>
GlideHashMap<T>::GlideHashMap() : positionList(), hashMap() {
}

template<class T>
GlideHashMap<T>::GlideHashMap(const GlideHashMap<T> &input) : positionList(input.positionList), hashMap(input.hashMap) {
  reindex();
}

template<class T>
GlideHashMap<T>::GlideHashMap(GlideHashMap<T> &&input) : positionList(std::move(input.positionList)), hashMap(std::move(input.hashMap)) {
}

template<class T>
GlideHashMap<T>::~GlideHashMap() {
}

template<class T>
GlideHashMap<T> & GlideHashMap<T>::operator=(const GlideHashMap<T> &input) {
  hashMap = input.hashMap;
  positionList = input.positionList;
  reindex();
  return *this;
}

template<class T>
GlideHashMap<T> & GlideHashMap<T>::operator=(GlideHashMap<T> &&input) {
  hashMap = std::move(input.hashMap);
  positionList = std::move(input.positionList);
  return *this;
}

template<class T>
size_t GlideHashMap<T>::size() const {
  return hashMap.size();
}

template<class T>
size_t GlideHashMap<T>::count(const std::string &key) const {
  const Key itemKey(&key);
  return hashMap.count(itemKey);
}

template<class T>
bool GlideHashMap<T>::empty() const {
  return hashMap.empty();
}

template<class T>
const T & GlideHashMap<T>::at(const std::string &key) const {
  const Key itemKey(&key);
  return hashMap.at(itemKey).second.value();
}

template<class T>
T & GlideHashMap<T>::at(const std::string &key) {
  const Key itemKey(&key);
  return hashMap.at(itemKey).second.value();
}

template<class T>
T & GlideHashMap<T>::operator[](const std::string &key) {
  Key newItemKey(key);
  ValuePair &pair = hashMap[newItemKey];
  if(!pair.first.first) {
    positionList.push_back(KeyPair(std::move(newItemKey), pair.second));
    pair.first.first = true;
    pair.first.second = positionList.end();
    --(pair.first.second);
  }
  return pair.second.value();
}

template<class T>
T & GlideHashMap<T>::operator[](std::string &&key) {
  Key newItemKey(std::move(key));
  ValuePair &pair = hashMap[newItemKey];
  if(!pair.first.first) {
    positionList.push_back(KeyPair(std::move(newItemKey), pair.second));
    pair.first.first = true;
    pair.first.second = positionList.end();
    --(pair.first.second);
  }
  return pair.second.value();
}

template<class T>
size_t GlideHashMap<T>::erase(const std::string &key) {
  const Key itemKey(&key);
  if(hashMap.count(itemKey)) {
    positionList.erase(hashMap[itemKey].first.second);
    return hashMap.erase(itemKey);
  }
  return 0;
}

template<class T>
void GlideHashMap<T>::clear() {
  hashMap.clear();
  positionList.clear();
}

template<class T>
void GlideHashMap<T>::sort() {
  positionList.sort(GlideHashMap::lessThan);
}

template<class T>
void GlideHashMap<T>::rsort() {
  positionList.sort(GlideHashMap::greaterThan);
}

template<class T>
auto GlideHashMap<T>::begin() const -> GlideHashMapIterator<decltype(positionList.begin())> {
  return GlideHashMapIterator<decltype(positionList.begin())>(positionList.begin());
}

template<class T>
auto GlideHashMap<T>::end() const -> GlideHashMapIterator<decltype(positionList.end())> {
  return GlideHashMapIterator<decltype(positionList.end())>(positionList.end());
}

template<class T>
auto GlideHashMap<T>::begin() -> GlideHashMapIterator<decltype(positionList.begin())> {
  return GlideHashMapIterator<decltype(positionList.begin())>(positionList.begin());
}

template<class T>
auto GlideHashMap<T>::end() -> GlideHashMapIterator<decltype(positionList.end())> {
  return GlideHashMapIterator<decltype(positionList.end())>(positionList.end());
}

template<class T>
auto GlideHashMap<T>::rbegin() const -> GlideHashMapIterator<decltype(positionList.rbegin())> {
  return GlideHashMapIterator<decltype(positionList.rbegin())>(positionList.rbegin());
}

template<class T>
auto GlideHashMap<T>::rend() const -> GlideHashMapIterator<decltype(positionList.rend())> {
  return GlideHashMapIterator<decltype(positionList.rend())>(positionList.rend());
}

template<class T>
auto GlideHashMap<T>::rbegin() -> GlideHashMapIterator<decltype(positionList.rbegin())> {
  return GlideHashMapIterator<decltype(positionList.rbegin())>(positionList.rbegin());
}

template<class T>
auto GlideHashMap<T>::rend() -> GlideHashMapIterator<decltype(positionList.rend())> {
  return GlideHashMapIterator<decltype(positionList.rend())>(positionList.rend());
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
    inline void initialize(GlideJson::Type input);
  public:
    GlideJson();
    GlideJson(GlideJson::Type input);
    GlideJson(const GlideJson &input);
    GlideJson(GlideJson &&input);
    explicit GlideJson(bool input);
    explicit GlideJson(int input);
    explicit GlideJson(unsigned int input);
    explicit GlideJson(long int input);
    explicit GlideJson(long unsigned int input);
    explicit GlideJson(size_t count, char input);
    explicit GlideJson(const char *input);
    explicit GlideJson(const char *input, size_t size);
    explicit GlideJson(const std::string &input);
    explicit GlideJson(std::string &&input);
    ~GlideJson();
    GlideJson & operator=(GlideJson::Type input);
    GlideJson & operator=(const GlideJson &input);
    GlideJson & operator=(GlideJson &&input);
    GlideJson & operator=(bool input);
    GlideJson & operator=(int input);
    GlideJson & operator=(unsigned int input);
    GlideJson & operator=(long int input);
    GlideJson & operator=(unsigned long int input);
    bool setNumber(const std::string &input);
    bool setNumber(const char *input, size_t size);
    GlideJson & setString(size_t count, char input);
    GlideJson & operator=(const char *input);
    GlideJson & setString(const char *input, size_t size);
    GlideJson & operator=(const std::string &input);
    GlideJson & operator=(std::string &&input);
  private:
    std::string toJson(GlideJson::Whitespace type, size_t depth) const;
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
    std::string toJson(GlideJson::Whitespace type) const;
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
    bool & boolean();
    std::string & string();
    std::vector<GlideJson> & array();
    GlideHashMap<GlideJson> & object();
    static unsigned char getHex(unsigned char input);
    static GlideJson parse(const std::string &input);
    static GlideJson parse(const char *input, size_t size);
    static std::string encodeString(const std::string &input);
    static std::string encodeString(const char *input, size_t size);
    static std::string base64Encode(const std::string &input);
    static std::string base64Encode(const char *input, size_t size);
    static std::string base64Decode(const std::string &input);
    static std::string base64Decode(const char *input, size_t size);
};

namespace GlideJsonScheme {

  class EncoderInitializer;

  class Encoder {
    friend class EncoderInitializer;
    friend unsigned char GlideJson::getHex(unsigned char input);
    private:
      static unsigned char hexMap[16];
      static unsigned char stateMap[GLIDE_BYTE_SIZE * GLIDE_JSON_ENCODER_STATES];
      static unsigned char b64eMap[64];
      static unsigned char b64dMap[GLIDE_BYTE_SIZE];
      static void setEscapable(size_t state);
      static void copyTransitions(size_t from, size_t to);
      static void initialize();
      Encoder();
      Encoder(const Encoder &input);
    public:
      ~Encoder();
      Encoder & operator=(const Encoder &input);
      static std::string encode(const std::string &input);
      static std::string encode(const char *cInput, size_t size);
      static std::string base64Encode(const std::string &input);
      static std::string base64Encode(const char *cInput, size_t length);
      static std::string base64Decode(const std::string &input);
      static std::string base64Decode(const char *cInput, size_t length);
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

  class Base : public GlideLfsNode {
    protected:
      Base();
      Base(const Base &input);
    public:
      virtual ~Base();
      Base & operator=(const Base &input);
      virtual GlideJson::Type getType() const;
      virtual std::string toJson() const;
      virtual std::string toJson(GlideJson::Whitespace type, size_t depth) const;
      virtual const std::string & theError() const;
      virtual const bool & theBoolean() const;
      virtual bool & theBoolean();
      virtual const std::string & theNumber() const;
      virtual const std::string & theString() const;
      virtual std::string & theString();
      virtual const std::vector<GlideJson> & theArray() const;
      virtual std::vector<GlideJson> & theArray();
      virtual const GlideHashMap<GlideJson> & theObject() const;
      virtual GlideHashMap<GlideJson> & theObject();
      virtual void dispose();
      virtual Base * duplicate() const;
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
      virtual std::string toJson(GlideJson::Whitespace type, size_t depth) const;
      const std::string & theError() const;
    private:
      static GlideLfs errorCache;
    public:
      static Error * make();
      virtual void dispose();
      virtual Error * duplicate() const;
  };

  class Null : public Base {
    public:
      Null();
      Null(const Null &input);
      virtual ~Null();
      Null & operator=(const Null &input);
      virtual GlideJson::Type getType() const;
      virtual std::string toJson() const;
      virtual std::string toJson(GlideJson::Whitespace type, size_t depth) const;
      static Null * soleNull();
      virtual void dispose();
      virtual Null * duplicate() const;
  };

  class Boolean : public Base {
    public:
      bool boolean;
      Boolean();
      Boolean(bool input);
      Boolean(const Boolean &input);
      virtual ~Boolean();
      Boolean & operator=(const Boolean &input);
      virtual GlideJson::Type getType() const;
      virtual std::string toJson() const;
      virtual std::string toJson(GlideJson::Whitespace type, size_t depth) const;
      virtual const bool & theBoolean() const;
      virtual bool & theBoolean();
    private:
      static GlideLfs booleanCache;
    public:
      static Boolean * make();
      virtual void dispose();
      virtual Boolean * duplicate() const;
  };

  class Number : public Base {
    friend class Parser;
    protected:
      std::string number;
    public:
      Number();
      Number(const std::string &input);
      Number(int input);
      Number(unsigned int input);
      Number(long int input);
      Number(unsigned long int input);
      Number(const Number &input);
      Number(Number &&input);
      virtual ~Number();
      Number & operator=(int input);
      Number & operator=(unsigned int input);
      Number & operator=(long int input);
      Number & operator=(unsigned long int input);
      Number & operator=(const Number &input);
      Number & operator=(Number &&input);
      virtual GlideJson::Type getType() const;
      virtual std::string toJson() const;
      virtual std::string toJson(GlideJson::Whitespace type, size_t depth) const;
      virtual const std::string & theNumber() const;
      bool set(const std::string &input);
      bool set(const char *input, size_t size);
    private:
      static GlideLfs numberCache;
    public:
      static Number * make();
      virtual void dispose();
      virtual Number * duplicate() const;
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
      virtual std::string toJson(GlideJson::Whitespace type, size_t depth) const;
      virtual const std::string & theString() const;
      virtual std::string & theString();
    private:
      static GlideLfs stringCache;
    public:
      static String * make();
      virtual void dispose();
      virtual String * duplicate() const;
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
      virtual std::string toJson(GlideJson::Whitespace type, size_t depth) const;
      virtual const std::vector<GlideJson> & theArray() const;
      virtual std::vector<GlideJson> & theArray();
    private:
      static GlideLfs arrayCache;
    public:
      static Array * make();
      virtual void dispose();
      virtual Array * duplicate() const;
  };

  class Object : public Base {
    public:
      GlideHashMap<GlideJson> object;
      Object();
      Object(const GlideHashMap<GlideJson> &input);
      Object(const Object &input);
      Object(Object &&input);
      virtual ~Object();
      Object & operator=(const Object &input);
      Object & operator=(Object &&input);
      virtual GlideJson::Type getType() const;
      virtual std::string toJson() const;
      virtual std::string toJson(GlideJson::Whitespace type, size_t depth) const;
      virtual const GlideHashMap<GlideJson> & theObject() const;
      virtual GlideHashMap<GlideJson> & theObject();
    private:
      static GlideLfs objectCache;
    public:
      static Object * make();
      virtual void dispose();
      virtual Object * duplicate() const;
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
      static GlideJson parse(const char *cInput, size_t size);
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
