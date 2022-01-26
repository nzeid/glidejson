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

#include "GlideJsonTest.hpp"

std::vector<std::string> GlideJsonTest::errors;

std::string GlideJsonTest::hashMapToString(const GlideHashMap<int> &input) {
  std::string output;
  auto i(input.begin());
  auto iEnd(input.end());
  while(i != iEnd) {
    output += "(";
    output += i.key();
    output += ",";
    output += std::to_string(i.value());
    output += ")";
    i.next();
  }
  return output;
}

std::string GlideJsonTest::vectorToString(const std::vector< std::pair<std::string, int> > &input) {
  std::string output;
  int i(0);
  int l(input.size());
  while(i < l) {
    output += "(";
    output += input[i].first;
    output += ",";
    output += std::to_string(input[i].second);
    output += ")";
    ++i;
  }
  return output;
}

int GlideJsonTest::vectorGet(const std::vector< std::pair<std::string, int> > &input, const std::string &key) {
  int i(0);
  int l(input.size());
  while(i < l) {
    if(input[i].first == key) {
      return i;
    }
    ++i;
  }
  return -1;
}

void GlideJsonTest::testGlideHashMap() {
  auto start(std::chrono::steady_clock::now());
  GlideHashMap<int> subject;
  std::vector< std::pair<std::string, int> > checker;
  int mode;
  std::string key;
  int value;
  int vIndex;
  int i(10000);
  do {
    mode = std::rand() % 4;
    switch(mode) {
      case 0:
        key = std::to_string(std::rand() % 500);
        value = std::rand();
        vIndex = GlideJsonTest::vectorGet(checker, key);
        if(vIndex >= 0) {
          checker[vIndex].second = value;
        }
        else {
          checker.push_back(std::pair<std::string, int>(key, value));
        }
        if(std::rand() & 1) {
          subject[key] = value;
        }
        else {
          subject[std::move(key)] = value;
        }
        break;
      case 1:
        if(subject.size()) {
          vIndex = 10;
          do {
            key = std::to_string(std::rand() % 500);
          }
          while(!subject.count(key) && --vIndex);
          if(vIndex) {
            subject.erase(key);
            vIndex = GlideJsonTest::vectorGet(checker, key);
            if(vIndex >= 0) {
              checker.erase(checker.begin() + vIndex);
            }
            else {
              GlideJsonTest::errors.push_back("Key \"" + key + "\" is missing from the checker!");
            }
          }
        }
        break;
      case 2:
        if(GlideJsonTest::hashMapToString(subject) != GlideJsonTest::vectorToString(checker)) {
          GlideJsonTest::errors.push_back("The subject and the checker are not the same!");
        }
        break;
      case 3:
        if(subject.size()) {
          vIndex = 10;
          do {
            key = std::to_string(std::rand() % 500);
          }
          while(!subject.count(key) && --vIndex);
          if(vIndex) {
            vIndex = GlideJsonTest::vectorGet(checker, key);
            if(vIndex >= 0) {
              if(checker[vIndex].second != subject[key]) {
                GlideJsonTest::errors.push_back("The value at key \"" + key + "\" doesn't match!");
              }
            }
            else {
              GlideJsonTest::errors.push_back("Key \"" + key + "\" is missing from the checker!");
            }
          }
        }
        break;
      default:
        abort();
    }
  }
  while(--i > 0);
  std::cout << "Indexed hash map test: " << (std::chrono::steady_clock::now() - start).count() << "\n";
  // Create a map for the checker:
  std::map<std::string, int> checkerMap;
  size_t j(0);
  size_t l(checker.size());
  while(j < l) {
    checkerMap[checker[j].first] = checker[j].second;
    ++j;
  }
  // Sort the checker and compare it:
  checker.clear();
  auto k(checkerMap.begin());
  auto kEnd(checkerMap.end());
  while(k != kEnd) {
    checker.push_back(std::pair<std::string, int>(k->first, k->second));
    ++k;
  }
  subject.sort();
  if(GlideJsonTest::hashMapToString(subject) != GlideJsonTest::vectorToString(checker)) {
    GlideJsonTest::errors.push_back("Sorting the subject caused it not to be the same as the checker!");
  }
  // Reverse-sort the checker and compare it:
  checker.clear();
  auto m(checkerMap.rbegin());
  auto mEnd(checkerMap.rend());
  while(m != mEnd) {
    checker.push_back(std::pair<std::string, int>(m->first, m->second));
    ++m;
  }
  subject.rsort();
  if(GlideJsonTest::hashMapToString(subject) != GlideJsonTest::vectorToString(checker)) {
    GlideJsonTest::errors.push_back("Sorting the subject caused it not to be the same as the checker!");
  }
}

void GlideJsonTest::escapeByte(const unsigned char &input, std::string &output) {
  if(input < 32 || input == '"' || input == '\\' || input > 127) {
    switch(input) {
      case '"':
      case '\\':
        output.push_back('\\');
        output.push_back(input);
        break;
      case '\b':
        output.push_back('\\');
        output.push_back('b');
        break;
      case '\f':
        output.push_back('\\');
        output.push_back('f');
        break;
      case '\n':
        output.push_back('\\');
        output.push_back('n');
        break;
      case '\r':
        output.push_back('\\');
        output.push_back('r');
        break;
      case '\t':
        output.push_back('\\');
        output.push_back('t');
        break;
      default:
        output.append("\\u00");
        output.push_back(GlideJson::getHex(input >> 4));
        output.push_back(GlideJson::getHex(input & 15));
    }
  }
  else {
    output.push_back(input);
  }
}

