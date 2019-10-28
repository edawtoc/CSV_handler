static: CsvHandler.o CsvHandlerExceptions.o
	ar rs target/libCsvHandler CsvHandler.o CsvHandlerExceptions.o && rm -f CsvHandler.o CsvHandlerExceptions.o

CsvHandler.o: src/CsvHandler.hpp src/CsvHandler.cpp
	g++ -c -Wall -std=c++11  -pedantic src/CsvHandler.cpp

CsvHandlerExceptions.o: src/CsvHandlerExceptions.hpp src/CsvHandlerExceptions.cpp
	g++ -c -Wall -std=c++11 -pedantic src/CsvHandlerExceptions.cpp

CsvHandler.exe: src/main.cpp src/CsvHandler.hpp
	g++ -Wall -std=c++11 -pedantic src/main.cpp -o target/CsvHandler.exe -static -I. -L. -ltarget/CsvHandler

clean:
	rm -f main.o CsvHandler.o CsvHandlerExceptions.o target/CsvHandler.exe target/libCsvHandler
