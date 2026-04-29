all: test

clean:
	rm -f *.o *.s test compilateur tokeniser.cpp

tokeniser.cpp: tokeniser.l tokeniser.h
	flex++ -d -otokeniser.cpp tokeniser.l

tokeniser.o: tokeniser.cpp
	g++ -I/usr/include -c tokeniser.cpp

compilateur: compilateur.cpp tokeniser.o tokeniser.h
	g++ -ggdb -I/usr/include -o compilateur compilateur.cpp tokeniser.o

test: compilateur test.p
	./compilateur < test.p > test.s
	gcc -ggdb -no-pie -fno-pie test.s -o test
