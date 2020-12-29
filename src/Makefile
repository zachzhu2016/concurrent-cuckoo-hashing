BITS  = 64
ODIR  = obj64
tmp  := $(shell mkdir -p $(ODIR))

CXX      = g++
CXXFLAGS = -MMD -ggdb -O3 -std=gnu++17 -m$(BITS) -fgnu-tm 
LDFLAGS	 = -m$(BITS) -lpthread -lrt -fgnu-tm

CXXFILES = hashset
TARGET = $(ODIR)/hashset $(ODIR)/

OFILES = $(patsubst %, $(ODIR)/%.o, $(CXXFILES))
DFILES = $(patsubst %.o, %.d, $(OFILES))

all: $(TARGET)

$(ODIR)/%.o: %.cc
	@echo [CXX] $< "-->" $@
	@$(CXX) $(CXXFLAGS) -c -o $@ $<

$(TARGET): $(OFILES)
	@echo [LD] $^ "-->" $@
	@$(CXX) -o $@ $^ $(LDFLAGS)

clean: 
	@echo cleaning up...
	@rm -rf $(ODIR)

.PHONY: all clean
-include $(DFILES)

