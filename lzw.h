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

    const size_t bufferSize = 100000;//100000; // 10000
    size_t counter = 0;
    const unsigned int eob = 257; /* end of buffer */
    unsigned int next_code = 258;
    
    const unsigned int max_code = 4294967293; 
    
    std::unordered_map< std::basic_string<char>, unsigned int> StrToCodeDict;
    std::unordered_map<unsigned int,  std::basic_string<char>> CodeToStrDict;

    void InitDict();


};


template<typename In, typename Out>
LZW<In, Out>::LZW() {

}

template<typename In, typename Out>
void LZW<In, Out>::InitDict() {
    StrToCodeDict.clear();
    CodeToStrDict.clear();
    next_code = 258;
    counter = 0;
    
    for (unsigned int ch = 0; ch < 256; ch++) {

        auto str =  std::basic_string<char>(1, static_cast<char>(ch));
        StrToCodeDict.insert(std::make_pair(str, counter));
        CodeToStrDict.insert(std::make_pair(counter, str));

        counter++;
    }
    /* added to make levels (end of buffer */
    StrToCodeDict.insert(std::make_pair("", eob));
    CodeToStrDict.insert(std::make_pair(eob, ""));

}

template<typename In, typename Out>
void LZW<In, Out>::Code(In *&is, Out *&os) {

    // remember - u int - 4 bytes - 32 bits

    /* TODO :
        1 - redo << for bytes  */

    std::vector<char> input(bufferSize);

    while (*is) {
        InitDict();
        (*is).read(input.data(), input.capacity());
        unsigned int buffSize = (*is).gcount(); 

        if (buffSize) {
            
            std::string current_str = "";
            for(size_t i = 0; i < buffSize; i++) {
                current_str += input[i];
                if (StrToCodeDict.find(current_str) == StrToCodeDict.end()) {
                    if (next_code <= std::numeric_limits<unsigned int>::max())
                        StrToCodeDict[current_str] = next_code++;
                    current_str.erase(current_str.size()-1);
                    *os << StrToCodeDict[current_str];
                    //std::cout << "put: "<< StrToCodeDict[current_str] << " " << current_str<< std::endl;
                    
                    *os << " ";
                    current_str = input[i];
                }
            }

            if ( current_str.size() ){
                *os << StrToCodeDict[current_str] << " ";
                //std::cout << "Put: "<< StrToCodeDict[current_str] << " " << current_str << std::endl;
            }

            *os << eob << " ";
            //std::cout << "put: "<< eob << std::endl;

        } 
    }/* end while */

}


template<typename In, typename Out>
void LZW<In, Out>::Decode(In *&is, Out *&os) {
    
    
    std::vector<unsigned int> input(bufferSize);
    unsigned int buffSize = 0;
    InitDict();
    while(true) { // WARNING
        
        while (buffSize < bufferSize && (*is)){
            long long checking = 0;
            (*is) >> checking;
            if (checking == 0) break; /* EOF */
            input[buffSize] = checking;
            buffSize++;
        }

        if (buffSize == 0 && !(*is)) break;

        

        if (buffSize) {
            std::string prev_str = "";
            unsigned int code;
            bool flag = 0;
            size_t code_flag;

            for(size_t i = 0; i < buffSize; i++) {
                code = input[i];
                
                if (code == eob) {
                    for(size_t p = 0; p < buffSize-i-1; p++){
                        input[p] = input[p+i+1];
                    }
                    flag = 1;
                    code_flag = i;
                    
                    InitDict();
                    break;
                }

                if ( CodeToStrDict.find(code) == CodeToStrDict.end() )
                    CodeToStrDict[code] = prev_str + prev_str[0];
                //std::cout << "got: " << CodeToStrDict[code] << " " << code << std::endl;
                *os << CodeToStrDict[code];


                if ( prev_str.size() && next_code <= std::numeric_limits<unsigned int>::max() ){
                    std::string newstr = prev_str;
                    auto a = CodeToStrDict[code][0];
                    newstr += a;
                    CodeToStrDict.insert(std::make_pair(next_code++, newstr));
                    
                }
                prev_str = CodeToStrDict[code];
            } // end for

            if (flag) 
                buffSize = buffSize-code_flag-1;
            else 
                buffSize = 0;

        }  // end if
    }

}



#endif