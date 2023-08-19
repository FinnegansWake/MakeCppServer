server: util.o server.o
	g++ -o day2server util.o server.o
client: util.o client.o
	g++ -o day2client util.o client.o
util.o: util.cc util.h
	g++ -c util.cc
server.o: server.cc
	g++ -c server.cc
client.o: client.cc
	g++ -c client.cc
.PHONY: clean
clean:
	-rm server.o client.o day2server day2client util.o
