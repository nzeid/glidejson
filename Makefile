# Copyright (c) 2020 Nader G. Zeid
#
# This file is part of GlideJson.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GlideJson. If not, see <https://www.gnu.org/licenses/gpl.html>.

CC = g++
CC1FLAGS = -std=c++11 -Wpedantic -Wall -Wextra -Wpointer-arith -Werror=vla -march=native
CC2FLAGS = -c -O3 -I./include/glidejson -I./include/test
CCOFLAGS = $(CC2FLAGS) $(CC1FLAGS)
CCSOFLAGS = $(CC2FLAGS) -fPIC $(CC1FLAGS)

GLIDE_JSON = lib/libglidejson.so
GLIDE_JSON_SOURCES = $(wildcard src/glidejson/*.cpp)
GLIDE_JSON_OBJECTS = $(GLIDE_JSON_SOURCES:.cpp=.o)

GLIDE_JSON_TEST = bin/GlideJsonTest
GLIDE_JSON_TEST_SOURCES = $(wildcard src/test/*.cpp)
GLIDE_JSON_TEST_OBJECTS = $(GLIDE_JSON_TEST_SOURCES:.cpp=.o)

.PHONY: all clean distclean

all: $(GLIDE_JSON_TEST)

clean:
	rm -vrf lib bin $(GLIDE_JSON_OBJECTS) $(GLIDE_JSON_TEST_OBJECTS)

distclean: clean

src/glidejson/%.o: src/glidejson/%.cpp
	$(CC) $(CCSOFLAGS) -o $@ $<

lib:
	mkdir -p lib

bin: lib
	mkdir -p bin
	ln -sn ../lib bin/lib

$(GLIDE_JSON): $(GLIDE_JSON_OBJECTS) lib
	$(CC) -shared $(CC1FLAGS) -o $(GLIDE_JSON) $(GLIDE_JSON_OBJECTS)

src/test/%.o: src/test/%.cpp $(GLIDE_JSON)
	$(CC) $(CCOFLAGS) -o $@ $<

$(GLIDE_JSON_TEST): $(GLIDE_JSON_TEST_OBJECTS) bin
	$(CC) $(CC1FLAGS) -Wl,-rpath,\$$ORIGIN/lib -o $(GLIDE_JSON_TEST) $(GLIDE_JSON_TEST_OBJECTS) -L./lib -lpthread -lglidejson