void GlideJsonTest::generateJson(const int &mode, std::string * const &output, std::string * const &outputWithWs) {
  switch(mode) {
    case 0:
      *output = "null";
      if(outputWithWs) {
        *outputWithWs = *output;
      }
      break;
    case 1:
      *output = "false";
      if(outputWithWs) {
        *outputWithWs = *output;
      }
      break;
    case 2:
      *output = "true";
      if(outputWithWs) {
        *outputWithWs = *output;
      }
      break;
    case 3:
      output->clear();
      if(std::rand() & 1) {
        *output += "-";
      }
      *output += std::to_string(std::rand() % 1000);
      if(std::rand() & 1) {
        *output += ".";
        if(std::rand() & 1) {
          *output += "0";
        }
        *output += std::to_string(std::rand() % 1000);
      }
      if(std::rand() & 1) {
        *output += ((std::rand() & 1) ? "E" : "e");
        if(std::rand() & 1) {
          *output += ((std::rand() & 1) ? "+" : "-");
        }
        if(std::rand() & 1) {
          *output += "0";
        }
        *output += std::to_string(std::rand() % 1000);
      }
      if(outputWithWs) {
        *outputWithWs = *output;
      }
      break;
    case 4:
      *output = '"';
      {
        unsigned char cChar;
        size_t length(std::rand() % 250);
        while(length) {
          cChar = std::rand() % 248;
          if(cChar >= 128 && cChar <= 193) {
          }
          else if(cChar >= 194 && cChar <= 223) {
            output->push_back(cChar);
            output->push_back(128 + (std::rand() % 64));
          }
          else if(cChar == 224) {
            output->push_back(cChar);
            output->push_back(160 + (std::rand() % 32));
            output->push_back(128 + (std::rand() % 64));
          }
          else if((cChar >= 225 && cChar <= 236) || (cChar >= 238 && cChar <= 239)) {
            output->push_back(cChar);
            output->push_back(128 + (std::rand() % 64));
            output->push_back(128 + (std::rand() % 64));
          }
          else if(cChar == 237) {
            output->push_back(cChar);
            output->push_back(128 + (std::rand() % 32));
            output->push_back(128 + (std::rand() % 64));
          }
          else if(cChar == 240) {
            output->push_back(cChar);
            output->push_back(144 + (std::rand() % 48));
            output->push_back(128 + (std::rand() % 64));
            output->push_back(128 + (std::rand() % 64));
          }
          else if(cChar >= 241 && cChar <= 243) {
            output->push_back(cChar);
            output->push_back(128 + (std::rand() % 64));
            output->push_back(128 + (std::rand() % 64));
            output->push_back(128 + (std::rand() % 64));
          }
          else if(cChar == 244) {
            output->push_back(cChar);
            output->push_back(128 + (std::rand() % 16));
            output->push_back(128 + (std::rand() % 64));
            output->push_back(128 + (std::rand() % 64));
          }
          else if(cChar >= 245 && cChar <= 247) {
          }
          else {
            GlideJsonTest::escapeByte(cChar, *output);
          }
          --length;
        }
      }
      output->push_back('"');
      if(outputWithWs) {
        *outputWithWs = *output;
      }
      break;
    case 5:
      *output = '[';
      if(outputWithWs) {
        *outputWithWs = '[';
      }
      {
        size_t length((std::rand() & 1) * (1 + std::rand() % 10));
        if(length) {
          do {
            std::string nested;
            if(outputWithWs) {
              std::string nestedWithWs;
              GlideJsonTest::generateJson(std::rand() % 7, &nested, &nestedWithWs);
              *outputWithWs += nestedWithWs;
              *outputWithWs += ',';
            }
            else {
              GlideJsonTest::generateJson(std::rand() % 7, &nested, NULL);
            }
            *output += nested;
            *output += ',';
          }
          while(--length);
          (*output)[output->size() - 1] = ']';
          if(outputWithWs) {
            (*outputWithWs)[outputWithWs->size() - 1] = ']';
          }
        }
        else {
          *output += ']';
          if(outputWithWs) {
            *outputWithWs += ']';
          }
        }
      }
      break;
    case 6:
      *output = '{';
      if(outputWithWs) {
        *outputWithWs = '{';
      }
      {
        std::string cKey;
        std::map<std::string, bool> knownKeys;
        size_t length((std::rand() & 1) * (1 + std::rand() % 10));
        if(length) {
          do {
            if(outputWithWs) {
              std::string cKeyWithWs;
              do {
                GlideJsonTest::generateJson(4, &cKey, &cKeyWithWs);
              }
              while(knownKeys.count(cKey));
              *output += cKey;
              *outputWithWs += cKeyWithWs;
              knownKeys[std::move(cKey)] = true;
              *output += ':';
              *outputWithWs += ':';
              GlideJsonTest::generateJson(std::rand() % 7, &cKey, &cKeyWithWs);
              *output += cKey;
              *output += ',';
              *outputWithWs += cKeyWithWs;
              *outputWithWs += ',';
            }
            else {
              do {
                GlideJsonTest::generateJson(4, &cKey, NULL);
              }
              while(knownKeys.count(cKey));
              *output += cKey;
              knownKeys[std::move(cKey)] = true;
              *output += ':';
              GlideJsonTest::generateJson(std::rand() % 7, &cKey, NULL);
              *output += cKey;
              *output += ',';
            }
          }
          while(--length);
          (*output)[output->size() - 1] = '}';
          if(outputWithWs) {
            (*outputWithWs)[outputWithWs->size() - 1] = '}';
          }
        }
        else {
          *output += '}';
          if(outputWithWs) {
            *outputWithWs += '}';
          }
        }
      }
      break;
    default:
      abort();
  }
  if(outputWithWs) {
    size_t length(std::rand() % 6);
    if(length) {
      unsigned char whitespace;
      do {
        whitespace = std::rand() % 8;
        switch(whitespace) {
          case 0:
            *outputWithWs += '\t';
            break;
          case 1:
            outputWithWs->insert(0, 1, '\t');
            break;
          case 2:
            *outputWithWs += '\n';
            break;
          case 3:
            outputWithWs->insert(0, 1, '\n');
            break;
          case 4:
            *outputWithWs += '\r';
            break;
          case 5:
            outputWithWs->insert(0, 1, '\r');
            break;
          case 6:
            *outputWithWs += ' ';
            break;
          case 7:
            outputWithWs->insert(0, 1, ' ');
            break;
          default:
            abort();
        }
      }
      while(--length);
    }
  }
}

