ENABLE_DEBUG=-D DEBUG_ON
CCFLAGS=-Wall $(ENABLE_DEBUG)

SRC=src
INC=include

LIBS=-L/usr/lib -lboost_thread -lboost_system
LIBS_INC=/usr/include

INCLUDE=-I$(INC) -I$(LIBS_INC)


OBJ=Main.o Listener.o Network.o Peer.o Utils.o
EXE=node


$(EXE): $(OBJ)
	g++ -o $(EXE) $(LIBS) $(OBJ)

Main.o : $(SRC)/Main.cpp $(INC)/Config.hpp
Listener.o : $(SRC)/Listener.cpp $(INC)/Listener.hpp $(INC)/Config.hpp
Network.o : $(SRC)/Network.cpp $(INC)/Network.hpp $(INC)/Config.hpp
Peer.o : $(SRC)/Peer.cpp $(INC)/Peer.hpp $(INC)/Config.hpp
Utils.o : $(SRC)/Utils.cpp $(INC)/Utils.hpp $(INC)/Config.hpp


%.o : $(SRC)/%.cpp
	g++ -c $(CCFLAGS) $(INCLUDE) $<