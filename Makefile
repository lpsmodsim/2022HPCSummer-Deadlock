# Tell Make that these are NOT files, just targets
.PHONY: all install test uninstall clean sst-info sst-help viz_makefile viz_dot black mypy help 

# shortcut for running anything inside the singularity container
CONTAINER=/usr/local/bin/additions.sif
SINGULARITY=singularity exec $(CONTAINER)

# SST environment variables (gathered from the singularity container)
CXX=g++
CXXFLAGS=-std=c++1y -D__STDC_FORMAT_MACROS -fPIC -DHAVE_CONFIG_H -I/opt/SST/11.1.0/include
LDFLAGS =-shared -fno-common -Wl,-undefined -Wl,dynamic_lookup

# Grab all the .cc files, put objs and depends in the .build folder
SRC=$(wildcard *.cc)
OBJ=$(SRC:%.cc=.build/%.o)
DEP=$(OBJ:%.o=%.d)

# Name of the library we will be working with
PACKAGE=deadlock

# Simply typing "make" calls this by default, so everything gets built and installed
all: test

# make sure the sstsimulator.conf exists
# You shouldn't call this target directly, it gets called by other targets
~/.sst/sstsimulator.conf:
	mkdir -p ~/.sst
	touch ~/.sst/sstsimulator.conf

# Use the dependencies when compiling to check for header file changes
# You shouldn't call this target directly, it gets called by other targets
-include $(DEP)
.build/%.o: %.cc
	mkdir -p $(@D)
	$(SINGULARITY) $(CXX) $(CXXFLAGS) -MMD -c $< -o $@

# Link all the objects to create the library
# You shouldn't call this target directly, it gets called by other targets
lib$(PACKAGE).so: $(OBJ)
	$(SINGULARITY) $(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

# User callable targets below:

# Register the model with SST
install: $(CONTAINER) ~/.sst/sstsimulator.conf lib$(PACKAGE).so
	$(SINGULARITY) sst-register $(PACKAGE) $(PACKAGE)_LIBDIR=$(CURDIR)

# Run the tests for the model
test: $(CONTAINER) install black mypy
	$(SINGULARITY) sst tests/$(PACKAGE).py

# Unregister the model with SST
uninstall: $(CONTAINER) ~/.sst/sstsimulator.conf
	$(SINGULARITY) sst-register -u $(PACKAGE)

# Remove the build files and the library
clean: uninstall
	rm -rf .build *.so

sst-info: $(CONTAINER)
	$(SINGULARITY) sst-info $(arg)

sst-help: $(CONTAINER)
	$(SINGULARITY) sst --help

viz_makefile: $(CONTAINER)
	$(SINGULARITY) makefile2dot --output $(PACKAGE)makefile.dot
	$(SINGULARITY) dot -Tpng $(PACKAGE)makefile.dot > Makefile_viz.png

# Run the tests for the model and output a dot file which is converted to a png file.
viz_dot: $(CONTAINER) install
	$(SINGULARITY) sst tests/$(PACKAGE).py --output-dot=$(PACKAGE).dot --dot-verbosity=6
	$(SINGULARITY) dot -Tpng $(PACKAGE).dot > $(PACKAGE).png

black: $(CONTAINER)
	$(SINGULARITY) black tests/*.py

mypy: $(CONTAINER)
	$(SINGULARITY) mypy tests/*.py

help:
	@echo "Target     | Description"
	@echo "-----------+-------------------------------------------------------"
	@echo "install    | Builds all .cc files into a library lib$(PACKAGE).so,"
	@echo "           |  then registers the package with SST"
	@echo "           |"
	@echo "test       | Runs tests"
	@echo "           |"
	@echo "uninstall  | Un-registers the package with SST"
	@echo "           |"
	@echo "clean      | Cleans up the .build folder (.o and .d files) and"
	@echo "           |  removes the library .so file"
	@echo "           |"
	@echo "sst-info   | Runs sst-info to show what packages SST knows of."
	@echo "           |  You can also pass an argument to this target to only"
	@echo "           |  ask sst-info for a specific package or model."
	@echo "           |  For example: make sst-info arg=$(PACKAGE)"
	@echo "           |  For example: make sst-info arg=merlin.hr_router"
	@echo "           |"
	@echo "sst-help   | Runs sst --help to show available options when running"
	@echo "           |  sst (run modes, output graphs, threads, etc.)"
