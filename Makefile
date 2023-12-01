########################################################
# Variable definitions
########################################################
# C++ compiler
CXX      = g++

# C++ compiler flags
CXXFLAGS := -O3 -Wall -Werror -std=c++23 $(shell Magick++-config --cxxflags --cppflags)

# Linker: for C++ should be $(CXX)
LINK     := $(CXX)

# Linker flags. Usually none.
LDFLAGS  := $(shell Magick++-config --ldflags --libs)

# Library paths, prefaced with "-L". Usually none.
LDPATHS := 

# Libraries we're using, prefaced with "-l".
LDLIBS := -ltbb -lfmt -lfreeimageplus -fopenmp

# Executable names
EXEC_MAIN := main
EXEC_FREEPAR := freePar

#############################################################
# Rules
#   Rules have the form
#   target : prerequisites
#   	  recipe
#############################################################

# Add additional object files if you're using more than one
#   source file.

# Main target
all: $(EXEC_MAIN) $(EXEC_FREEPAR)

$(EXEC_MAIN) : main.o
	$(LINK) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(EXEC_FREEPAR) : freePar.o
	$(LINK) $(LDFLAGS) $^ $(LDLIBS) -o $@

# Add rules for each object file
# No recipes are typically needed
main.o : main.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

freePar.o : freePar.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

#############################################################

clean :
	@$(RM) $(EXEC_MAIN) $(EXEC_FREEPAR) a.out core
	@$(RM) *.o
	@$(RM) *.d
	@$(RM) *~