std::string GlideJsonTest::generateJson(const int &mode) {
  std::string output;
  GlideJsonTest::generateJson(mode, &output, NULL);
  return output;
}

std::pair<std::string, std::string> GlideJsonTest::generateJsonWithWs() {
  std::pair<std::string, std::string> output;
  GlideJsonTest::generateJson(std::rand() % 7, &(output.first), &(output.second));
  return output;
}

void GlideJsonTest::injectBadUtf8(const bool &lenient, std::string &input, std::string &output) {
  size_t theSize(output.size() - 1);
  size_t badPos(std::rand() % theSize);
  ++badPos;
  std::string badSeq;
  int mode(std::rand() % 16);
  switch(mode) {
    case 0:
      badSeq += (unsigned char)(std::rand() % 32);
      break;
    case 1:
      badSeq += (unsigned char)(128 + std::rand() % 66);
      break;
    case 2:
      badSeq += (unsigned char)(245 + std::rand() % 11);
      break;
    case 3:
      badSeq += (unsigned char)(194 + std::rand() % 30);
      if(std::rand() & 1) {
        if(std::rand() & 1) {
          badSeq += (unsigned char)(std::rand() % 128);
        }
        else {
          badSeq += (unsigned char)(192 + std::rand() % 64);
        }
      }
      break;
    case 4:
      badSeq += (unsigned char)224;
      if(std::rand() & 1) {
        if(std::rand() & 1) {
          badSeq += (unsigned char)(std::rand() % 160);
        }
        else {
          badSeq += (unsigned char)(192 + std::rand() % 64);
        }
      }
      break;
    case 5:
      if(std::rand() & 1) {
        badSeq += (unsigned char)(225 + std::rand() % 12);
      }
      else {
        badSeq += (unsigned char)(238 + (std::rand() & 1));
      }
      if(std::rand() & 1) {
        if(std::rand() & 1) {
          badSeq += (unsigned char)(std::rand() % 128);
        }
        else {
          badSeq += (unsigned char)(192 + std::rand() % 64);
        }
      }
      break;
    case 6:
      badSeq += (unsigned char)237;
      if(std::rand() & 1) {
        if(std::rand() & 1) {
          badSeq += (unsigned char)(std::rand() % 128);
        }
        else {
          badSeq += (unsigned char)(160 + std::rand() % 96);
        }
      }
      break;
    case 7:
      badSeq += (unsigned char)224;
      badSeq += (unsigned char)(160 + std::rand() % 32);
      if(std::rand() & 1) {
        if(std::rand() & 1) {
          badSeq += (unsigned char)(std::rand() % 128);
        }
        else {
          badSeq += (unsigned char)(192 + std::rand() % 64);
        }
      }
      break;
    case 8:
      if(std::rand() & 1) {
        badSeq += (unsigned char)(225 + std::rand() % 12);
      }
      else {
        badSeq += (unsigned char)(238 + (std::rand() & 1));
      }
      badSeq += (unsigned char)(128 + std::rand() % 64);
      if(std::rand() & 1) {
        if(std::rand() & 1) {
          badSeq += (unsigned char)(std::rand() % 128);
        }
        else {
          badSeq += (unsigned char)(192 + std::rand() % 64);
        }
      }
      break;
    case 9:
      badSeq += (unsigned char)237;
      badSeq += (unsigned char)(128 + std::rand() % 32);
      if(std::rand() & 1) {
        if(std::rand() & 1) {
          badSeq += (unsigned char)(std::rand() % 128);
        }
        else {
          badSeq += (unsigned char)(192 + std::rand() % 64);
        }
      }
      break;
    case 10:
      badSeq += (unsigned char)240;
      if(std::rand() & 1) {
        if(std::rand() & 1) {
          badSeq += (unsigned char)(std::rand() % 144);
        }
        else {
          badSeq += (unsigned char)(192 + std::rand() % 64);
        }
      }
      break;
    case 11:
      badSeq += (unsigned char)(241 + std::rand() % 3);
      if(std::rand() & 1) {
        if(std::rand() & 1) {
          badSeq += (unsigned char)(std::rand() % 128);
        }
        else {
          badSeq += (unsigned char)(192 + std::rand() % 64);
        }
      }
      break;
    case 12:
      badSeq += (unsigned char)244;
      if(std::rand() & 1) {
        if(std::rand() & 1) {
          badSeq += (unsigned char)(std::rand() % 128);
        }
        else {
          badSeq += (unsigned char)(144 + std::rand() % 112);
        }
      }
      break;
    case 13:
      badSeq += (unsigned char)240;
      badSeq += (unsigned char)(144 + std::rand() % 48);
      if(std::rand() & 1) {
        if(std::rand() & 1) {
          badSeq += (unsigned char)(std::rand() % 128);
        }
        else {
          badSeq += (unsigned char)(192 + std::rand() % 64);
        }
      }
      else if(std::rand() & 1) {
        badSeq += (unsigned char)(128 + std::rand() % 64);
        if(std::rand() & 1) {
          if(std::rand() & 1) {
            badSeq += (unsigned char)(std::rand() % 128);
          }
          else {
            badSeq += (unsigned char)(192 + std::rand() % 64);
          }
        }
      }
      break;
    case 14:
      badSeq += (unsigned char)(241 + std::rand() % 3);
      badSeq += (unsigned char)(128 + std::rand() % 64);
      if(std::rand() & 1) {
        if(std::rand() & 1) {
          badSeq += (unsigned char)(std::rand() % 128);
        }
        else {
          badSeq += (unsigned char)(192 + std::rand() % 64);
        }
      }
      else if(std::rand() & 1) {
        badSeq += (unsigned char)(128 + std::rand() % 64);
        if(std::rand() & 1) {
          if(std::rand() & 1) {
            badSeq += (unsigned char)(std::rand() % 128);
          }
          else {
            badSeq += (unsigned char)(192 + std::rand() % 64);
          }
        }
      }
      break;
    case 15:
      badSeq += (unsigned char)244;
      badSeq += (unsigned char)(128 + std::rand() % 16);
      if(std::rand() & 1) {
        if(std::rand() & 1) {
          badSeq += (unsigned char)(std::rand() % 128);
        }
        else {
          badSeq += (unsigned char)(192 + std::rand() % 64);
        }
      }
      else if(std::rand() & 1) {
        badSeq += (unsigned char)(128 + std::rand() % 64);
        if(std::rand() & 1) {
          if(std::rand() & 1) {
            badSeq += (unsigned char)(std::rand() % 128);
          }
          else {
            badSeq += (unsigned char)(192 + std::rand() % 64);
          }
        }
      }
      break;
    default:
      abort();
  }
  if(lenient) {
    unsigned char theChar(output[badPos]);
    while(theChar > 127 && badPos < theSize) {
      theChar = output[++badPos];
    }
    theSize = badSeq.size() - 1;
    theChar = badSeq[theSize];
    if(theChar < 128) {
      input.insert(badPos, 1, theChar);
    }
  }
  output.insert(badPos, badSeq);
}

