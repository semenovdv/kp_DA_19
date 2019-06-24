#ifndef LZW_H
#define LZW_H

#include "includes.h"


template<typename In, typename Out>
class LZW {
public:

    LZW();
    
    void Code(In *&is, Out *&os);
    void Decode(In *&is, Out *&os);


protected:

    const size_t bufferSize = 100000;
    size_t counter = 0;
    unsigned int next_code = 257;
    
    const unsigned int max_code = 4294967293; 
    const unsigned int eob = 4294967294; /* end of buffer */
    
    std::unordered_map< std::basic_string<char>, unsigned int> StrToCodeDict;
    std::unordered_map<unsigned int,  std::basic_string<char>> CodeToStrDict;

    void InitDict();


};


template<typename In, typename Out>
LZW<In, Out>::LZW() {

    InitDict();
}

template<typename In, typename Out>
void LZW<In, Out>::InitDict() {
    
    for (unsigned int ch = 0; ch < 256; ch++) {

        auto str =  std::basic_string<char>(1, static_cast<char>(ch));
        StrToCodeDict.insert(std::make_pair(str, counter));
        CodeToStrDict.insert(std::make_pair(counter, str));

        counter++;
    }

}

template<typename In, typename Out>
void LZW<In, Out>::Code(In *&is, Out *&os) {

    // remember - u int - 4 bytes - 32 bits

    /* TODO :
        1 - redo << for bytes  */

    std::vector<char> input(bufferSize);

    while (*is) {
        (*is).read(input.data(), input.capacity());
        unsigned int buffSize = (*is).gcount();    
        std::cout << buffSize << std::endl;
        
        int a;
        std::cin >> a;  

        if (buffSize) {
            std::string current_str = "";
            for(size_t i = 0; i < buffSize; i++) {
                current_str += input[i];
                if (StrToCodeDict.find(current_str) == StrToCodeDict.end()) {
                    if (next_code <= std::numeric_limits<unsigned int>::max())
                        StrToCodeDict[current_str] = next_code++;
                    current_str.erase(current_str.size()-1);
                    *os << StrToCodeDict[current_str];
                    std::cout << current_str << std::endl;
                    
                    *os << " ";
                    current_str = input[i];
                }
            }

            if ( current_str.size() )
            *os << StrToCodeDict[current_str];
            

        } 
    }/* end while */

}


template<typename In, typename Out>
void LZW<In, Out>::Decode(In *&is, Out *&os) {
    
    //input_code_stream<In> in(is);
    //output_symbol_stream<Out> out(os);

    std::string prevStr = "";
    unsigned int code;
    
    while(*is >> code) {
        //std::cout << code << ": ";

        if ( CodeToStrDict.find(code) == CodeToStrDict.end() )
            CodeToStrDict[code] = prevStr + prevStr[0];
        *os << CodeToStrDict[code];
        //std::cout <<code << ": " << CodeToStrDict[code] << std::endl;

        if ( prevStr.size() && next_code <= std::numeric_limits<unsigned int>::max() ){
            std::string newstr = prevStr;
            auto a = CodeToStrDict[code][0];
            newstr += a;
            //std::cout << newstr << std::endl;
            //std::cout << next_code+1 << std::endl;
            CodeToStrDict.insert(std::make_pair(next_code++, newstr));
            //std::cout << "Put: " << CodeToStrDict[next_code-1] << std::endl;
        }
        prevStr = CodeToStrDict[code];
        
    }
}



#endif