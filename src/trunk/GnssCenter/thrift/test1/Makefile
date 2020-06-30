
BOOST_DIR  = /usr/include/boost
THRIFT_DIR = /usr/local/include/thrift
LIB_DIR    = /usr/local/lib
DEFS       = -DHAVE_INTTYPES_H -DHAVE_NETINET_IN_H

INC        = -I$(THRIFT_DIR) -I$(BOOST_DIR)

GEN_SRC    = ./gen-cpp/myService.cpp \
             ./gen-cpp/myservice_types.cpp ./gen-cpp/myservice_constants.cpp

GEN_OBJ    = $(patsubst %.cpp,%.o, $(GEN_SRC))


.PHONY: all clean

all: gen-cpp server client

gen-cpp: myservice.thrift
	thrift -r -gen cpp myservice.thrift

%.o: %.cpp
	$(CXX) -Wall $(DEFS) $(INC) -c $< -o $@

server: server.o $(GEN_OBJ)
	$(CXX) $^ -o $@ -L$(LIB_DIR) -lthrift 

client: client.o $(GEN_OBJ)
	$(CXX) $^ -o $@ -L$(LIB_DIR) -lthrift 

clean:
	$(RM) *.o server client
	$(RM) -R gen-cpp