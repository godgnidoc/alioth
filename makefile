SHELL = /bin/bash

LEXICAL := $(wildcard syntax/*.fl)
LSRC := $(LEXICAL:syntax/%.fl=src/%.cpp)
SYNTAX := $(wildcard syntax/*.yy)
SSRC := $(SYNTAX:syntax/%.yy=src/%.cpp)
SRC := $(wildcard src/*.cpp) $(wildcard src/**/*.cpp) $(LSRC) $(SSRC)
SRC := $(shell echo $(SRC) | tr " " "\n" | sort -u | xargs )
OBJ := $(SRC:src/%.cpp=obj/%.o)
DEP := $(SRC:src/%.cpp=temp/%.d)

ARCH    =x86_64
OS      =Linux
VERSION =1.0.0-build1

CC = g++
CPPFLAGS = -g -Iinc -Iinc/basis -std=gnu++20 -D__ARCH=$(ARCH) -D__OS=$(OS) -D__VERSION=$(VERSION)
LIBRARIES = -lstdc++fs

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

clean:
	rm $(SSRC) $(LSRC) $(OBJ) $(DEP)

.PHONY:clean