std::string GlideJsonTest::generateBadUtf8() {
  std::string output;
  GlideJsonTest::generateJson(4, &output, NULL);
  GlideJsonTest::injectBadUtf8(false, output, output);
  return output;
}

std::pair<std::string, std::string> GlideJsonTest::generateBadUtf8Pair() {
  std::pair<std::string, std::string> output;
  GlideJsonTest::generateJson(4, &(output.first), NULL);
  output.second = output.first;
  GlideJsonTest::injectBadUtf8(true, output.first, output.second);
  return output;
}

std::string GlideJsonTest::breakContainer(const unsigned char &input) {
  static const unsigned char symbols[] = {'[', ']', ',', '{', '}', ':'};
  std::string output;
  int random(std::rand() % 6);
  switch(input) {
    case '[':
      if(random != 0) {
        output = symbols[random];
      }
      break;
    case ']':
      if(random != 1) {
        output = symbols[random];
      }
      break;
    case ',':
      /*
        Cropping a comma can cause two JSON numbers to combine into one valid
        JSON number, so we add a second comma instead.
      */
      if(random == 2) {
        output = ",,";
      }
      else {
        output = symbols[random];
      }
      break;
    case '{':
      if(random != 3) {
        output = symbols[random];
      }
      break;
    case '}':
      if(random != 4) {
        output = symbols[random];
      }
      break;
    case ':':
      if(random != 5) {
        output = symbols[random];
      }
      break;
    default:
      abort();
  }
  return output;
}

