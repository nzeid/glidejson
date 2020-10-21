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

/*
  As detailed in GlideJson.cpp, there are no failure states. State 0 is
  the initial state. This function always produces valid UTF-8 JSON
  strings. Any invalid UTF-8 sequences detected in the input are simply
  gutted. It's the user's responsibility to maintain valid UTF-8 within
  the native string if they wish to convert it sanely.

  This can cause some confusion for those unwittingly encoding then
  decoding the same invalid UTF-8 string. All the invalid bytes will be
  removed in the resulting JSON string. E.g., the C string "a\xc0\xffz"
  encodes to the JSON string "az" which then decodes to the C string
  "az". Some thought was given to throwing an error when invalid UTF-8
  is given. But a snappier, fault-tolerant conversion was preferred.

  One way to ensure proper transmission of arbitrary/raw binary is to
  use a separate encoder to convert it into UTF-8 first *then* encode it
  into JSON with this class. When the JSON is parsed, decode the UTF-8
  result and you will get what you started with. A common solution is to
  use a Latin-1 to UTF-8 encoder, since the width of a Latin-1 codepoint
  is exactly one byte. But there is no reason you can't use any other
  encoder as long as the order of conversion is preserved.

  The *most* common solution is to use a Base64 encoder. This will
  deflate the binary significantly but the resulting string is always
  valid UTF-8 and ASCII.
*/
#ifdef GLIDE_JSON_PART_STDSTRING
std::string Encoder::encode(const std::string &input) {
#endif
#ifdef GLIDE_JSON_PART_CSTRING
std::string Encoder::encode(const char * const &cInput, const size_t &size) {
#endif
  static const EncoderInitializer &encoderInitializer(EncoderInitializer::initializer());
  (void)encoderInitializer;
  //
  #ifdef GLIDE_JSON_PART_STDSTRING
  size_t size(input.size());
  const char *cInput(input.data());
  #endif
  //
  size_t i(0);
  unsigned char cChar;
  unsigned char state(0);
  std::string output(1, '"');
  size_t outputSize(1);
  size_t outputCapacity;
  GlideString::initialize(size, outputCapacity, output);
  unsigned char utf8_1, utf8_2, utf8_3;
  while(i < size) {
    cChar = cInput[i];
    state = stateMap[cChar + GLIDE_BYTE_SIZE * state];
    switch(state) {
      case 0:
        GlideString::append(cChar, outputSize, outputCapacity, output);
        break;
      case 1:
        GlideString::append('\\', outputSize, outputCapacity, output);
        GlideString::append('u', outputSize, outputCapacity, output);
        GlideString::append('0', outputSize, outputCapacity, output);
        GlideString::append('0', outputSize, outputCapacity, output);
        GlideString::append(hexMap[cChar >> 4], outputSize, outputCapacity, output);
        GlideString::append(hexMap[cChar & 15], outputSize, outputCapacity, output);
        break;
      case 2:
        GlideString::append('\\', outputSize, outputCapacity, output);
        GlideString::append('"', outputSize, outputCapacity, output);
        break;
      case 3:
        GlideString::append('\\', outputSize, outputCapacity, output);
        GlideString::append('\\', outputSize, outputCapacity, output);
        break;
      case 4:
        GlideString::append('\\', outputSize, outputCapacity, output);
        GlideString::append('b', outputSize, outputCapacity, output);
        break;
      case 5:
        GlideString::append('\\', outputSize, outputCapacity, output);
        GlideString::append('f', outputSize, outputCapacity, output);
        break;
      case 6:
        GlideString::append('\\', outputSize, outputCapacity, output);
        GlideString::append('n', outputSize, outputCapacity, output);
        break;
      case 7:
        GlideString::append('\\', outputSize, outputCapacity, output);
        GlideString::append('r', outputSize, outputCapacity, output);
        break;
      case 8:
        GlideString::append('\\', outputSize, outputCapacity, output);
        GlideString::append('t', outputSize, outputCapacity, output);
        break;
      case 9:
        break;
      case 10:
        utf8_1 = cChar;
        break;
      case 11:
        GlideString::append(utf8_1, outputSize, outputCapacity, output);
        GlideString::append(cChar, outputSize, outputCapacity, output);
        break;
      case 12:
      case 13:
      case 14:
        utf8_1 = cChar;
        break;
      case 15:
        utf8_2 = cChar;
        break;
      case 16:
        GlideString::append(utf8_1, outputSize, outputCapacity, output);
        GlideString::append(utf8_2, outputSize, outputCapacity, output);
        GlideString::append(cChar, outputSize, outputCapacity, output);
        break;
      case 17:
      case 18:
      case 19:
        utf8_1 = cChar;
        break;
      case 20:
        utf8_2 = cChar;
        break;
      case 21:
        utf8_3 = cChar;
        break;
      case 22:
        GlideString::append(utf8_1, outputSize, outputCapacity, output);
        GlideString::append(utf8_2, outputSize, outputCapacity, output);
        GlideString::append(utf8_3, outputSize, outputCapacity, output);
        GlideString::append(cChar, outputSize, outputCapacity, output);
        break;
      default:
        abort();
    }
    ++i;
  }
  GlideString::append('"', outputSize, outputCapacity, output);
  output.resize(outputSize);
  return output;
}

/*
  The Base64 codec is not particularly complex. Only thing worth noting
  here is the decoder:
  - It never fails. It's up to the user to provide valid Base64.
  - Invalid Base64 will produce an accurate result only where valid
    sequences are processed.
  - It will fill in zero bits where invalid characters are processed (see
    "b64dMap").
  - An invalid number of trailing characters will cause said characters to
    be omitted.
  - All whitespace is invalid. Remove any whitespace before decoding.
*/
#ifdef GLIDE_JSON_PART_STDSTRING
std::string Encoder::base64Encode(const std::string &input) {
#endif
#ifdef GLIDE_JSON_PART_CSTRING
std::string Encoder::base64Encode(const char * const &cInput, const size_t &length) {
#endif
  static const EncoderInitializer &encoderInitializer(EncoderInitializer::initializer());
  (void)encoderInitializer;
  //
  #ifdef GLIDE_JSON_PART_STDSTRING
  size_t length(input.size());
  const char *cInput(input.data());
  #endif
  //
  size_t i(0);
  std::string output;
  size_t outputSize(0);
  output.resize(length * 4 / 3 + 3);
  unsigned char first, second;
  while(i + 3 <= length) {
    first = cInput[i];
    second = cInput[i + 1];
    output[outputSize++] = b64eMap[first >> 2];
    output[outputSize++] = b64eMap[((first & 3) << 4) | (second >> 4)];
    first = cInput[i + 2];
    output[outputSize++] = b64eMap[((second & 15) << 2) | (first >> 6)];
    output[outputSize++] = b64eMap[first & 63];
    i += 3;
  }
  first = length - i;
  if(first == 1) {
    first = cInput[length - 1];
    output[outputSize++] = b64eMap[first >> 2];
    output[outputSize++] = b64eMap[(first & 3) << 4];
    output[outputSize++] = '=';
    output[outputSize++] = '=';
  }
  else if(first == 2) {
    first = cInput[length - 2];
    second = cInput[length - 1];
    output[outputSize++] = b64eMap[first >> 2];
    output[outputSize++] = b64eMap[((first & 3) << 4) | (second >> 4)];
    output[outputSize++] = b64eMap[(second & 15) << 2];
    output[outputSize++] = '=';
  }
  output.resize(outputSize);
  return output;
}

#ifdef GLIDE_JSON_PART_STDSTRING
std::string Encoder::base64Decode(const std::string &input) {
#endif
#ifdef GLIDE_JSON_PART_CSTRING
std::string Encoder::base64Decode(const char * const &cInput, const size_t &length) {
#endif
  static const EncoderInitializer &encoderInitializer(EncoderInitializer::initializer());
  (void)encoderInitializer;
  //
  #ifdef GLIDE_JSON_PART_STDSTRING
  size_t length(input.size());
  const char *cInput(input.data());
  #endif
  //
  size_t i(0);
  std::string output;
  size_t outputSize(0);
  output.resize(length * 3 / 4);
  unsigned char first, second;
  while(i + 4 <= length) {
    first = b64dMap[(unsigned char)(cInput[i])];
    second = b64dMap[(unsigned char)(cInput[i + 1])];
    output[outputSize++] = (first << 2) | (second >> 4);
    first = b64dMap[(unsigned char)(cInput[i + 2])];
    output[outputSize++] = (second << 4) | (first >> 2);
    second = b64dMap[(unsigned char)(cInput[i + 3])];
    output[outputSize++] = (first << 6) | second;
    i += 4;
  }
  i = length - i;
  if(i == 2) {
    first = b64dMap[(unsigned char)(cInput[length - 2])];
    second = b64dMap[(unsigned char)(cInput[length - 1])];
    output[outputSize++] = (first << 2) | (second >> 4);
  }
  else if(i == 3) {
    first = b64dMap[(unsigned char)(cInput[length - 3])];
    second = b64dMap[(unsigned char)(cInput[length - 2])];
    output[outputSize++] = (first << 2) | (second >> 4);
    first = b64dMap[(unsigned char)(cInput[length - 1])];
    output[outputSize++] = (second << 4) | (first >> 2);
  }
  if(cInput[length - 1] == '=') {
    --outputSize;
    if(cInput[length - 2] == '=') {
      --outputSize;
    }
  }
  output.resize(outputSize);
  return output;
}
