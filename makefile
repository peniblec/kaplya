ENABLE_DEBUG=-D DEBUG_ON
CCFLAGS=-Wall $(ENABLE_DEBUG)

LIBS=-L/usr/lib -lboost_thread -lboost_system
INCLUDE=-I/usr/include

OBJ=Main.o Listener.o Network.o Peer.o Utils.o
EXE=node


$(EXE): $(OBJ)
	g++ -o $(EXE) $(LIBS) $(OBJ)

Main.o : Main.cpp Config.hpp
Listener.o : Listener.cpp Listener.hpp Config.hpp
Network.o : Network.cpp Network.hpp Config.hpp
Peer.o : Peer.cpp Peer.hpp Config.hpp
Utils.o : Utils.cpp Utils.hpp Config.hpp


%.o : %.cpp
	g++ -c $(CCFLAGS) $(INCLUDE) $<