size_t GlideJsonTest::locateSymbol(const std::string &input, const size_t &skip, const char &target) {
  size_t i(0);
  size_t l(input.size());
  size_t cSkip(skip);
  bool noQuote(true);
  if(i < l) {
    do {
      if(noQuote) {
        if(input[i] == '"') {
          noQuote = false;
        }
        else if(input[i] == target) {
          if(cSkip) {
            --cSkip;
          }
          else {
            return i;
          }
        }
      }
      else {
        if(input[i] == '\\') {
          ++i;
        }
        else if(input[i] == '"') {
          noQuote = true;
        }
      }
    }
    while(++i < l);
  }
  return i;
}

std::string GlideJsonTest::generateBadContainer(const int &mode) {
  std::string output;
  size_t outputSize, symbol;
  int skip, target;
  switch(mode) {
    case 0:
      output = GlideJsonTest::generateJson(5);
      outputSize = output.size();
      skip = std::rand() % 5;
      for(;;) {
        target = std::rand() % 3;
        if(target == 0) {
          symbol = GlideJsonTest::locateSymbol(output, skip, ',');
          if(symbol < outputSize) {
            output.replace(symbol, 1, GlideJsonTest::breakContainer(','));
            break;
          }
          else {
            target = 1;
          }
        }
        if(target == 1) {
          symbol = GlideJsonTest::locateSymbol(output, skip, '[');
          if(symbol < outputSize) {
            output.replace(symbol, 1, GlideJsonTest::breakContainer('['));
            break;
          }
          else {
            target = 2;
          }
        }
        if(target == 2) {
          symbol = GlideJsonTest::locateSymbol(output, skip, ']');
          if(symbol < outputSize) {
            output.replace(symbol, 1, GlideJsonTest::breakContainer(']'));
            break;
          }
        }
        if(skip) {
          --skip;
        }
        else {
          break;
        }
      }
      break;
    case 1:
      output = GlideJsonTest::generateJson(6);
      outputSize = output.size();
      skip = std::rand() % 5;
      for(;;) {
        target = std::rand() % 4;
        if(target == 0) {
          symbol = GlideJsonTest::locateSymbol(output, skip, ',');
          if(symbol < outputSize) {
            output.replace(symbol, 1, GlideJsonTest::breakContainer(','));
            break;
          }
          else {
            target = 1;
          }
        }
        if(target == 1) {
          symbol = GlideJsonTest::locateSymbol(output, skip, ':');
          if(symbol < outputSize) {
            output.replace(symbol, 1, GlideJsonTest::breakContainer(':'));
            break;
          }
          else {
            target = 2;
          }
        }
        if(target == 2) {
          symbol = GlideJsonTest::locateSymbol(output, skip, '{');
          if(symbol < outputSize) {
            output.replace(symbol, 1, GlideJsonTest::breakContainer('{'));
            break;
          }
          else {
            target = 3;
          }
        }
        if(target == 3) {
          symbol = GlideJsonTest::locateSymbol(output, skip, '}');
          if(symbol < outputSize) {
            output.replace(symbol, 1, GlideJsonTest::breakContainer('}'));
            break;
          }
        }
        if(skip) {
          --skip;
        }
        else {
          break;
        }
      }
      break;
    default:
      abort();
  }
  return output;
}

std::string GlideJsonTest::generateBadContainer() {
  return GlideJsonTest::generateBadContainer(std::rand() & 1);
}

