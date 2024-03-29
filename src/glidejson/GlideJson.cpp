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

#include "GlideJson.hpp"

GlideLfsNode::GlideLfsNode() : below(NULL) {
}

GlideLfsNode::GlideLfsNode(const GlideLfsNode &input) : below(NULL) {
  (void)input;
  throw GlideError("GlideLfsNode::GlideLfsNode(const GlideLfsNode &input): No copy constructor!");
}

GlideLfsNode::~GlideLfsNode() {
}

GlideLfsNode & GlideLfsNode::operator=(const GlideLfsNode &input) {
  (void)input;
  throw GlideError("GlideLfsNode::operator=(const GlideLfsNode &input): No assignment operator!");
  return *this;
}

// ========================================

GlideLfs::GlideLfs() : top(NULL) {
}

GlideLfs::GlideLfs(const GlideLfs &input) : top(NULL) {
  (void)input;
  throw GlideError("GlideLfs::GlideLfs(const GlideLfs &input): No copy constructor!");
}

GlideLfs::~GlideLfs() {
  GlideLfsNode *current;
  while((current = pop())) {
    delete current;
  }
}

GlideLfs & GlideLfs::operator=(const GlideLfs &input) {
  (void)input;
  throw GlideError("GlideLfs::operator=(const GlideLfs &input): No assignment operator!");
  return *this;
}

void GlideLfs::push(GlideLfsNode *input) {
  /*
    - If "input->below" equals "top", set "top" to "input".
    - If "input->below" does not equal "top", set "input->below" to "top".
  */
  input->below = top.load(std::memory_order_relaxed);
  while(!top.compare_exchange_weak(input->below, input, std::memory_order_release, std::memory_order_relaxed));
}

GlideLfsNode * GlideLfs::pop() {
  /*
    - If "output" equals "top", set "top" to "output->below".
    - If "output" does not equal "top", set "output" to "top".
  */
  GlideLfsNode *output(top.load(std::memory_order_relaxed));
  do {
    if(output == NULL) {
      return output;
    }
  }
  while(!top.compare_exchange_weak(output, output->below, std::memory_order_release, std::memory_order_relaxed));
  output->below = NULL;
  return output;
}

// ========================================

Glide32Hasher::Glide32Hasher() : secret(SIP_HASH_SECRET_SIZE) {
  std::random_device generator;
  unsigned char * pSecret(secret.data());
  size_t index(0);
  do {
    *((unsigned int *)(pSecret + index)) = generator();
    index += sizeof(unsigned int);
  }
  while(index < SIP_HASH_SECRET_SIZE);
}

Glide32Hasher::Glide32Hasher(const Glide32Hasher &input) : secret(input.secret) {
}

Glide32Hasher::Glide32Hasher(Glide32Hasher &&input) : secret(std::move(input.secret)) {
}

Glide32Hasher::~Glide32Hasher() {
}

Glide32Hasher & Glide32Hasher::operator=(const Glide32Hasher &input) {
  secret = input.secret;
  return *this;
}

Glide32Hasher & Glide32Hasher::operator=(Glide32Hasher &&input) {
  secret = std::move(input.secret);
  return *this;
}

size_t Glide32Hasher::operator()(const GlideSortItem<std::string> &key) const {
  size_t output;
  halfsiphash(key.value().data(), key.value().size(), secret.data(), (uint8_t *)(&output), sizeof(output));
  return output;
}

size_t Glide64Hasher::operator()(const GlideSortItem<std::string> &key) const {
  size_t output;
  siphash(key.value().data(), key.value().size(), secret.data(), (uint8_t *)(&output), sizeof(output));
  return output;
}

// ========================================

GlideCheck::GlideCheck() {
  if(sizeof(unsigned char) > 1) {
    throw GlideError("GlideCheck::GlideCheck(): \"unsigned char\" has more than one byte!");
  }
  if(sizeof(size_t) <= 1) {
    throw GlideError("GlideCheck::GlideCheck(): \"size_t\" only has one or zero bytes!");
  }
  unsigned char byte(0);
  size_t mword(0);
  byte = ~byte;
  mword = byte + 1;
  if(mword != GLIDE_BYTE_SIZE) {
    throw GlideError("GlideCheck::GlideCheck(): The byte size is invalid!");
  }
  mword = 1 << GLIDE_BYTE_WIDTH;
  if(mword != GLIDE_BYTE_SIZE) {
    throw GlideError("GlideCheck::GlideCheck(): The byte width is invalid!");
  }
  mword = GLIDE_BYTE_WIDTH >> 1;
  if(mword != GLIDE_BYTE_HALF_WIDTH) {
    throw GlideError("GlideCheck::GlideCheck(): The byte half-width is invalid!");
  }
}

GlideCheck::GlideCheck(const GlideCheck &input) {
  (void)input;
  throw GlideError("GlideCheck::GlideCheck(const GlideCheck &input): No copy constructor!");
}

GlideCheck::~GlideCheck() {
}

GlideCheck & GlideCheck::operator=(const GlideCheck &input) {
  (void)input;
  throw GlideError("GlideCheck::operator=(const GlideCheck &input): No assignment operator!");
  return *this;
}

const GlideCheck GlideCheck::check;

// ========================================

inline void GlideString::nearestPower(const size_t &inputSize, size_t &inputCapacity) {
  inputCapacity = 2;
  size_t current(inputSize - 1);
  size_t currentSplit(GlideString::wordBisector);
  do {
    if(current >> currentSplit) {
      inputCapacity <<= currentSplit;
      current >>= currentSplit;
    }
    currentSplit >>= 1;
  }
  while(currentSplit);
}

inline void GlideString::initialize(const size_t &inputSize, size_t &inputCapacity, std::string &input) {
  if(inputSize > GlideString::initialCapacity) {
    nearestPower(inputSize, inputCapacity);
  }
  else {
    inputCapacity = GlideString::initialCapacity;
  }
  input.resize(inputCapacity);
}

inline void GlideString::append(const unsigned char &inputChar, size_t &inputSize, size_t &inputCapacity, std::string &input) {
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
  if(inputSize == inputCapacity) {
    input.resize((inputCapacity <<= 1));
  }
  input[inputSize++] = inputChar;
  #pragma GCC diagnostic pop
}

// ========================================

inline void GlideJson::initialize(GlideJson::Type input) {
  switch(input) {
    case GlideJson::Error:
      content = GlideJsonScheme::Error::make();
      break;
    case GlideJson::Null:
      content = GlideJsonScheme::Null::soleNull();
      break;
    case GlideJson::Boolean:
      content = GlideJsonScheme::Boolean::make();
      break;
    case GlideJson::Number:
      content = GlideJsonScheme::Number::make();
      break;
    case GlideJson::String:
      content = GlideJsonScheme::String::make();
      break;
    case GlideJson::Array:
      content = GlideJsonScheme::Array::make();
      break;
    case GlideJson::Object:
      content = GlideJsonScheme::Object::make();
      break;
    default:
      abort();
  }
}

GlideJson::GlideJson() {
  content = GlideJsonScheme::Null::soleNull();
}

GlideJson::GlideJson(GlideJson::Type input) {
  initialize(input);
}

GlideJson::GlideJson(const GlideJson &input) {
  content = input.content->duplicate();
}

GlideJson::GlideJson(GlideJson &&input) {
  content = input.content;
  input.content = GlideJsonScheme::Null::soleNull();
}

GlideJson::GlideJson(bool input) {
  content = GlideJsonScheme::Boolean::make();
  ((GlideJsonScheme::Boolean *)content)->boolean = input;
}

GlideJson::GlideJson(int input) {
  content = GlideJsonScheme::Number::make();
  *((GlideJsonScheme::Number *)content) = input;
}

GlideJson::GlideJson(unsigned int input) {
  content = GlideJsonScheme::Number::make();
  *((GlideJsonScheme::Number *)content) = input;
}

