
SRC_DIR  = src
BIN_DIR  = bin
BIN_NAME = run_picoscope
BIN_EXT  = .exe

INCLUDES = -I./src -I./include
LIBS = -L./lib -lps4000 -lps6000

ifeq ($(OS),windows)

CXX = g++
RM  = del
PATH_SEP = "\\"

else

CXX = i386-mingw32-g++
RM  = rm
PATH_SEP = /

endif


all :
	$(CXX) $(SRC_DIR)/run_picoscope.cpp $(SRC_DIR)/picoscope.cpp $(SRC_DIR)/measurement.cpp $(SRC_DIR)/channel.cpp $(INCLUDES) $(LIBS) -o $(BIN_DIR)/$(BIN_NAME)$(BIN_EXT)
	# $(CXX) -c $(SRC_DIR)/picoscope.cpp
#	$(CXX) $(SRC_DIR)/empty.cpp         $(SRC_DIR)/picoscope.cpp $(INCLUDES) $(LIBS) -o $(BIN_DIR)/$(BIN_NAME)$(BIN_EXT)

clean :
	$(RM) $(BIN_DIR)$(PATH_SEP)$(BIN_NAME)$(BIN_EXT)
