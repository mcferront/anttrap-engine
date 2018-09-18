
ifdef _RELEASE
	CONFIG=_RELEASE
	DEBUGFLAGS = g0
   OPTIMIZATIONS = O3
else
	CONFIG=_DEBUG
	DEBUGFLAGS = g2
	OPTIMIZATIONS = O0
endif

CC=g++
CFLAGS=-c -Wall -D_REENTRANT -D$(CONFIG) -$(OPTIMIZATIONS) -$(DEBUGFLAGS) -DLINUX -I../Linux -I../ -I./


PCH=../EnginePch

SOURCES=../Socket.cpp ../RingBuffer.cpp ../MemoryStreams.cpp ../Channel.cpp ../ChannelSystem.cpp\
        ../StringPool.cpp ../TcpIpPipe.cpp ../TcpIpPipeListener.cpp ../Threads.cpp ../Debug.cpp ../PipeStreams.cpp\
        ../EnginePch.cpp ../Timer.cpp ../Entity.cpp ../Resource.cpp ../TouchEvent.cpp ../FileStreams.cpp\
        ../ResourceLoader.cpp  ../TouchObject.cpp ../FloatController.cpp ../ResourceMaps.cpp ../TouchWorld.cpp ../ResourceWorld.cpp\
        ../UtilityMath.cpp ../Serializer.cpp ../Linux/LinuxThread.cpp ../UtilityTime.cpp ../TcpIpBurst.cpp ../TcpIpStream.cpp ../Hub.cpp\
        ../Proxy.cpp

OBJECTS=$(SOURCES:.cpp=.o)
LIBRARY=Engine.a

all: $(SOURCES) $(LIBRARY)

$(LIBRARY): $(OBJECTS)
	ar -rfs $(LIBRARY) $(OBJECTS)

.cpp.o: $(PCH).pch
	$(CC) $(CFLAGS) $< -o $@

$(PCH).pch : $(PCH).h

clean:
	rm -rf ../*.o Engine.a