GlideJson::GlideJson(long int input) {
  content = GlideJsonScheme::Number::make();
  *((GlideJsonScheme::Number *)content) = input;
}

GlideJson::GlideJson(unsigned long int input) {
  content = GlideJsonScheme::Number::make();
  *((GlideJsonScheme::Number *)content) = input;
}

GlideJson::GlideJson(size_t count, char input) {
  content = GlideJsonScheme::String::make();
  ((GlideJsonScheme::String *)content)->string.assign(count, input);
}

GlideJson::GlideJson(const char *input) {
  content = GlideJsonScheme::String::make();
  ((GlideJsonScheme::String *)content)->string = input;
}

GlideJson::GlideJson(const char *input, size_t size) {
  content = GlideJsonScheme::String::make();
  ((GlideJsonScheme::String *)content)->string.assign(input, size);
}

GlideJson::GlideJson(const std::string &input) {
  content = GlideJsonScheme::String::make();
  ((GlideJsonScheme::String *)content)->string = input;
}

GlideJson::GlideJson(std::string &&input) {
  content = GlideJsonScheme::String::make();
  ((GlideJsonScheme::String *)content)->string = std::move(input);
}

GlideJson::~GlideJson() {
  content->dispose();
}

GlideJson & GlideJson::operator=(GlideJson::Type input) {
  content->dispose();
  initialize(input);
  return *this;
}

GlideJson & GlideJson::operator=(const GlideJson &input) {
  content->dispose();
  content = input.content->duplicate();
  return *this;
}

GlideJson & GlideJson::operator=(GlideJson &&input) {
  content->dispose();
  content = input.content;
  input.content = GlideJsonScheme::Null::soleNull();
  return *this;
}

GlideJson & GlideJson::operator=(bool input) {
  content->dispose();
  content = GlideJsonScheme::Boolean::make();
  ((GlideJsonScheme::Boolean *)content)->boolean = input;
  return *this;
}

GlideJson & GlideJson::operator=(int input) {
  content->dispose();
  content = GlideJsonScheme::Number::make();
  *((GlideJsonScheme::Number *)content) = input;
  return *this;
}

GlideJson & GlideJson::operator=(unsigned int input) {
  content->dispose();
  content = GlideJsonScheme::Number::make();
  *((GlideJsonScheme::Number *)content) = input;
  return *this;
}

GlideJson & GlideJson::operator=(long int input) {
  content->dispose();
  content = GlideJsonScheme::Number::make();
  *((GlideJsonScheme::Number *)content) = input;
  return *this;
}

GlideJson & GlideJson::operator=(unsigned long int input) {
  content->dispose();
  content = GlideJsonScheme::Number::make();
  *((GlideJsonScheme::Number *)content) = input;
  return *this;
}

bool GlideJson::setNumber(const char *input, size_t size) {
  content->dispose();
  content = GlideJsonScheme::Number::make();
  return ((GlideJsonScheme::Number *)content)->set(input, size);
}

bool GlideJson::setNumber(const std::string &input) {
  content->dispose();
  content = GlideJsonScheme::Number::make();
  return ((GlideJsonScheme::Number *)content)->set(input);
}

GlideJson & GlideJson::setString(size_t count, char input) {
  content->dispose();
  content = GlideJsonScheme::String::make();
  ((GlideJsonScheme::String *)content)->string.assign(count, input);
  return *this;
}

GlideJson & GlideJson::operator=(const char *input) {
  content->dispose();
  content = GlideJsonScheme::String::make();
  ((GlideJsonScheme::String *)content)->string = input;
  return *this;
}

GlideJson & GlideJson::setString(const char *input, size_t size) {
  content->dispose();
  content = GlideJsonScheme::String::make();
  ((GlideJsonScheme::String *)content)->string.assign(input, size);
  return *this;
}

GlideJson & GlideJson::operator=(const std::string &input) {
  content->dispose();
  content = GlideJsonScheme::String::make();
  ((GlideJsonScheme::String *)content)->string = input;
  return *this;
}

GlideJson & GlideJson::operator=(std::string &&input) {
  content->dispose();
  content = GlideJsonScheme::String::make();
  ((GlideJsonScheme::String *)content)->string = std::move(input);
  return *this;
}

std::string GlideJson::toJson(GlideJson::Whitespace type, size_t depth) const {
  return content->toJson(type, depth);
}

GlideJson::Type GlideJson::getType() const {
  return content->getType();
}

bool GlideJson::isError() const {
  return content->getType() == GlideJson::Error;
}

bool GlideJson::isNull() const {
  return content->getType() == GlideJson::Null;
}

bool GlideJson::isBoolean() const {
  return content->getType() == GlideJson::Boolean;
}

bool GlideJson::isNumber() const {
  return content->getType() == GlideJson::Number;
}

bool GlideJson::isString() const {
  return content->getType() == GlideJson::String;
}

bool GlideJson::isArray() const {
  return content->getType() == GlideJson::Array;
}

bool GlideJson::isObject() const {
  return content->getType() == GlideJson::Object;
}

bool GlideJson::notError() const {
  return content->getType() != GlideJson::Error;
}

bool GlideJson::notNull() const {
  return content->getType() != GlideJson::Null;
}

bool GlideJson::notBoolean() const {
  return content->getType() != GlideJson::Boolean;
}

bool GlideJson::notNumber() const {
  return content->getType() != GlideJson::Number;
}

bool GlideJson::notString() const {
  return content->getType() != GlideJson::String;
}

bool GlideJson::notArray() const {
  return content->getType() != GlideJson::Array;
}

bool GlideJson::notObject() const {
  return content->getType() != GlideJson::Object;
}

std::string GlideJson::toJson() const {
  return content->toJson();
}

std::string GlideJson::toJson(GlideJson::Whitespace type) const {
  return content->toJson(type, 0);
}

const std::string & GlideJson::error() const {
  return content->theError();
}

const bool & GlideJson::boolean() const {
  return content->theBoolean();
}

const std::string & GlideJson::number() const {
  return content->theNumber();
}

const std::string & GlideJson::string() const {
  return content->theString();
}

const std::vector<GlideJson> & GlideJson::array() const {
  return content->theArray();
}

const GlideHashMap<GlideJson> & GlideJson::object() const {
  return content->theObject();
}

int GlideJson::toInt() const {
  return std::stoi(content->theNumber());
}

unsigned int GlideJson::toUInt() const {
  return std::stoul(content->theNumber());
}

long int GlideJson::toLong() const {
  return std::stol(content->theNumber());
}

unsigned long int GlideJson::toULong() const {
  return std::stoul(content->theNumber());
}

bool & GlideJson::boolean() {
  return content->theBoolean();
}

std::string & GlideJson::string() {
  return content->theString();
}

std::vector<GlideJson> & GlideJson::array() {
  return content->theArray();
}

GlideHashMap<GlideJson> & GlideJson::object() {
  return content->theObject();
}

unsigned char GlideJson::getHex(unsigned char input) {
  static const GlideJsonScheme::EncoderInitializer &encoderInitializer(GlideJsonScheme::EncoderInitializer::initializer());
  (void)encoderInitializer;
  //
  return GlideJsonScheme::Encoder::hexMap[input];
}

GlideJson GlideJson::parse(const std::string &input) {
  return GlideJsonScheme::Parser::parse(input);
}

GlideJson GlideJson::parse(const char *input, size_t size) {
  return GlideJsonScheme::Parser::parse(input, size);
}

std::string GlideJson::encodeString(const std::string &input) {
  return GlideJsonScheme::Encoder::encode(input);
}

std::string GlideJson::encodeString(const char *input, size_t size) {
  return GlideJsonScheme::Encoder::encode(input, size);
}

