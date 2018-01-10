ZLIB		= zlib-1.2.8
BOOST 		= /usr/src/boost_1_66_0

INCLUDES	= -I$(ZLIB) -I$(BOOST)
TARGET		= ecrp

CPPFLAGS	= -O3 $(INCLUDES) -pedantic -Wall -std=c++0x -D__STDC_LIMIT_MACROS -DBOOST_SYSTEM_NO_DEPRECATED
LDFLAGS		= -L$(ZLIB) -L$/lib -lstdc++ -lz -lboost_system -lboost_filesystem -lboost_thread

.SUFFIXES: .cpp .o

SOURCES = \
compression.cpp \
error.cpp \
ecrp.cpp \
paging.cpp \
point.cpp \
processor.cpp \
registry.cpp \
request.cpp \
response.cpp \
server.cpp \
session.cpp \
tree.cpp \
utils.cpp \

OBJECTS = ${SOURCES:.cpp=.o}

all: $(OBJECTS)
	g++ $(OBJECTS) $(LDFLAGS) -o $(TARGET)

%.o: %.cpp 
	g++ $(CPPFLAGS) -o $@ -c $<

clean:	
	/bin/rm -rf $(TARGETS) $(OBJECTS) $(TARGET)
