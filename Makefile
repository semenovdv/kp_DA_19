gzip: main.cpp zip.h lzw.h includes.h arifm.h 
	g++ -std=c++17 -pedantic -Wall -Wno-long-long -o gzip -g  main.cpp -lstdc++fs -lm