std::string GlideJson::base64Encode(const std::string &input) {
  return GlideJsonScheme::Encoder::base64Encode(input);
}

std::string GlideJson::base64Encode(const char *input, size_t size) {
  return GlideJsonScheme::Encoder::base64Encode(input, size);
}

std::string GlideJson::base64Decode(const std::string &input) {
  return GlideJsonScheme::Encoder::base64Decode(input);
}

std::string GlideJson::base64Decode(const char *input, size_t size) {
  return GlideJsonScheme::Encoder::base64Decode(input, size);
}

// ========================================

namespace GlideJsonScheme {

  unsigned char Encoder::hexMap[] = {0};
  unsigned char Encoder::stateMap[GLIDE_BYTE_SIZE * GLIDE_JSON_ENCODER_STATES] = {0};
  unsigned char Encoder::b64eMap[] = {0};
  unsigned char Encoder::b64dMap[GLIDE_BYTE_SIZE] = {0};

  void Encoder::setEscapable(size_t state) {
    // Control characters:
    unsigned char i(0);
    do {
      stateMap[i + GLIDE_BYTE_SIZE * state] = 1;
    }
    while(++i <= 31);
    // Special characters escaped with a backslash:
    stateMap['"' + GLIDE_BYTE_SIZE * state] = 2;
    stateMap['\\' + GLIDE_BYTE_SIZE * state] = 3;
    stateMap['\b' + GLIDE_BYTE_SIZE * state] = 4;
    stateMap['\f' + GLIDE_BYTE_SIZE * state] = 5;
    stateMap['\n' + GLIDE_BYTE_SIZE * state] = 6;
    stateMap['\r' + GLIDE_BYTE_SIZE * state] = 7;
    stateMap['\t' + GLIDE_BYTE_SIZE * state] = 8;
  }

  void Encoder::copyTransitions(size_t from, size_t to) {
    size_t i(0);
    do {
      stateMap[i + GLIDE_BYTE_SIZE * to] = stateMap[i + GLIDE_BYTE_SIZE * from];
    }
    while(++i <= 255);
  }

  /*
    This "Encoder" is a FSM guided by a state map. The state map is always
    the width of an unsigned char multiplied by the number of states.
    Every character from an input string is mapped to a state. The
    conversion of strings to JSON strings is binary-safe, so this FSM has
    no failure states. Literally any combination of bytes leads to a valid
    UTF-8 JSON string.

    Grammar for UTF-8 from RFC 3629:

    UTF8-octets = *( UTF8-char )
    UTF8-char   = UTF8-1 / UTF8-2 / UTF8-3 / UTF8-4
    UTF8-1      = %x00-7F
    UTF8-2      = %xC2-DF UTF8-tail
    UTF8-3      = %xE0 %xA0-BF UTF8-tail /
                  %xE1-EC 2( UTF8-tail ) /
                  %xED %x80-9F UTF8-tail /
                  %xEE-EF 2( UTF8-tail )
    UTF8-4      = %xF0 %x90-BF 2( UTF8-tail ) /
                  %xF1-F3 3( UTF8-tail ) /
                  %xF4 %x80-8F 2( UTF8-tail )
    UTF8-tail   = %x80-BF
  */
  void Encoder::initialize() {
    // Map integers to hex characters:
    unsigned char i(0);
    do {
      hexMap[i] = i + 48;
    }
    while(++i <= 9);
    i = 10;
    do {
      hexMap[i] = i + 87;
    }
    while(++i <= 15);
    // Escapable at state 0:
    Encoder::setEscapable(0);
    // Non-ASCII characters:
    i = 128;
    do {
      stateMap[i] = 9;
    }
    while(++i <= 193);
    i = 255;
    do {
      stateMap[i] = 9;
    }
    while(--i >= 245);
    // UTF-8 pair:
    i = 194;
    do {
      stateMap[i] = 10;
    }
    while(++i <= 223);
    // UTF-8 triple:
    stateMap[224] = 12;
    i = 160;
    do {
      stateMap[i + GLIDE_BYTE_SIZE * 12] = 15;
    }
    while(++i <= 191);
    i = 225;
    do {
      stateMap[i] = 13;
    }
    while(++i <= 236);
    stateMap[238] = 13;
    stateMap[239] = 13;
    stateMap[237] = 14;
    i = 128;
    do {
      stateMap[i + GLIDE_BYTE_SIZE * 14] = 15;
    }
    while(++i <= 159);
    // UTF-8 quad:
    stateMap[240] = 17;
    i = 144;
    do {
      stateMap[i + GLIDE_BYTE_SIZE * 17] = 20;
    }
    while(++i <= 191);
    stateMap[241] = 18;
    stateMap[242] = 18;
    stateMap[243] = 18;
    stateMap[244] = 19;
    i = 128;
    do {
      stateMap[i + GLIDE_BYTE_SIZE * 19] = 20;
    }
    while(++i <= 143);
    // Trailing UTF-8 bytes:
    i = 128;
    do {
      stateMap[i + GLIDE_BYTE_SIZE * 10] = 11;
      stateMap[i + GLIDE_BYTE_SIZE * 13] = 15;
      stateMap[i + GLIDE_BYTE_SIZE * 15] = 16;
      stateMap[i + GLIDE_BYTE_SIZE * 18] = 20;
      stateMap[i + GLIDE_BYTE_SIZE * 20] = 21;
      stateMap[i + GLIDE_BYTE_SIZE * 21] = 22;
    }
    while(++i <= 191);
    // Invalid UTF-8 triple:
    Encoder::setEscapable(12);
    i = 128;
    do {
      stateMap[i + GLIDE_BYTE_SIZE * 12] = 9;
    }
    while(++i <= 159);
    i = 255;
    do {
      stateMap[i + GLIDE_BYTE_SIZE * 12] = 9;
    }
    while(--i >= 192);
    Encoder::setEscapable(14);
    i = 255;
    do {
      stateMap[i + GLIDE_BYTE_SIZE * 14] = 9;
    }
    while(--i >= 160);
    // Invalid UTF-8 quad:
    Encoder::setEscapable(17);
    i = 128;
    do {
      stateMap[i + GLIDE_BYTE_SIZE * 17] = 9;
    }
    while(++i <= 143);
    i = 255;
    do {
      stateMap[i + GLIDE_BYTE_SIZE * 17] = 9;
    }
    while(--i >= 192);
    Encoder::setEscapable(19);
    i = 255;
    do {
      stateMap[i + GLIDE_BYTE_SIZE * 19] = 9;
    }
    while(--i >= 144);
    // Invalid trailing UTF-8 bytes:
    Encoder::setEscapable(10);
    Encoder::setEscapable(13);
    Encoder::setEscapable(15);
    Encoder::setEscapable(18);
    Encoder::setEscapable(20);
    Encoder::setEscapable(21);
    i = 255;
    do {
      stateMap[i + GLIDE_BYTE_SIZE * 10] = 9;
      stateMap[i + GLIDE_BYTE_SIZE * 13] = 9;
      stateMap[i + GLIDE_BYTE_SIZE * 15] = 9;
      stateMap[i + GLIDE_BYTE_SIZE * 18] = 9;
      stateMap[i + GLIDE_BYTE_SIZE * 20] = 9;
      stateMap[i + GLIDE_BYTE_SIZE * 21] = 9;
    }
    while(--i >= 192);
    /*
      All the following states need to behave exactly the same as state
      0. They all terminate handlers of special characters, so they need to
      behave as though the FSM is at state 0 again.
    */
    Encoder::copyTransitions(0, 1);
    Encoder::copyTransitions(0, 2);
    Encoder::copyTransitions(0, 3);
    Encoder::copyTransitions(0, 4);
    Encoder::copyTransitions(0, 5);
    Encoder::copyTransitions(0, 6);
    Encoder::copyTransitions(0, 7);
    Encoder::copyTransitions(0, 8);
    Encoder::copyTransitions(0, 9);
    Encoder::copyTransitions(0, 11);
    Encoder::copyTransitions(0, 16);
    Encoder::copyTransitions(0, 22);
    // Base64 encoding map:
    i = 0;
    do {
      b64eMap[i] = i + 65;
      b64dMap[i + 65] = i;
    }
    while(++i < 26);
    i = 26;
    do {
      b64eMap[i] = i + 71;
      b64dMap[i + 71] = i;
    }
    while(++i < 52);
    i = 52;
    do {
      b64eMap[i] = i - 4;
      b64dMap[i - 4] = i;
    }
    while(++i < 62);
    b64eMap[62] = '+';
    b64dMap['+'] = 62;
    b64eMap[63] = '/';
    b64dMap['/'] = 63;
  }

