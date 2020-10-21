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

#ifdef GLIDE_JSON_NO_WHITESPACE
std::string Array::toJson() const {
#endif
#ifdef GLIDE_JSON_WHITESPACE
std::string Array::toJson(const GlideJson::Whitespace &type, const size_t &depth) const {
  std::string indent;
  size_t width;
  std::string newLine;
  switch(type) {
    case GlideJson::SpaceLf:
      width = 2;
      indent.assign(width * (depth + 1), ' ');
      newLine = '\n';
      break;
    case GlideJson::SpaceCrlf:
      width = 2;
      indent.assign(width * (depth + 1), ' ');
      newLine = "\r\n";
      break;
    case GlideJson::TabLf:
      width = 1;
      indent.assign(width * (depth + 1), '\t');
      newLine = '\n';
      break;
    case GlideJson::TabCrlf:
      width = 1;
      indent.assign(width * (depth + 1), '\t');
      newLine = "\r\n";
      break;
    default:
      abort();
  }
#endif
  std::string output(1, '[');
  size_t arraySize(array.size());
  if(arraySize) {
    size_t i(0);
    #ifdef GLIDE_JSON_WHITESPACE
    output += newLine;
    output += indent;
    output += array[i].toJson(type, depth + 1);
    #endif
    #ifdef GLIDE_JSON_NO_WHITESPACE
    output += array[i].toJson();
    #endif
    while(++i < arraySize) {
      output += ',';
      #ifdef GLIDE_JSON_WHITESPACE
      output += newLine;
      output += indent;
      output += array[i].toJson(type, depth + 1);
      #endif
      #ifdef GLIDE_JSON_NO_WHITESPACE
      output += array[i].toJson();
      #endif
    }
    #ifdef GLIDE_JSON_WHITESPACE
    indent.resize(indent.size() - width);
    output += newLine;
    output += indent;
    #endif
  }
  output += ']';
  return output;
}