void GlideJsonTest::testCodepoints() {
  unsigned char hex;
  unsigned int i;
  std::string generated, testEncoded;
  GlideJson testJson;
  auto start(std::chrono::steady_clock::now());
  // Parsing range:
  generated = "\"";
  generated.append("\\u0000 - \\u0001 - \\u0002 - \\u007e - \\u007f - \\u0080 - \\u0081 - \\u0082");
  generated.append(" - \\u07FE - \\u07FF - \\u0800 - \\u0801 - \\u0802");
  generated.append(" - \\u0FFF - \\u1000 - \\u1001 - \\uFFFD - \\uFFFE - \\uFFFF");
  generated.append("\"");
  testJson = GlideJson::parse(generated);
  testEncoded = std::string("\x00 - \x01 - \x02 - \x7e - \x7f - \xc2\x80 - \xc2\x81 - \xc2\x82", 32);
  testEncoded.append(" - \xDF\xBE - \xDF\xBF - \xE0\xA0\x80 - \xE0\xA0\x81 - \xE0\xA0\x82", 28);
  testEncoded.append(" - \xE0\xBF\xBF - \xE1\x80\x80 - \xE1\x80\x81 - \xEF\xBF\xBD - \xEF\xBF\xBE - \xEF\xBF\xBF", 36);
  if(testEncoded != testJson.string()) {
    GlideJsonTest::errors.push_back("The unicode JSON:\n\n" + generated + "\n\n... produced " + testJson.toJson() + "\n\n... but does not match:\n\n" + testEncoded + "\n");
  }
  // Test the escaped unicode range:
  i = 0;
  do {
    // Lower case:
    generated = "\"\\u";
    generated.push_back(GlideJson::getHex(i >> 12));
    generated.push_back(GlideJson::getHex((i >> 8) & 15));
    generated.push_back(GlideJson::getHex((i >> 4) & 15));
    generated.push_back(GlideJson::getHex(i & 15));
    generated.push_back('"');
    testEncoded = GlideJson::parse(generated).string();
    if(
      ((testEncoded.size() != 1) & (i < 0x0080))
      | ((testEncoded.size() != 2) & (i >= 0x0080) & (i < 0x0800))
      | ((testEncoded.size() != 3) & (i >= 0x0800) & (i <= 0xFFFF))
      | (testEncoded.size() > 3)
      | (testEncoded.size() < 1)
    ) {
      GlideJsonTest::errors.push_back("The unicode JSON " + generated + " produced a UTF-8 sequence with size " + std::to_string(testEncoded.size()) + "!");
    }
    // Upper case:
    generated = "\"\\u";
    hex = GlideJson::getHex(i >> 12);
    generated.push_back((hex >= 97) ? hex - 32 : hex);
    hex = GlideJson::getHex((i >> 8) & 15);
    generated.push_back((hex >= 97) ? hex - 32 : hex);
    hex = GlideJson::getHex((i >> 4) & 15);
    generated.push_back((hex >= 97) ? hex - 32 : hex);
    hex = GlideJson::getHex(i & 15);
    generated.push_back((hex >= 97) ? hex - 32 : hex);
    generated.push_back('"');
    testEncoded = GlideJson::parse(generated).string();
    if(
      ((testEncoded.size() != 1) & (i < 0x0080))
      | ((testEncoded.size() != 2) & (i >= 0x0080) & (i < 0x0800))
      | ((testEncoded.size() != 3) & (i >= 0x0800) & (i <= 0xFFFF))
      | (testEncoded.size() > 3)
      | (testEncoded.size() < 1)
    ) {
      GlideJsonTest::errors.push_back("The unicode JSON " + generated + " produced a UTF-8 sequence with size " + std::to_string(testEncoded.size()) + "!");
    }
  }
  while(++i <= 0xFFFF);
  std::cout << "Codepoints test: " << (std::chrono::steady_clock::now() - start).count() << "\n";
}