  Encoder::Encoder() {
    throw GlideError("GlideJsonScheme::Encoder::Encoder(): No default constructor!");
  }

  Encoder::Encoder(const Encoder &input) {
    (void)input;
    throw GlideError("GlideJsonScheme::Encoder::Encoder(const Encoder &input): No copy constructor!");
  }

  Encoder::~Encoder() {
  }

  Encoder & Encoder::operator=(const Encoder &input) {
    (void)input;
    throw GlideError("GlideJsonScheme::Encoder::operator=(const Encoder &input): No assignment operator!");
    return *this;
  }

  /*
    See the source file for comments:
  */
  #define GLIDE_JSON_PART_STDSTRING
  #include "Encoder.inc"
  #undef GLIDE_JSON_PART_STDSTRING

  #define GLIDE_JSON_PART_CSTRING
  #include "Encoder.inc"
  #undef GLIDE_JSON_PART_CSTRING

  // ========================================

  EncoderInitializer::EncoderInitializer() {
    Encoder::initialize();
  }

  EncoderInitializer::EncoderInitializer(const EncoderInitializer &input) {
    (void)input;
    throw GlideError("GlideJsonScheme::EncoderInitializer::EncoderInitializer(const EncoderInitializer &input): No copy constructor!");
  }

  EncoderInitializer::~EncoderInitializer() {
  }

  EncoderInitializer & EncoderInitializer::operator=(const EncoderInitializer &input) {
    (void)input;
    throw GlideError("GlideJsonScheme::EncoderInitializer::operator=(const EncoderInitializer &input): No assignment operator!");
    return *this;
  }

  const EncoderInitializer & EncoderInitializer::initializer() {
    static const EncoderInitializer initializer;
    return initializer;
  }

  // ========================================

  Base::Base() : GlideLfsNode() {
  }

  Base::Base(const Base &input) : GlideLfsNode() {
    (void)input;
  }

  Base::~Base() {
  }

  Base & Base::operator=(const Base &input) {
    (void)input;
    return *this;
  }

  GlideJson::Type Base::getType() const {
    throw GlideError("GlideJsonScheme::Base::getType(): This is an abstract class!");
    return GlideJson::Null;
  }

  std::string Base::toJson() const {
    throw GlideError("GlideJsonScheme::Base::toJson(): This is an abstract class!");
    return std::string();
  }

  std::string Base::toJson(GlideJson::Whitespace type, size_t depth) const {
    (void)type;
    (void)depth;
    throw GlideError("GlideJsonScheme::Base::toJson(GlideJson::Whitespace type, size_t depth): This is an abstract class!");
    return std::string();
  }

  const std::string & Base::theError() const {
    static const std::string nothing;
    throw GlideError("GlideJsonScheme::Base::theError(): This is NOT a GlideJsonScheme::Error object!");
    return nothing;
  }

  const bool & Base::theBoolean() const {
    static const bool nothing(false);
    throw GlideError("GlideJsonScheme::Base::theBoolean(): This is NOT a GlideJsonScheme::Boolean object!");
    return nothing;
  }

  bool & Base::theBoolean() {
    static bool nothing(false);
    throw GlideError("GlideJsonScheme::Base::theBoolean(): This is NOT a GlideJsonScheme::Boolean object!");
    return nothing;
  }

  const std::string & Base::theNumber() const {
    static const std::string nothing;
    throw GlideError("GlideJsonScheme::Base::theNumber(): This is NOT a GlideJsonScheme::Number object!");
    return nothing;
  }

  const std::string & Base::theString() const {
    static const std::string nothing;
    throw GlideError("GlideJsonScheme::Base::theString(): This is NOT a GlideJsonScheme::String object!");
    return nothing;
  }

  std::string & Base::theString() {
    static std::string nothing;
    throw GlideError("GlideJsonScheme::Base::theString(): This is NOT a GlideJsonScheme::String object!");
    return nothing;
  }

  const std::vector<GlideJson> & Base::theArray() const {
    static const std::vector<GlideJson> nothing;
    throw GlideError("GlideJsonScheme::Base::theArray(): This is NOT a GlideJsonScheme::Array object!");
    return nothing;
  }

  std::vector<GlideJson> & Base::theArray() {
    static std::vector<GlideJson> nothing;
    throw GlideError("GlideJsonScheme::Base::theArray(): This is NOT a GlideJsonScheme::Array object!");
    return nothing;
  }

  const GlideHashMap<GlideJson> & Base::theObject() const {
    static const GlideHashMap<GlideJson> nothing;
    throw GlideError("GlideJsonScheme::Base::theObject(): This is NOT a GlideJsonScheme::Object object!");
    return nothing;
  }

  GlideHashMap<GlideJson> & Base::theObject() {
    static GlideHashMap<GlideJson> nothing;
    throw GlideError("GlideJsonScheme::Base::theObject(): This is NOT a GlideJsonScheme::Object object!");
    return nothing;
  }

  void Base::dispose() {
    throw GlideError("GlideJsonScheme::Base::dispose(void *input): This is an abstract class!");
  }

  Base * Base::duplicate() const {
    throw GlideError("GlideJsonScheme::Base::duplicate(Base * &location): This is an abstract class!");
  }

  // ========================================

  Error::Error() : Base(), error() {
  }

  Error::Error(const std::string &input) : Base(), error(input) {
  }

  Error::Error(const Error &input) : Base(), error(input.error) {
  }

  Error::Error(Error &&input) : Base(), error(std::move(input.error)) {
  }

  Error::~Error() {
  }

  Error & Error::operator=(const Error &input) {
    error = input.error;
    return *this;
  }

  Error & Error::operator=(Error &&input) {
    error = std::move(input.error);
    return *this;
  }

  GlideJson::Type Error::getType() const {
    return GlideJson::Error;
  }

  std::string Error::toJson() const {
    return Encoder::encode(error);
  }

  std::string Error::toJson(GlideJson::Whitespace type, size_t depth) const {
    (void)type;
    (void)depth;
    return Encoder::encode(error);
  }

  const std::string & Error::theError() const {
    return error;
  }

  GlideLfs Error::errorCache;

  Error * Error::make() {
    Error *output((Error *)(errorCache.pop()));
    if(output == NULL) {
      output = new Error();
    }
    return output;
  }

  void Error::dispose() {
    error.clear();
    errorCache.push(this);
  }

  Error * Error::duplicate() const {
    Error *output((Error *)(errorCache.pop()));
    if(output == NULL) {
      output = new Error();
    }
    output->error = error;
    return output;
  }

  // ========================================

  Null::Null() : Base() {
  }

  Null::Null(const Null &input) : Base() {
    (void)input;
  }

  Null::~Null() {
  }

  Null & Null::operator=(const Null &input) {
    (void)input;
    return *this;
  }

  GlideJson::Type Null::getType() const {
    return GlideJson::Null;
  }

  std::string Null::toJson() const {
    static const std::string nullString("null");
    return nullString;
  }

