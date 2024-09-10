CXX = g++
CXXFLAGS = -Wall -g -Werror -Wno-error=unused-variable
LDLIBS = -lstdc++ -pthread

# Portul pe care asculta serverul
PORT = 12345

# Adresa IP a serverului
IP_SERVER = 192.168.0.2

all: server subscriber

common.o: common.cpp common.hpp
	$(CXX) $(CXXFLAGS) -c $<

# Compileaza server.cpp si leaga cu common.o
server: server.cpp common.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDLIBS)

# Compileaza subscriber.cpp si leaga cu common.o
subscriber: subscriber.cpp common.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDLIBS)

.PHONY: clean run_server run_subscriber

# Ruleaza serverul
run_server: server
	./server $(IP_SERVER) $(PORT)

# Ruleaza subscriberul
run_subscriber: subscriber
	./subscriber $(IP_SERVER) $(PORT)

clean:
	rm -rf server subscriber *.o
