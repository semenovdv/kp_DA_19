gzip: main.cpp zip.h lzw.h includes.h
	g++ -std=c++17 -pedantic -Wall -Werror -Wno-long-long -o gzip -g  main.cpp -lstdc++fs