  std::string Null::toJson(GlideJson::Whitespace type, size_t depth) const {
    static const std::string nullString("null");
    (void)type;
    (void)depth;
    return nullString;
  }

  Null * Null::soleNull() {
    static Null theOnlyNull;
    return &theOnlyNull;
  }

  void Null::dispose() {
  }

  Null * Null::duplicate() const {
    return soleNull();
  }

  // ========================================

  Boolean::Boolean() : Base(), boolean(false) {
  }

  Boolean::Boolean(bool input) : Base(), boolean(input) {
  }

  Boolean::Boolean(const Boolean &input) : Base(), boolean(input.boolean) {
  }

  Boolean::~Boolean() {
  }

  Boolean & Boolean::operator=(const Boolean &input) {
    boolean = input.boolean;
    return *this;
  }

  GlideJson::Type Boolean::getType() const {
    return GlideJson::Boolean;
  }

  std::string Boolean::toJson() const {
    static const std::string falseString("false");
    static const std::string trueString("true");
    return (boolean ? trueString : falseString);
  }

  std::string Boolean::toJson(GlideJson::Whitespace type, size_t depth) const {
    static const std::string falseString("false");
    static const std::string trueString("true");
    (void)type;
    (void)depth;
    return (boolean ? trueString : falseString);
  }

  const bool & Boolean::theBoolean() const {
    return boolean;
  }

  bool & Boolean::theBoolean() {
    return boolean;
  }

  GlideLfs Boolean::booleanCache;

  Boolean * Boolean::make() {
    Boolean *output((Boolean *)(booleanCache.pop()));
    if(output == NULL) {
      output = new Boolean();
    }
    return output;
  }

  void Boolean::dispose() {
    boolean = false;
    booleanCache.push(this);
  }

  Boolean * Boolean::duplicate() const {
    Boolean *output((Boolean *)(booleanCache.pop()));
    if(output == NULL) {
      output = new Boolean();
    }
    output->boolean = boolean;
    return output;
  }

  // ========================================

  Number::Number() : Base(), number("0") {
  }

  Number::Number(const std::string &input) : Base(), number() {
    GlideJson parsed(GlideJson::parse(input));
    if(parsed.content->getType() == GlideJson::Number) {
      number = std::move(((Number *)(parsed.content))->number);
    }
    else {
      number = '0';
    }
  }

  Number::Number(int input) : Base(), number(std::to_string(input)) {
  }

  Number::Number(unsigned int input) : Base(), number(std::to_string(input)) {
  }

  Number::Number(long int input) : Base(), number(std::to_string(input)) {
  }

  Number::Number(unsigned long int input) : Base(), number(std::to_string(input)) {
  }

  Number::Number(const Number &input) : Base(), number(input.number) {
  }

  Number::Number(Number &&input) : Base(), number(std::move(input.number)) {
  }

  Number::~Number() {
  }

  Number & Number::operator=(int input) {
    number = std::to_string(input);
    return *this;
  }

  Number & Number::operator=(unsigned int input) {
    number = std::to_string(input);
    return *this;
  }

  Number & Number::operator=(long int input) {
    number = std::to_string(input);
    return *this;
  }

  Number & Number::operator=(unsigned long int input) {
    number = std::to_string(input);
    return *this;
  }

  Number & Number::operator=(const Number &input) {
    number = input.number;
    return *this;
  }

  Number & Number::operator=(Number &&input) {
    number = std::move(input.number);
    return *this;
  }

  GlideJson::Type Number::getType() const {
    return GlideJson::Number;
  }

  std::string Number::toJson() const {
    return number;
  }

  std::string Number::toJson(GlideJson::Whitespace type, size_t depth) const {
    (void)type;
    (void)depth;
    return number;
  }

  const std::string & Number::theNumber() const {
    return number;
  }

  bool Number::set(const std::string &input) {
    GlideJson parsed(GlideJson::parse(input));
    bool output(parsed.content->getType() == GlideJson::Number);
    if(output) {
      number = std::move(((Number *)(parsed.content))->number);
    }
    return output;
  }

  bool Number::set(const char *input, size_t size) {
    GlideJson parsed(GlideJson::parse(input, size));
    bool output(parsed.content->getType() == GlideJson::Number);
    if(output) {
      number = std::move(((Number *)(parsed.content))->number);
    }
    return output;
  }

  GlideLfs Number::numberCache;

  Number * Number::make() {
    Number *output((Number *)(numberCache.pop()));
    if(output == NULL) {
      output = new Number();
    }
    return output;
  }

  void Number::dispose() {
    number = '0';
    numberCache.push(this);
  }

  Number * Number::duplicate() const {
    Number *output((Number *)(numberCache.pop()));
    if(output == NULL) {
      output = new Number();
    }
    output->number = number;
    return output;
  }

  // ========================================

  String::String() : Base(), string() {
  }

  String::String(const std::string &input) : Base(), string(input) {
  }

  String::String(std::string &&input) : Base(), string(std::move(input)) {
  }

  String::String(const String &input) : Base(), string(input.string) {
  }

  String::String(String &&input) : Base(), string(std::move(input.string)) {
  }

  String::~String() {
  }

  String & String::operator=(const String &input) {
    string = input.string;
    return *this;
  }

  String & String::operator=(String &&input) {
    string = std::move(input.string);
    return *this;
  }

  GlideJson::Type String::getType() const {
    return GlideJson::String;
  }

  std::string String::toJson() const {
    return Encoder::encode(string);
  }

  std::string String::toJson(GlideJson::Whitespace type, size_t depth) const {
    (void)type;
    (void)depth;
    return Encoder::encode(string);
  }

  const std::string & String::theString() const {
    return string;
  }

  std::string & String::theString() {
    return string;
  }

  GlideLfs String::stringCache;

  String * String::make() {
    String *output((String *)(stringCache.pop()));
    if(output == NULL) {
      output = new String();
    }
    return output;
  }

  void String::dispose() {
    string.clear();
    stringCache.push(this);
  }

  String * String::duplicate() const {
    String *output((String *)(stringCache.pop()));
    if(output == NULL) {
      output = new String();
    }
    output->string = string;
    return output;
  }

  // ========================================

  Array::Array() : Base(), array() {
  }

  Array::Array(const std::vector<GlideJson> &input) : Base(), array(input) {
  }

  Array::Array(const Array &input) : Base(), array(input.array) {
  }

  Array::Array(Array &&input) : Base(), array(std::move(input.array)) {
  }

  Array::~Array() {
  }

  Array & Array::operator=(const Array &input) {
    array = input.array;
    return *this;
  }

  Array & Array::operator=(Array &&input) {
    array = std::move(input.array);
    return *this;
  }

  GlideJson::Type Array::getType() const {
    return GlideJson::Array;
  }

  /*
    See the source file for comments:
  */
  #define GLIDE_JSON_NO_WHITESPACE
  #include "Array.inc"
  #undef GLIDE_JSON_NO_WHITESPACE

  #define GLIDE_JSON_WHITESPACE
  #include "Array.inc"
  #undef GLIDE_JSON_WHITESPACE

  const std::vector<GlideJson> & Array::theArray() const {
    return array;
  }

  std::vector<GlideJson> & Array::theArray() {
    return array;
  }

  GlideLfs Array::arrayCache;

  Array * Array::make() {
    Array *output((Array *)(arrayCache.pop()));
    if(output == NULL) {
      output = new Array();
    }
    return output;
  }

  void Array::dispose() {
    array.clear();
    arrayCache.push(this);
  }

  Array * Array::duplicate() const {
    Array *output((Array *)(arrayCache.pop()));
    if(output == NULL) {
      output = new Array();
    }
    output->array = array;
    return output;
  }

  // ========================================

  Object::Object() : Base(), object() {
  }

  Object::Object(const GlideHashMap<GlideJson> &input) : Base(), object(input) {
  }

