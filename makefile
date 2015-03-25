################################################################################
# make		- build all object files
# make rebld	- clean up and build objs
# make test	- compile with tb and run
# make dist	- establish the programm
# make dbg	- run test with gdb (mod = debug)
# make tgmod	- toggle between debug/release mode
# make dply 	- deploy project structure
# make pkg	- create new packages
# make pkg_scan	- scan package structure
# make pkg_del	- delete packages
# make libdp	- add lib dependency
# make libdp_rst- reset lib dependency
# make tpsrc	- create source using templates
# make tb	- create testbench template
# make tags	- update tags file
# make clean	- clean up objs, deps and tags
# make purge	- clean up all the files !
################################################################################
# project compiling environment definition
SHELL := /bin/sh
CC := gcc
MAKE := make

# project directory structure
PRJ_PREFIX := pie
TOP_DIR := $(PWD)
# TOP_DIR := /home/qiuwen/Projects/imgm/

SRC_DIR := $(TOP_DIR)/src/
BLD_DIR := $(TOP_DIR)/bld/
LIB_DIR := $(TOP_DIR)/lib/
DOC_DIR := $(TOP_DIR)/doc/
MAK_DIR := $(TOP_DIR)/mak/
DIST_DIR := $(TOP_DIR)/dist/
TEST_DIR := $(TOP_DIR)/test/
TEMPLATE_DIR := $(TOP_DIR)/template/
RESOURCE_DIR := $(TOP_DIR)/resource/
OBJ_DIR := $(BLD_DIR)obj/
DEP_DIR := $(BLD_DIR)dep/
TAG_DIR := $(BLD_DIR)tag/

# auxiliary makefiles
PACKAGE_MK := $(MAK_DIR)package.mk
LIBDEP_MK := $(MAK_DIR)libdep.mk
CFLAGS_MK := $(MAK_DIR)cflags.mk

# project output/debug setting
TESTBENCH := $(TEST_DIR)TestBench.c
OUTPUT := $(notdir $(TOP_DIR))

# compiling parameters
LIB := -L $(LIB_DIR)
INCLUDE := -I $(SRC_DIR)
DBG_FLAGS := -Wall -std=c99 -g
RLS_FLAGS := -Wall -std=c99 -O2

# auxiliary makefile variables
sinclude $(PACKAGE_MK)
sinclude $(LIBDEP_MK)
sinclude $(CFLAGS_MK)

# dierctory pattern searching paths
SRCPATH := $(SRC_DIR)
SRCPATH += $(foreach p, $(DIR_PACK), $(shell echo ": $(SRC_DIR)$(p)"))
vpath %.c $(SRCPATH)
vpath %.h $(SRCPATH)
vpath %.o $(OBJ_DIR)
vpath %.d $(DEP_DIR)

