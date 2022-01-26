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

#ifdef GLIDE_JSON_PART_STDSTRING
GlideJson Parser::parse(const std::string &input) {
#endif
#ifdef GLIDE_JSON_PART_CSTRING
GlideJson Parser::parse(const char *cInput, size_t size) {
#endif
  static const ParserInitializer parserInitializer;
  static const GlideJson gNull;
  //
  #ifdef GLIDE_JSON_PART_STDSTRING
  size_t size(input.size());
  const char *cInput(input.data());
  #endif
  /*
    If the input string is empty, the initial state is the failure state 0
    because nothing is represented by an empty string in JSON. Otherwise
    we start at state 1.
  */
  size_t i(0);
  unsigned char cChar;
  unsigned short unicode;
  unsigned char utf8;
  unsigned char state(size > 0);
  size_t finalIndex(size);
  GlideJson output;
  GlideJson *cOutput(&output);
  size_t decodedSize;
  size_t decodedCapacity;
  std::string *decoded(NULL);
  GlideJson objectKey;
  std::vector<GlideJson *> containers(1, &objectKey);
  std::vector<GlideJson> *innerArray;
  while(i < size) {
    cChar = cInput[i];
    state = stateMap[cChar + GLIDE_BYTE_SIZE * state];
    switch(state) {
      case 0:
        finalIndex = i;
        i = size;
        break;
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
        /*
          State 5 is the success state for "null". "cOutput" always points to a
          null GlideJson at this point, so no action is taken.
        */
      case 6:
      case 7:
      case 8:
      case 9:
        break;
      case 10:
        *cOutput = GlideJson::Boolean;
        *((Boolean *)(cOutput->content)) = false;
        break;
      case 11:
      case 12:
      case 13:
        break;
      case 14:
        *cOutput = GlideJson::Boolean;
        *((Boolean *)(cOutput->content)) = true;
        break;
      case 15:
      case 16:
      case 17:
        *cOutput = GlideJson::Number;
        if(decoded) {
          decoded->resize(decodedSize);
        }
        decoded = &(((Number *)(cOutput->content))->number);
        *decoded = cChar;
        decodedSize = 1;
        decodedCapacity = GlideString::initialCapacity;
        decoded->resize(decodedCapacity);
        break;
      case 18:
      case 19:
      case 20:
      case 21:
      case 22:
      case 23:
      case 24:
        GlideString::append(cChar, decodedSize, decodedCapacity, *decoded);
        break;
      case 25:
        *cOutput = GlideJson::String;
        if(decoded) {
          decoded->resize(decodedSize);
        }
        decoded = &(((String *)(cOutput->content))->string);
        decodedSize = 0;
        decodedCapacity = GlideString::initialCapacity;
        decoded->resize(decodedCapacity);
        break;
      case 26:
        GlideString::append(cChar, decodedSize, decodedCapacity, *decoded);
        break;
      case 27:
      case 28:
        break;
      case 29:
        GlideString::append('"', decodedSize, decodedCapacity, *decoded);
        break;
      case 30:
        GlideString::append('\\', decodedSize, decodedCapacity, *decoded);
        break;
      case 31:
        GlideString::append('/', decodedSize, decodedCapacity, *decoded);
        break;
      case 32:
        GlideString::append('\b', decodedSize, decodedCapacity, *decoded);
        break;
      case 33:
        GlideString::append('\f', decodedSize, decodedCapacity, *decoded);
        break;
      case 34:
        GlideString::append('\n', decodedSize, decodedCapacity, *decoded);
        break;
      case 35:
        GlideString::append('\r', decodedSize, decodedCapacity, *decoded);
        break;
      case 36:
        GlideString::append('\t', decodedSize, decodedCapacity, *decoded);
        break;
      case 37:
        unicode = 0;
        break;
      case 38:
      case 39:
      case 40:
      case 41:
      case 42:
      case 43:
      case 44:
      case 45:
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
        unicode |= hexMap[cChar];
        #pragma GCC diagnostic pop
        unicode <<= 4;
        break;
      case 46:
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
        unicode |= hexMap[cChar];
        #pragma GCC diagnostic pop
        GlideString::append(unicode, decodedSize, decodedCapacity, *decoded);
        break;
      case 47:
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
        unicode |= hexMap[cChar];
        #pragma GCC diagnostic pop
        utf8 = 0xC0 | (unicode >> 6);
        GlideString::append(utf8, decodedSize, decodedCapacity, *decoded);
        utf8 = 0x80 | (unicode & 0x3F);
        GlideString::append(utf8, decodedSize, decodedCapacity, *decoded);
        break;
      case 48:
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
        unicode |= hexMap[cChar];
        #pragma GCC diagnostic pop
        utf8 = 0xE0 | (unicode >> 12);
        GlideString::append(utf8, decodedSize, decodedCapacity, *decoded);
        unicode &= 0x0FFF;
        utf8 = 0x80 | (unicode >> 6);
        GlideString::append(utf8, decodedSize, decodedCapacity, *decoded);
        utf8 = 0x80 | (unicode & 0x3F);
        GlideString::append(utf8, decodedSize, decodedCapacity, *decoded);
        break;
      case 49:
      case 50:
      case 51:
      case 52:
      case 53:
      case 54:
      case 55:
      case 56:
        GlideString::append(cChar, decodedSize, decodedCapacity, *decoded);
        break;
      case 57:
        *cOutput = GlideJson::Array;
        containers.push_back(cOutput);
        innerArray = &(((Array *)(cOutput->content))->array);
        innerArray->push_back(gNull);
        cOutput = &(innerArray->back());
        break;
      case 58:
        cOutput = containers.back();
        containers.pop_back();
        ((Array *)(cOutput->content))->array.pop_back();
        break;
      case 59:
        if(decoded) {
          decoded->resize(decodedSize);
          decoded = NULL;
        }
        cOutput = containers.back();
        containers.pop_back();
        if(cOutput->getType() != GlideJson::Array) {
          state = 0;
          finalIndex = i;
          i = size;
        }
        break;
      case 60:
        if(decoded) {
          decoded->resize(decodedSize);
          decoded = NULL;
        }
        cOutput = containers.back();
        if(cOutput->getType() == GlideJson::Array) {
          innerArray = &(((Array *)(cOutput->content))->array);
          innerArray->push_back(gNull);
          cOutput = &(innerArray->back());
        }
        else if(
          (cOutput->getType() == GlideJson::Object)
          & (objectKey.getType() == GlideJson::Null)
        ) {
          state = 64;
          cOutput = &objectKey;
        }
        else {
          state = 0;
          finalIndex = i;
          i = size;
        }
        break;
      case 61:
        *cOutput = GlideJson::Object;
        containers.push_back(cOutput);
        cOutput = &objectKey;
        break;
      case 62:
        containers.pop_back();
        break;
      case 63:
        if(decoded) {
          decoded->resize(decodedSize);
          decoded = NULL;
        }
        cOutput = containers.back();
        containers.pop_back();
        if(cOutput->getType() != GlideJson::Object) {
          state = 0;
          finalIndex = i;
          i = size;
        }
        break;
      case 64:
        break;
      case 65:
        if(decoded) {
          decoded->resize(decodedSize);
          decoded = NULL;
        }
        if(objectKey.getType() == GlideJson::String) {
          cOutput = containers.back();
          cOutput = &(((Object *)(cOutput->content))->object[std::move(((String *)(objectKey.content))->string)]);
          objectKey = GlideJson::Null;
        }
        else {
          state = 0;
          finalIndex = i;
          i = size;
        }
        break;
      case 66:
      case 67:
      case 68:
        break;
      default:
        abort();
    }
    ++i;
  }
  if(decoded) {
    decoded->resize(decodedSize);
  }
  if(incompleteMap[state] | (containers.size() != 1)) {
    output = GlideJson::Error;
    ((GlideJsonScheme::Error *)(output.content))->error = "Parsing failed at index " + std::to_string(finalIndex) + "!";
  }
  return output;
}