  Object::Object(const Object &input) : Base(), object(input.object) {
  }

  Object::Object(Object &&input) : Base(), object(std::move(input.object)) {
  }

  Object::~Object() {
  }

  Object & Object::operator=(const Object &input) {
    object = input.object;
    return *this;
  }

  Object & Object::operator=(Object &&input) {
    object = std::move(input.object);
    return *this;
  }

  GlideJson::Type Object::getType() const {
    return GlideJson::Object;
  }

  /*
    See the source file for comments:
  */
  #define GLIDE_JSON_NO_WHITESPACE
  #include "Object.inc"
  #undef GLIDE_JSON_NO_WHITESPACE

  #define GLIDE_JSON_WHITESPACE
  #include "Object.inc"
  #undef GLIDE_JSON_WHITESPACE

  const GlideHashMap<GlideJson> & Object::theObject() const {
    return object;
  }

  GlideHashMap<GlideJson> & Object::theObject() {
    return object;
  }

  GlideLfs Object::objectCache;

  Object * Object::make() {
    Object *output((Object *)(objectCache.pop()));
    if(output == NULL) {
      output = new Object();
    }
    return output;
  }

  void Object::dispose() {
    object.clear();
    objectCache.push(this);
  }

  Object * Object::duplicate() const {
    Object *output((Object *)(objectCache.pop()));
    if(output == NULL) {
      output = new Object();
    }
    output->object = object;
    return output;
  }

  // ========================================

  unsigned char Parser::hexMap[] = {0};
  unsigned char Parser::stateMap[] = {0};
  bool Parser::incompleteMap[] = {0};

  void Parser::copyTransitions(const size_t &from, const size_t &to) {
    size_t i(0);
    do {
      stateMap[i + GLIDE_BYTE_SIZE * to] = stateMap[i + GLIDE_BYTE_SIZE * from];
    }
    while(++i <= 255);
  }

  void Parser::setWhitespace(const size_t &at, const size_t &next) {
    stateMap['\t' + GLIDE_BYTE_SIZE * at] = next;
    stateMap['\n' + GLIDE_BYTE_SIZE * at] = next;
    stateMap['\r' + GLIDE_BYTE_SIZE * at] = next;
    stateMap[' ' + GLIDE_BYTE_SIZE * at] = next;
  }