# list for source, object and dependency files
SOURCE := $(notdir $(wildcard $(SRC_DIR)*.c))
SOURCE += $(foreach p, $(DIR_PACK), $(notdir $(wildcard $(SRC_DIR)$(p)/*.c)))
OBJECT := $(patsubst %.c, %.o, $(SOURCE))
DEPEND := $(patsubst %.o, %.d, $(OBJECT))
HEADER := $(notdir $(wildcard $(SRC_DIR)*.h))
HEADER += $(foreach p, $(DIR_PACK), $(notdir $(wildcard $(SRC_DIR)$(p)/*.h)))
TAGS := $(patsubst %, $(TAG_DIR)%.tag, $(SOURCE))
TAGS += $(patsubst %, $(TAG_DIR)%.tag, $(HEADER))

# pattern rule for object files
%.o: %.c
	@echo "building obj: $(notdir $<) -> $(notdir $@) .. " 
	@$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $(OBJ_DIR)$@
	@echo "--------   OK   --------"

# pattern rule for dependency files
$(DEP_DIR)%.d: %.c
	@echo "building dep: $(notdir $<) -> $(notdir $@) .. "
	@$(CC) $(CFLAGS) $(INCLUDE) -MM $< > tempfile.$$$$; \
	sed "s/e:/\/e/g" < tempfile.$$$$ | sed "s/:/ $(subst /,\/,$@):/g" > $@; \
	rm -rf tempfile.$$$$
	@echo "--------   OK   --------"

# pattern rule for tag filse
$(TAG_DIR)%.tag: $(notdir %)
	@echo "updating tag: $(notdir $<) .. "
	@echo "$< \\" >> temp_tag_file
	@echo "/$(notdir $<)/d" >> temp_sed_file
	@echo "" > $@
	@echo "--------   OK   --------"

# routine for scanning package structure
define scan_package
echo "scanning packages .. "; \
ls -F $(SRC_DIR) | grep / | tr '\n' ' ' > tempfile.$$$$; \
read out < tempfile.$$$$; \
cmp=; \
new="$$out"; \
while ! [ "$$cmp" = "$$out" ];do \
    cmp="$$out"; \
    temp1=; \
    temp2=; \
    for p in $$new;do \
        ls -F $(SRC_DIR)$$p | grep / | tr '\n' ' ' > tempfile.$$$$; \
        read temp1 < tempfile.$$$$; \
        for t in $$temp1;do \
            temp2="$$temp2 $$p$$t"; \
        done; \
    done; \
    new="$$temp2"; \
    out="$$out$$new"; \
done; \
echo "DIR_PACK = $$out" > $(PACKAGE_MK); \
rm -rf tempfile.$$$$; \
echo "--------   OK   --------"; \
echo "package list: $$out"
endef

# build all obj files
all: $(OBJECT) 

# automatic dependency
sinclude $(addprefix $(DEP_DIR), $(DEPEND))

# clean and build all
rebld: clean all

# run testbench
test: all
	@if ! [ -e "$(TESTBENCH)" ]; then \
	    echo "cannot find test bench file: $(TESTBENCH)"; \
	    exit 11; \
	fi
	@echo "building output: $(OUTPUT) .. "
	@$(CC) $(TESTBENCH) $(addprefix $(OBJ_DIR),$(OBJECT)) \
	       $(CFLAGS) $(LIB_DEP) $(INCLUDE) -o $(TEST_DIR)$(OUTPUT)
	@echo "--------   OK   --------"
	@echo "start >>>"
	@$(TEST_DIR)$(OUTPUT)
	@echo "test program stopped"

# run with gdb
dbg:
	gdb $(TEST_DIR)$(OUTPUT)

# toggle debug/release mod
tgmod: clean
	@echo -n "select compile mode[debug/release]";	\
	read mode;	\
	if  [ $$mode = debug ]; then \
		echo "CFLAGS = $(DBG_FLAGS)" > $(CFLAGS_MK);	\
	fi;	\
	if  [ $$mode = release ]; then \
		echo "CFLAGS = $(RLS_FLAGS)" > $(CFLAGS_MK);	\
	fi;	\
	echo "mode $$mode";

# deploy project file/env structure
dply:
	@echo "deploy project structure .. "
	@mkdir -p $(SRC_DIR)
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(DEP_DIR)
	@mkdir -p $(TAG_DIR)
	@mkdir -p $(LIB_DIR)
	@mkdir -p $(DOC_DIR)
	@mkdir -p $(MAK_DIR)
	@mkdir -p $(DIST_DIR)
	@mkdir -p $(TEST_DIR)
	@mkdir -p $(TEMPLATE_DIR)
	@mkdir -p $(RESOURCE_DIR)
	@echo "--------   OK   --------"
	@$(call scan_package)
	@echo "LIB_DEP = " > $(LIBDEP_MK)
	@echo "CFLAGS = $(DBG_FLAGS)" > $(CFLAGS_MK)
	@echo "project deployed"

# create new packages
pkg:
	@echo -n "input package names/paths: "
	@read pack; \
	echo "create package: $$pack .. "; \
	cd $(SRC_DIR); \
	mkdir -p $$pack; \
	echo "--------   OK   --------"
	@$(call scan_package)
	
# scan package structure
pkg_scan:
	@$(call scan_package)

# delete packages
pkg_del:
	@echo -n "input package names/paths: "
	@read pack; \
	echo -n "src in package ($$pack) would be deleted as well; continue?[y/n]"; \
	read confirm; \
	if  [ $$confirm = y ]; then \
	    echo "delete package: $$pack .. "; \
	    cd $(SRC_DIR); \
	    rm -rf $$pack; \
	    echo "--------   OK   --------"; \
	fi; \
	@$(call scan_package)

# add lib dependency params for compiling
libdp:
	@echo -n "input lib dependencies: "
	@read libdeps; \
	echo "add lib dependency: $$libdeps .. "; \
	out="LIB_DEP =$(LIB_DEP)"; \
	for l in $$libdeps; do \
	    out="$$out -l $$l"; \
	done; \
	echo "$$out" > $(LIBDEP_MK); \
	echo "--------   OK   --------"; \
	echo "$$out"; \

# reset lib dependency
libdp_rst:	
	@echo "reset dependency .. "
	@echo "LIB_DEP = " > $(LIBDEP_MK)
	@echo "--------   OK   --------"

# generate source files with templates
tpsrc:
	@echo -n "input package name: "; \
	read pack; \
	echo -n "input src stereotype: "; \
	read type; \
	echo -n "input src name: "; \
	read name; \
	echo $$pack | tr '[a-z]' '[A-Z]' > tempfile.$$$$; \
	read upack < tempfile.$$$$; \
	echo $$type | tr '[a-z]' '[A-Z]' > tempfile.$$$$; \
	read utype < tempfile.$$$$; \
	echo $$name | tr '[a-z]' '[A-Z]' > tempfile.$$$$; \
	read uname < tempfile.$$$$; \
	ls $(TEMPLATE_DIR)$$type.* | tr '\n' ' ' > tempfile.$$$$; \
	read temps < tempfile.$$$$; \
	echo "create source using templates: "; \
	for t in $$temps; do \
	    echo "$$t" | awk -F '.' '{print $$2}' > tempfile.$$$$; \
	    read ext < tempfile.$$$$; \
	    echo $$ext | tr '[a-z]' '[A-Z]' > tempfile.$$$$; \
	    read uext < tempfile.$$$$; \
	    echo "$$type.$$ext -> $$name.$$ext .. "; \
	    mkdir -p $(SRC_DIR)$$pack; \
	    sed -e "s/%type/$$type/g" -e "s/%pack/$$pack/g" -e "s/%name/$$name/g" \
	        -e "s/%ext/$$ext/g" -e "s/%TYPE/$$utype/g" -e "s/%PACK/$$upack/g" \
	        -e "s/%NAME/$$uname/g" -e "s/%EXT/$$uext/g" -e "s/%prjpre/$(PRJ_PREFIX)/g"\
	        < $$t > $(SRC_DIR)$$pack/$$name.$$ext; \
	    echo "--------   OK   --------"; \
	done; \
	rm -rf tempfile.$$$$
	@$(call scan_package)

# create testbench src from template
tb:
	@if ! [ -f $(TEMPLATE_DIR)TestBench.c ]; then \
	    echo "testbench template not found"; \
	    exit 11; \
	fi
	@cp $(TEMPLATE_DIR)TestBench.c $(TESTBENCH)
	@echo "test bench template copied"
	
# create tag files
tags: $(TAGS) 
	@if ! [ -f temp_tag_file ]; then \
		echo "tags up to date"; \
		exit 11; \
	fi
	@echo "building tags file .."; \
	read sedpattern < temp_sed_file; \
	read tagfiles < temp_tag_file; \
	if ! [ -f tags ]; then \
	    ctags -R --c++-kinds=+p --fields=+iaS --extra=+q; \
	else \
		cp tags tags.$$$$; \
		sed -f temp_sed_file < tags.$$$$ > tags; \
		ctags --c++-kinds=+p --fields=+iaS --extra=+q -a $$tagfiles; \
		rm -r tags.$$$$; \
	fi; \
	rm -f temp_tag_file	temp_sed_file; \
	echo "--------   OK   --------"

# clean up obj and dep files
clean:
	@echo "clean up building files .. "
	@cd $(DEP_DIR); \
	rm -rf $(DEPEND)
	@cd $(OBJ_DIR); \
	rm -rf $(OBJECT)
	@cd $(TAG_DIR); \
	rm -rf $(TAGS)
	@echo "--------   OK   --------"

# clean up all files
purge:
	@echo -n "projects would be cleaned up; continue?[y/n]"
	@read confirm; \
	if  [ $$confirm = y ]; then \
	    echo "purge the project .. "; \
	    rm -rf $(DEP_DIR)*.d; \
	    rm -rf $(OBJ_DIR)*.o; \
	    rm -rf $(TAG_DIR)*.tag; \
	    rm -rf $(DIST_DIR)*; \
	    rm -rf $(SRC_DIR)*; \
	    rm -rf $(TEST_DIR)*; \
	    echo "--------   OK   --------"; \
	fi

# test target
show:
	@echo $(TAGS)
	
.PHONY: all rebld test dply pkg pkg_scan pkg_del libdp libdp_rst tpsrc tb \
        clean purge show tags
