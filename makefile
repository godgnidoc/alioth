SHELL = /bin/bash

LEXICAL := $(wildcard syntax/*.fl)
LSRC := $(LEXICAL:syntax/%.fl=src/%.cpp)
SYNTAX := $(wildcard syntax/*.yy)
SSRC := $(SYNTAX:syntax/%.yy=src/%.cpp)
SRC := $(wildcard src/*.cpp) $(wildcard src/**/*.cpp) $(LSRC) $(SSRC)
SRC := $(shell echo $(SRC) | tr " " "\n" | sort -u | xargs )
OBJ := $(SRC:src/%.cpp=obj/%.o)
DEP := $(SRC:src/%.cpp=temp/%.d)

NAME	="$(shell node scripts/about.js name)"
VERSION	="$(shell node scripts/about.js version)"
AUTHOR  ="$(shell node scripts/about.js publisher.name)"
EMAIL	="$(shell node scripts/about.js publisher.email)"
BRIEF	="$(shell node scripts/about.js brief)"
ARCH    ="$(shell node scripts/about.js arch)"
OS      ="$(shell node scripts/about.js os)"
LICENSE	="$(shell node scripts/about.js license)"

CC = g++
CPPFLAGS = -g -Iinc -Iinc/basis -std=gnu++20 
CPPFLAGS += -D__NAME=$(NAME) -D__VERSION=$(VERSION) -D__BRIEF=$(BRIEF)
CPPFLAGS += -D__ARCH=$(ARCH) -D__OS=$(OS)
CPPFLAGS += -D__AUTHOR=$(AUTHOR) -D__EMAIL=$(EMAIL) -D__LICENSE=$(LICENSE)
LIBRARIES = -lstdc++fs

build:
	node scripts/about.js build
	make bin/alioth

bin/alioth: $(OBJ)
	$(CC) $(CPPFLAGS) -o $@ $(OBJ)

include $(DEP)

$(DEP):temp/%.d:src/%.cpp
	@mkdir -p `dirname $@`
	@$(CC) $(CPPFLAGS) -MM $< -MT $(<:src/%.cpp=obj/%.o) > $@
	@echo "	@\$$(CC) \$$(CPPFLAGS) -MM \$$< -MT \$$@ > $@ep" >> $@
	@echo "	@sed -i -e \"/^[^\\t]/d;\" $@" >> $@
	@echo "	@cat $@ep $@ | tee $@ > /dev/null " >> $@
	@echo "	@rm $@ep" >> $@
	@echo "	@mkdir -p \`dirname \$$@\`" >> $@
	@echo "	\$$(CC) \$$(CPPFLAGS) -c \$$< -o \$$@" >> $@

$(LSRC):src/%.cpp:syntax/%.fl
	flex -Ce -o $@ $<

$(SSRC):src/%.cpp:syntax/%.yy
	bison $<

src/compiler.options.cpp:inc/compiler.hpp scripts/options.js
	node scripts/options.js

src/logger.cpp inc/logger.helper.hpp src/logger.helper.cpp:doc/logging.json scripts/logging.js
	node scripts/logging.js

clean:
	rm $(SSRC) $(LSRC) $(OBJ) $(DEP)

.PHONY:clean build