  /*
    This "Parser" is a FSM guided by a state map. The state map is always
    the width of an unsigned char multiplied by the number of
    states. Every character from an input string is mapped to a
    state. State 0 is always a failure state. State 1 is the point of
    entry.
  */
  void Parser::initialize() {
    // Initialize "incomplete map" to true:
    unsigned char i(0);
    do {
      incompleteMap[i] = true;
    }
    while(++i < GLIDE_JSON_PARSER_STATES);
    // Mapping hex characters to integers:
    i = '0';
    do {
      hexMap[i] = i - 48;
    }
    while(++i <= '9');
    i = 'A';
    do {
      hexMap[i] = i - 55;
    }
    while(++i <= 'F');
    i = 'a';
    do {
      hexMap[i] = i - 87;
    }
    while(++i <= 'f');
    // Null:
    stateMap['n' + GLIDE_BYTE_SIZE] = 2;
    stateMap['u' + GLIDE_BYTE_SIZE * 2] = 3;
    stateMap['l' + GLIDE_BYTE_SIZE * 3] = 4;
    stateMap['l' + GLIDE_BYTE_SIZE * 4] = 5;
    incompleteMap[5] = false;
    // False:
    stateMap['f' + GLIDE_BYTE_SIZE] = 6;
    stateMap['a' + GLIDE_BYTE_SIZE * 6] = 7;
    stateMap['l' + GLIDE_BYTE_SIZE * 7] = 8;
    stateMap['s' + GLIDE_BYTE_SIZE * 8] = 9;
    stateMap['e' + GLIDE_BYTE_SIZE * 9] = 10;
    incompleteMap[10] = false;
    // True:
    stateMap['t' + GLIDE_BYTE_SIZE] = 11;
    stateMap['r' + GLIDE_BYTE_SIZE * 11] = 12;
    stateMap['u' + GLIDE_BYTE_SIZE * 12] = 13;
    stateMap['e' + GLIDE_BYTE_SIZE * 13] = 14;
    incompleteMap[14] = false;
    // Zero:
    stateMap['0' + GLIDE_BYTE_SIZE] = 15;
    incompleteMap[15] = false;
    // Initial digit:
    i = '1';
    do {
      stateMap[i + GLIDE_BYTE_SIZE] = 16;
    }
    while(++i <= '9');
    incompleteMap[16] = false;
    // Negative:
    stateMap['-' + GLIDE_BYTE_SIZE] = 17;
    // Second digit:
    i = '0';
    do {
      stateMap[i + GLIDE_BYTE_SIZE * 16] = 19;
    }
    while(++i <= '9');
    incompleteMap[19] = false;
    // Remaining digits:
    i = '0';
    do {
      stateMap[i + GLIDE_BYTE_SIZE * 19] = 19;
    }
    while(++i <= '9');
    // Negative zero:
    stateMap['0' + GLIDE_BYTE_SIZE * 17] = 18;
    incompleteMap[18] = false;
    // Initial digit from negative:
    i = '1';
    do {
      stateMap[i + GLIDE_BYTE_SIZE * 17] = 19;
    }
    while(++i <= '9');
    // Decimal point:
    stateMap['.' + GLIDE_BYTE_SIZE * 15] = 20;
    stateMap['.' + GLIDE_BYTE_SIZE * 16] = 20;
    stateMap['.' + GLIDE_BYTE_SIZE * 18] = 20;
    stateMap['.' + GLIDE_BYTE_SIZE * 19] = 20;
    // Initial digit from decimal point:
    i = '0';
    do {
      stateMap[i + GLIDE_BYTE_SIZE * 20] = 21;
    }
    while(++i <= '9');
    incompleteMap[21] = false;
    // Remaining digits from decimal point:
    i = '0';
    do {
      stateMap[i + GLIDE_BYTE_SIZE * 21] = 21;
    }
    while(++i <= '9');
    // Exponent:
    stateMap['E' + GLIDE_BYTE_SIZE * 15] = 22;
    stateMap['e' + GLIDE_BYTE_SIZE * 15] = 22;
    stateMap['E' + GLIDE_BYTE_SIZE * 16] = 22;
    stateMap['e' + GLIDE_BYTE_SIZE * 16] = 22;
    stateMap['E' + GLIDE_BYTE_SIZE * 18] = 22;
    stateMap['e' + GLIDE_BYTE_SIZE * 18] = 22;
    stateMap['E' + GLIDE_BYTE_SIZE * 19] = 22;
    stateMap['e' + GLIDE_BYTE_SIZE * 19] = 22;
    stateMap['E' + GLIDE_BYTE_SIZE * 21] = 22;
    stateMap['e' + GLIDE_BYTE_SIZE * 21] = 22;
    // Initial digit from exponent:
    i = '0';
    do {
      stateMap[i + GLIDE_BYTE_SIZE * 22] = 23;
    }
    while(++i <= '9');
    incompleteMap[23] = false;
    // Exponent sign:
    stateMap['+' + GLIDE_BYTE_SIZE * 22] = 24;
    stateMap['-' + GLIDE_BYTE_SIZE * 22] = 24;
    // Remaining digits from exponent:
    i = '0';
    do {
      stateMap[i + GLIDE_BYTE_SIZE * 23] = 23;
    }
    while(++i <= '9');
    // Initial digit from exponent sign:
    i = '0';
    do {
      stateMap[i + GLIDE_BYTE_SIZE * 24] = 23;
    }
    while(++i <= '9');
    /*
      In this initial state, a string is initialized then led to the
      looping state 26. See "copyTransitions" below.
    */
    stateMap['"' + GLIDE_BYTE_SIZE] = 25;
    // End of the string:
    stateMap['"' + GLIDE_BYTE_SIZE * 26] = 27;
    incompleteMap[27] = false;
    // Escape sequence:
    stateMap['\\' + GLIDE_BYTE_SIZE * 26] = 28;
    // Every other ASCII character from codepoint 32 to 127:
    i = 32;
    do {
      if((i != '"') & (i != '\\')) {
        stateMap[i + GLIDE_BYTE_SIZE * 26] = 26;
      }
    }
    while(++i <= 127);
    /*
      Escape-sequence characters in JSON. Note that when an escape character
      is properly matched, it leads to state 26 (see "copyTransitions"
      below). In other words, the string simply continues when the escape
      sequence is valid.
    */
    stateMap['"' + GLIDE_BYTE_SIZE * 28] = 29;
    stateMap['\\' + GLIDE_BYTE_SIZE * 28] = 30;
    stateMap['/' + GLIDE_BYTE_SIZE * 28] = 31;
    stateMap['b' + GLIDE_BYTE_SIZE * 28] = 32;
    stateMap['f' + GLIDE_BYTE_SIZE * 28] = 33;
    stateMap['n' + GLIDE_BYTE_SIZE * 28] = 34;
    stateMap['r' + GLIDE_BYTE_SIZE * 28] = 35;
    stateMap['t' + GLIDE_BYTE_SIZE * 28] = 36;
    /*
      The escape sequence "\u0000", as shown, has 4 hex characters. But
      there is a lot going on in this part of the FSM. Since this is a UTF-8
      parser, these codepoints have to be decoded into UTF-8 sequences. This
      part of the FSM can easily be condensed into 4 or less states that
      contain follow-up if statements. But if statements, while simpler,
      would defeat the purpose of the FSM, and almost certainly compromise
      performance. So, to stay true to this strategy, the states must branch
      out according to the number of bytes of the resulting UTF-8 sequence:

      - The first state has two branches: one solely for '0' and another for
        1-F. The latter branch eliminates the possibility that the sequence
        decodes to less than 3 bytes.
      - If the first character is '0', the following state has 3 branches: one
        solely for '0', one for 1-7, and one for 8-F. At this point, the
        sequence can have any number of bytes. '0' is 1 or 2 bytes, 0-7 is
        always 2 bytes, and 8-F is always 3 bytes.
      - The state after the second '0' character has two branches: one for 0-7
        and another for 8-F. 0-7 is 1 byte, 8-F is 2 bytes.
      - A final note: the "leaf" states all terminate the sequence after the
        fourth hex character. There are 3 such states, and they correspond
        directly to each UTF-8 byte count determined by the aforementioned
        branching.
    */
    stateMap['u' + GLIDE_BYTE_SIZE * 28] = 37;
    // Any hex character (except 37 => 38, see below):
    i = '0';
    do {
      stateMap[i + GLIDE_BYTE_SIZE * 37] = 39;
      stateMap[i + GLIDE_BYTE_SIZE * 39] = 40;
      stateMap[i + GLIDE_BYTE_SIZE * 40] = 45;
      stateMap[i + GLIDE_BYTE_SIZE * 41] = 43;
      stateMap[i + GLIDE_BYTE_SIZE * 43] = 47;
      stateMap[i + GLIDE_BYTE_SIZE * 44] = 46;
      stateMap[i + GLIDE_BYTE_SIZE * 45] = 48;
    }
    while(++i <= '9');
    i = 'A';
    do {
      stateMap[i + GLIDE_BYTE_SIZE * 37] = 39;
      stateMap[i + GLIDE_BYTE_SIZE * 39] = 40;
      stateMap[i + GLIDE_BYTE_SIZE * 40] = 45;
      stateMap[i + GLIDE_BYTE_SIZE * 41] = 43;
      stateMap[i + GLIDE_BYTE_SIZE * 43] = 47;
      stateMap[i + GLIDE_BYTE_SIZE * 44] = 46;
      stateMap[i + GLIDE_BYTE_SIZE * 45] = 48;
    }
    while(++i <= 'F');
    i = 'a';
    do {
      stateMap[i + GLIDE_BYTE_SIZE * 37] = 39;
      stateMap[i + GLIDE_BYTE_SIZE * 39] = 40;
      stateMap[i + GLIDE_BYTE_SIZE * 40] = 45;
      stateMap[i + GLIDE_BYTE_SIZE * 41] = 43;
      stateMap[i + GLIDE_BYTE_SIZE * 43] = 47;
      stateMap[i + GLIDE_BYTE_SIZE * 44] = 46;
      stateMap[i + GLIDE_BYTE_SIZE * 45] = 48;
    }
    while(++i <= 'f');
    // At state 37, only '0' branches to state 38:
    stateMap['0' + GLIDE_BYTE_SIZE * 37] = 38;
    // Only 0-7 (hex):
    i = '0';
    do {
      stateMap[i + GLIDE_BYTE_SIZE * 38] = 41;
      stateMap[i + GLIDE_BYTE_SIZE * 42] = 44;
    }
    while(++i <= '7');
    // At state 38, only '0' branches to state 42:
    stateMap['0' + GLIDE_BYTE_SIZE * 38] = 42;
    // Only 8-F (hex):
    stateMap['8' + GLIDE_BYTE_SIZE * 38] = 40;
    stateMap['8' + GLIDE_BYTE_SIZE * 42] = 43;
    stateMap['9' + GLIDE_BYTE_SIZE * 38] = 40;
    stateMap['9' + GLIDE_BYTE_SIZE * 42] = 43;
    i = 'A';
    do {
      stateMap[i + GLIDE_BYTE_SIZE * 38] = 40;
      stateMap[i + GLIDE_BYTE_SIZE * 42] = 43;
    }
    while(++i <= 'F');
    i = 'a';
    do {
      stateMap[i + GLIDE_BYTE_SIZE * 38] = 40;
      stateMap[i + GLIDE_BYTE_SIZE * 42] = 43;
    }
    while(++i <= 'f');
    /*
      Finally, make sure the JSON string is valid UTF-8 by mapping the
      sequences in the remainder of the unicode range. We already took care
      of control characters and one-byte sequences above. These remaining
      branches are for 2 to 4 bytes.

      Note that state 52 terminates all valid byte sequences then leads to
      state 26. See "copyTransitions" below.

      Grammar for UTF-8 from RFC 3629:

      UTF8-octets = *( UTF8-char )
      UTF8-char   = UTF8-1 / UTF8-2 / UTF8-3 / UTF8-4
      UTF8-1      = %x00-7F
      UTF8-2      = %xC2-DF UTF8-tail
      UTF8-3      = %xE0 %xA0-BF UTF8-tail /
                    %xE1-EC 2( UTF8-tail ) /
                    %xED %x80-9F UTF8-tail /
                    %xEE-EF 2( UTF8-tail )
      UTF8-4      = %xF0 %x90-BF 2( UTF8-tail ) /
                    %xF1-F3 3( UTF8-tail ) /
                    %xF4 %x80-8F 2( UTF8-tail )
      UTF8-tail   = %x80-BF
    */
    // UTF8-2      = %xC2-DF UTF8-tail
    i = 194;
    do {
      stateMap[i + GLIDE_BYTE_SIZE * 26] = 49;
    }
    while(++i <= 223);
    i = 128;
    do {
      stateMap[i + GLIDE_BYTE_SIZE * 49] = 52;
    }
    while(++i <= 191);
    /*
      UTF8-3      = %xE0 %xA0-BF UTF8-tail /
                    %xE1-EC 2( UTF8-tail ) /
                    %xED %x80-9F UTF8-tail /
                    %xEE-EF 2( UTF8-tail )
    */
    stateMap[224 + GLIDE_BYTE_SIZE * 26] = 53;
    i = 160;
    do {
      stateMap[i + GLIDE_BYTE_SIZE * 53] = 49;
    }
    while(++i <= 191);
    //
    i = 225;
    do {
      stateMap[i + GLIDE_BYTE_SIZE * 26] = 50;
    }
    while(++i <= 236);
    stateMap[238 + GLIDE_BYTE_SIZE * 26] = 50;
    stateMap[239 + GLIDE_BYTE_SIZE * 26] = 50;
    i = 128;
    do {
      stateMap[i + GLIDE_BYTE_SIZE * 50] = 49;
    }
    while(++i <= 191);
    //
    stateMap[237 + GLIDE_BYTE_SIZE * 26] = 54;
    i = 128;
    do {
      stateMap[i + GLIDE_BYTE_SIZE * 54] = 49;
    }
    while(++i <= 159);
    /*
      UTF8-4      = %xF0 %x90-BF 2( UTF8-tail ) /
                    %xF1-F3 3( UTF8-tail ) /
                    %xF4 %x80-8F 2( UTF8-tail )
    */
    stateMap[240 + GLIDE_BYTE_SIZE * 26] = 55;
    i = 144;
    do {
      stateMap[i + GLIDE_BYTE_SIZE * 55] = 50;
    }
    while(++i <= 191);
    //
    stateMap[241 + GLIDE_BYTE_SIZE * 26] = 51;
    stateMap[242 + GLIDE_BYTE_SIZE * 26] = 51;
    stateMap[243 + GLIDE_BYTE_SIZE * 26] = 51;
    i = 128;
    do {
      stateMap[i + GLIDE_BYTE_SIZE * 51] = 50;
    }
    while(++i <= 191);
    //
    stateMap[244 + GLIDE_BYTE_SIZE * 26] = 56;
    i = 128;
    do {
      stateMap[i + GLIDE_BYTE_SIZE * 56] = 50;
    }
    while(++i <= 143);
    /*
      As noted above, the following states for parsing JSON strings need to
      behave exactly the same as state 26. They merely continue string
      parsing after a special case is handled successfully.
    */
    Parser::copyTransitions(26, 25);
    Parser::copyTransitions(26, 29);
    Parser::copyTransitions(26, 30);
    Parser::copyTransitions(26, 31);
    Parser::copyTransitions(26, 32);
    Parser::copyTransitions(26, 33);
    Parser::copyTransitions(26, 34);
    Parser::copyTransitions(26, 35);
    Parser::copyTransitions(26, 36);
    Parser::copyTransitions(26, 46);
    Parser::copyTransitions(26, 47);
    Parser::copyTransitions(26, 48);
    Parser::copyTransitions(26, 52);
    /*
      The following changes cover all whitespace scenarios:
      - Making whitespace at state 1 loop back to state 1 to support
        whitespace preceding all JSON elements, including most nested ones.
      - A new state 66 to capture whitespace before a string acting as an
        object key.
      - A new state 67 to capture whitespace at the end of valid strings. The
        separate state is necessary to provide for a colon when the string is
        acting as an object key.
      - A new state 68 to capture whitespace at the end of all valid JSON
        elements except strings.

      State 1 is defined right here since it must precede any calls to
      "copyTransitions". The rest of the setup is mixed in with the
      container definitions to make things a little easier to follow.
    */
    Parser::setWhitespace(1, 1);
    /*
      First, it's important to note that there is a lot of logic nested in
      these container states. This is because there exists no FSM that
      perfectly satisfies the definition of a JSON array or
      object. Recursive patterns are impossible with FSMs. So, we define a
      state machine that matches JSON arrays and objects, and use said logic
      to enforce the recursive patterns.

      The implementation below uses a stack for containers and a placeholder
      for object keys. States 57, 58, 61, and 62 are hopefully
      self-explanatory. States 59 and 63 are set as destinations for every
      final state defined above since they terminate the two
      containers. These two states can be reached regardless of whether or
      not the appropriate container has been initialized. This is a symptom
      of the whole recursive pattern dilemma detailed above. So checks are
      performed as needed.

      State 60 is for the comma, which is a separator for both JSON arrays
      and objects. This is the same situation as states 59 and 63, but both
      a JSON array and a JSON object must be handled here. In the case of a
      JSON object, the state is edited to 64 to restrict the next element to
      a string for the key. To enforce alternating keys and values, the
      object key's placeholder must be null at this point.

      State 65 for the colon terminates the object key and begins the
      corresponding JSON element. The object key placeholder is set to null
      to indicate that a new key can be parsed at the next comma.
    */
    stateMap['[' + GLIDE_BYTE_SIZE] = 57;
    incompleteMap[58] = false;
    incompleteMap[59] = false;
    stateMap['{' + GLIDE_BYTE_SIZE] = 61;
    stateMap['}' + GLIDE_BYTE_SIZE * 61] = 62;
    incompleteMap[62] = false;
    incompleteMap[63] = false;
    stateMap['"' + GLIDE_BYTE_SIZE * 61] = 25;
    stateMap['"' + GLIDE_BYTE_SIZE * 64] = 25;
    stateMap['"' + GLIDE_BYTE_SIZE * 66] = 25;
    Parser::setWhitespace(61, 66);
    Parser::setWhitespace(64, 66);
    Parser::setWhitespace(66, 66);
    stateMap[':' + GLIDE_BYTE_SIZE * 27] = 65;
    stateMap[':' + GLIDE_BYTE_SIZE * 67] = 65;
    Parser::setWhitespace(27, 67);
    Parser::setWhitespace(67, 67);
    incompleteMap[67] = false;
    Parser::copyTransitions(1, 57);
    stateMap[']' + GLIDE_BYTE_SIZE * 57] = 58;
    Parser::copyTransitions(1, 60);
    Parser::copyTransitions(1, 65);
    incompleteMap[68] = false;
    i = 0;
    do {
      if(!incompleteMap[i]) {
        stateMap[']' + GLIDE_BYTE_SIZE * i] = 59;
        stateMap[',' + GLIDE_BYTE_SIZE * i] = 60;
        stateMap['}' + GLIDE_BYTE_SIZE * i] = 63;
        if((i != 27) & (i != 67)) {
          Parser::setWhitespace(i, 68);
        }
      }
    }
    while(++i < GLIDE_JSON_PARSER_STATES);
  }