void GlideJsonTest::testBase64() {
  std::string input, output, test, base64Map;
  unsigned int i, j;
  base64Map = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  auto start(std::chrono::steady_clock::now());
  // Base64 smoke test:
  input = "Man is distinguished, not only by his reason, but by this singular passion from other animals, which is a lust of the mind, that by a perseverance of delight in the continued and indefatigable generation of knowledge, exceeds the short vehemence of any carnal pleasure.";
  output = "TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIGJ1dCBieSB0aGlzIHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxzLCB3aGljaCBpcyBhIGx1c3Qgb2YgdGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZXZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aGUgY29udGludWVkIGFuZCBpbmRlZmF0aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZGdlLCBleGNlZWRzIHRoZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS4=";
  test = GlideJson::base64Encode(input);
  if(output != test) {
    GlideJsonTest::errors.push_back("The input:\n\n" + input + "\n\n... gave the Base64 output:\n\n" + test + "\n\n... instead of:\n\n" + output);
  }
  test = GlideJson::base64Decode(output);
  if(input != test) {
    GlideJsonTest::errors.push_back("The Base64 input:\n\n" + output + "\n\n... gave the output:\n\n" + test + "\n\n... instead of:\n\n" + input);
  }
  input = "any carnal pleasure.";
  output = "YW55IGNhcm5hbCBwbGVhc3VyZS4=";
  test = GlideJson::base64Encode(input);
  if(output != test) {
    GlideJsonTest::errors.push_back("The input:\n\n" + input + "\n\n... gave the Base64 output:\n\n" + test + "\n\n... instead of:\n\n" + output);
  }
  test = GlideJson::base64Decode(output);
  if(input != test) {
    GlideJsonTest::errors.push_back("The Base64 input:\n\n" + output + "\n\n... gave the output:\n\n" + test + "\n\n... instead of:\n\n" + input);
  }
  input = "any carnal pleasure.";
  output = "YW55IGNhcm5hbCBwbGVhc3VyZS4";
  test = GlideJson::base64Decode(output);
  if(input != test) {
    GlideJsonTest::errors.push_back("The Base64 input:\n\n" + output + "\n\n... gave the output:\n\n" + test + "\n\n... instead of:\n\n" + input);
  }
  input = "any carnal pleasure";
  output = "YW55IGNhcm5hbCBwbGVhc3VyZQ==";
  test = GlideJson::base64Encode(input);
  if(output != test) {
    GlideJsonTest::errors.push_back("The input:\n\n" + input + "\n\n... gave the Base64 output:\n\n" + test + "\n\n... instead of:\n\n" + output);
  }
  test = GlideJson::base64Decode(output);
  if(input != test) {
    GlideJsonTest::errors.push_back("The Base64 input:\n\n" + output + "\n\n... gave the output:\n\n" + test + "\n\n... instead of:\n\n" + input);
  }
  input = "any carnal pleasure";
  output = "YW55IGNhcm5hbCBwbGVhc3VyZQ";
  test = GlideJson::base64Decode(output);
  if(input != test) {
    GlideJsonTest::errors.push_back("The Base64 input:\n\n" + output + "\n\n... gave the output:\n\n" + test + "\n\n... instead of:\n\n" + input);
  }
  input = "any carnal pleasur";
  output = "YW55IGNhcm5hbCBwbGVhc3Vy";
  test = GlideJson::base64Encode(input);
  if(output != test) {
    GlideJsonTest::errors.push_back("The input:\n\n" + input + "\n\n... gave the Base64 output:\n\n" + test + "\n\n... instead of:\n\n" + output);
  }
  test = GlideJson::base64Decode(output);
  if(input != test) {
    GlideJsonTest::errors.push_back("The Base64 input:\n\n" + output + "\n\n... gave the output:\n\n" + test + "\n\n... instead of:\n\n" + input);
  }
  input = "any carnal pleasu";
  output = "YW55IGNhcm5hbCBwbGVhc3U=";
  test = GlideJson::base64Encode(input);
  if(output != test) {
    GlideJsonTest::errors.push_back("The input:\n\n" + input + "\n\n... gave the Base64 output:\n\n" + test + "\n\n... instead of:\n\n" + output);
  }
  test = GlideJson::base64Decode(output);
  if(input != test) {
    GlideJsonTest::errors.push_back("The Base64 input:\n\n" + output + "\n\n... gave the output:\n\n" + test + "\n\n... instead of:\n\n" + input);
  }
  input = "any carnal pleasu";
  output = "YW55IGNhcm5hbCBwbGVhc3U";
  test = GlideJson::base64Decode(output);
  if(input != test) {
    GlideJsonTest::errors.push_back("The Base64 input:\n\n" + output + "\n\n... gave the output:\n\n" + test + "\n\n... instead of:\n\n" + input);
  }
  input = "any carnal pleas";
  output = "YW55IGNhcm5hbCBwbGVhcw==";
  test = GlideJson::base64Encode(input);
  if(output != test) {
    GlideJsonTest::errors.push_back("The input:\n\n" + input + "\n\n... gave the Base64 output:\n\n" + test + "\n\n... instead of:\n\n" + output);
  }
  test = GlideJson::base64Decode(output);
  if(input != test) {
    GlideJsonTest::errors.push_back("The Base64 input:\n\n" + output + "\n\n... gave the output:\n\n" + test + "\n\n... instead of:\n\n" + input);
  }
  input = "any carnal pleas";
  output = "YW55IGNhcm5hbCBwbGVhcw";
  test = GlideJson::base64Decode(output);
  if(input != test) {
    GlideJsonTest::errors.push_back("The Base64 input:\n\n" + output + "\n\n... gave the output:\n\n" + test + "\n\n... instead of:\n\n" + input);
  }
  //
  i = 1000;
  do {
    // Encoding:
    input.clear();
    j = 1000;
    do {
      if(std::rand() % 3) {
        input.push_back((unsigned char)(std::rand() % 256));
      }
    }
    while(--j);
    test = GlideJson::base64Encode(input);
    output = GlideJson::base64Decode(test);
    if(output != input) {
      GlideJsonTest::errors.push_back("The input for Base64 sanity testing:\n\n" + input + "\n\n... does not equal:\n\n" + output);
    }
    input.clear();
    // Decoding:
    j = 1000;
    do {
      if(std::rand() % 3) {
        input.push_back(base64Map[std::rand() % 64]);
      }
    }
    while(--j);
    /*
      Since we're making up Base64 sequences, we have to be aware of
      discarded bits. If a trailing Base64 character results in output bits
      that aren't needed by the output, make sure they're zero.
    */
    j = input.size() % 4;
    switch(j) {
      case 0:
        break;
      case 1:
        input.push_back(base64Map[(std::rand() % 64) & 48]);
        break;
      case 2:
        input[input.size() - 1] = base64Map[(std::rand() % 64) & 48];
        break;
      case 3:
        input[input.size() - 1] = base64Map[(std::rand() % 64) & 60];
        break;
      default:
        abort();
    }
    // Decoding without '=':
    test = GlideJson::base64Decode(input);
    output = GlideJson::base64Encode(test);
    j = input.size() % 4;
    input.append(4 * (j > 0) - j, '=');
    if(output != input) {
      GlideJsonTest::errors.push_back("The input for Base64 sanity testing:\n\n" + input + "\n\n... does not equal:\n\n" + output);
    }
    // Decoding with '=':
    test = GlideJson::base64Decode(input);
    output = GlideJson::base64Encode(test);
    if(output != input) {
      GlideJsonTest::errors.push_back("The input for Base64 sanity testing:\n\n" + input + "\n\n... does not equal:\n\n" + output);
    }
  }
  while(--i);
  std::cout << "Base64 test: " << (std::chrono::steady_clock::now() - start).count() << "\n";
}

