########################################################
# Variable definitions
########################################################
# C++ compiler
CXX      = g++
#CXX := clang++
#CXX := mpicxx

# C++ compiler flags
# Use this first configuration for debugging
#CXXFLAGS := -ggdb -Wall -std=c++23
# Use the following  configuration for release
# CXXFLAGS := -O3 -Wall -Werror -std=c++23
CXXFLAGS := -O3 -Wall -Werror -std=c++23 $(shell Magick++-config --cxxflags --cppflags)

# Linker: for C++ should be $(CXX)
LINK     := $(CXX)

# Linker flags. Usually none.
LDFLAGS  := $(shell Magick++-config --ldflags --libs)

# Library paths, prefaced with "-L". Usually none.
LDPATHS := 

# Libraries we're using, prefaced with "-l".
#LDLIBS := -lpthread
LDLIBS := -ltbb -lfmt -lfreeimageplus

# Executable name. Needs to be the basename of your driver
#   file. I.e., your driver must be named $(EXEC).cc
EXEC := main

#############################################################
# Rules
#   Rules have the form
#   target : prerequisites
#   	  recipe
#############################################################

# Add additional object files if you're using more than one
#   source file.
$(EXEC) : $(EXEC).o
	$(LINK) $(LDFLAGS) $^ $(LDLIBS) -o $@

# Add rules for each object file
# No recipes are typically needed
main.o : main.cc

#############################################################

clean :
	@$(RM) $(EXEC) a.out core
	@$(RM) *.o
	@$(RM) *.d
	@$(RM) *~ 

#############################################################