  Parser::Parser() {
    throw GlideError("GlideJsonScheme::Parser::Parser(): No default constructor!");
  }

  Parser::Parser(const Parser &input) {
    (void)input;
    throw GlideError("GlideJsonScheme::Parser::Parser(const Parser &input): No copy constructor!");
  }

  Parser::~Parser() {
  }

  Parser & Parser::operator=(const Parser &input) {
    (void)input;
    throw GlideError("GlideJsonScheme::Parser::operator=(const Parser &input): No assignment operator!");
    return *this;
  }

  /*
    See the source file for comments:
  */
  #define GLIDE_JSON_PART_STDSTRING
  #include "Parser.inc"
  #undef GLIDE_JSON_PART_STDSTRING

  #define GLIDE_JSON_PART_CSTRING
  #include "Parser.inc"
  #undef GLIDE_JSON_PART_CSTRING

  // ========================================

  ParserInitializer::ParserInitializer() {
    Parser::initialize();
  }

  ParserInitializer::ParserInitializer(const ParserInitializer &input) {
    (void)input;
    throw GlideError("GlideJsonScheme::ParserInitializer::ParserInitializer(const ParserInitializer &input): No copy constructor!");
  }

  ParserInitializer::~ParserInitializer() {
  }

  ParserInitializer & ParserInitializer::operator=(const ParserInitializer &input) {
    (void)input;
    throw GlideError("GlideJsonScheme::ParserInitializer::operator=(const ParserInitializer &input): No assignment operator!");
    return *this;
  }

}