void GlideJsonTest::testJson() {
  std::pair<std::string, std::string> generated;
  GlideJson testJson;
  std::string testEncoded;
  unsigned char mode;
  unsigned int i;
  auto start(std::chrono::steady_clock::now());
  i = 10000;
  do {
    generated = GlideJsonTest::generateJsonWithWs();
    testJson = GlideJson::parse(generated.first);
    testEncoded = testJson.toJson();
    if(testEncoded != generated.first) {
      GlideJsonTest::errors.push_back("The generated JSON:\n\n" + generated.first + "\n\n... does not match:\n\n" + testEncoded + "\n");
    }
    else {
      testJson = GlideJson::parse(generated.second);
      mode = std::rand() % 4;
      switch(mode) {
        case 0:
          testEncoded = testJson.toJson(GlideJson::SpaceLf);
          break;
        case 1:
          testEncoded = testJson.toJson(GlideJson::TabLf);
          break;
        case 2:
          testEncoded = testJson.toJson(GlideJson::SpaceCrlf);
          break;
        case 3:
          testEncoded = testJson.toJson(GlideJson::TabCrlf);
          break;
        default:
          abort();
      }
      testJson = GlideJson::parse(testEncoded);
      testEncoded = testJson.toJson();
      if(testEncoded != generated.first) {
        GlideJsonTest::errors.push_back("The generated JSON:\n\n" + generated.second + "\n\n... does not match:\n\n" + testEncoded + "\n");
      }
    }
  }
  while(--i > 0);
  std::cout << "JSON test: " << (std::chrono::steady_clock::now() - start).count() << "\n";
}

void GlideJsonTest::testJsonUtf8() {
  std::string generated;
  std::pair<std::string, std::string> generatedPair;
  GlideJson testJson;
  unsigned int i;
  auto start(std::chrono::steady_clock::now());
  i = 20000;
  do {
    generated = GlideJsonTest::generateBadUtf8();
    testJson = GlideJson::parse(generated);
    if(testJson.getType() != GlideJson::Error) {
      GlideJsonTest::errors.push_back("The generated JSON is somehow valid:\n\n" + generated + "\n");
    }
  }
  while(--i > 0);
  i = 20000;
  do {
    generatedPair = GlideJsonTest::generateBadUtf8Pair();
    testJson = GlideJson::parse(GlideJson::encodeString(generatedPair.second));
    if(testJson.getType() == GlideJson::String) {
      if(testJson.string() != generatedPair.first) {
        GlideJsonTest::errors.push_back("The encoded bad UTF-8 does not match the original UTF-8:\n\n" + generatedPair.first + "\n");
      }
    }
    else {
      GlideJsonTest::errors.push_back("The parsed bad UTF-8 is not a string!");
    }
  }
  while(--i > 0);
  std::cout << "JSON UTF-8 test: " << (std::chrono::steady_clock::now() - start).count() << "\n";
}

void GlideJsonTest::testJsonContainers() {
  std::string generated;
  GlideJson testJson;
  unsigned int i;
  auto start(std::chrono::steady_clock::now());
  i = 5000;
  do {
    generated = GlideJsonTest::generateBadContainer();
    testJson = GlideJson::parse(generated);
    if(testJson.getType() != GlideJson::Error) {
      GlideJsonTest::errors.push_back("The generated JSON is somehow valid:\n\n" + generated + "\n");
    }
  }
  while(--i > 0);
  std::cout << "JSON containers test: " << (std::chrono::steady_clock::now() - start).count() << "\n";
}

void GlideJsonTest::testJsonInParallel() {
  std::pair<std::string, std::string> generated;
  GlideJson testJson;
  std::string testEncoded;
  unsigned int i;
  auto start(std::chrono::steady_clock::now());
  i = 2000;
  do {
    generated = GlideJsonTest::generateJsonWithWs();
    testJson = GlideJson::parse(generated.first);
    testEncoded = testJson.toJson();
    if(testEncoded != generated.first) {
      GlideJsonTest::errors.push_back("The generated JSON:\n\n" + generated.first + "\n\n... does not match:\n\n" + testEncoded + "\n");
    }
    else {
      testJson = GlideJson::parse(generated.second);
      testEncoded = testJson.toJson();
      if(testEncoded != generated.first) {
        GlideJsonTest::errors.push_back("The generated JSON:\n\n" + generated.second + "\n\n... does not match:\n\n" + testEncoded + "\n");
      }
    }
  }
  while(--i > 0);
  std::cout << "JSON parallel test: " << (std::chrono::steady_clock::now() - start).count() << "\n";
}

int GlideJsonTest::run() {
  std::srand(std::time(NULL));
  GlideJsonTest::testGlideHashMap();
  GlideJsonTest::testCodepoints();
  GlideJsonTest::testBase64();
  GlideJsonTest::testJson();
  GlideJsonTest::testJsonUtf8();
  GlideJsonTest::testJsonContainers();
  // Check errors:
  size_t errorCount(GlideJsonTest::errors.size());
  if(errorCount) {
    std::cout << "\n--- " << errorCount << " ERRORS ---\n\n";
    size_t i(0);
    do {
      std::cout << GlideJsonTest::errors[i] << "\n";
    }
    while(++i < errorCount);
    return 1;
  }
  else {
    std::thread t1(GlideJsonTest::testJsonInParallel);
    std::thread t2(GlideJsonTest::testJsonInParallel);
    std::thread t3(GlideJsonTest::testJsonInParallel);
    std::thread t4(GlideJsonTest::testJsonInParallel);
    t1.join();
    t2.join();
    t3.join();
    t4.join();
  }
  return 0;
}

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;
  return GlideJsonTest::run();
}
