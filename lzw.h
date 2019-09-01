#ifndef LZW_H
#define LZW_H

#include "includes.h"



template<typename In, typename Out>
class LZW {
public:

    LZW(long Level);
    
    void Code(In *&is, Out *&os, bool is_in_symb, bool is_out_symb = false); // is input/output symbolic or binary
    void Decode(In *&is, Out *&os, bool is_in_symb, bool is_out_symb);


protected:
    // changes for level in constructor 
    
    bool is_bin;
    size_t counter;
    size_t bufferSize;
    const unsigned int eob = 257; // end of buffer 
    char eob2 = 257;
    unsigned int next_code;
    unsigned int max_code; // change for level 
    
    std::unordered_map< std::basic_string<char>, unsigned int> StrToCodeDict;
    std::unordered_map<unsigned int,  std::basic_string<char>> CodeToStrDict;

    void InitDict();

};


template<typename In, typename Out>
LZW<In, Out>::LZW
(  long Level   )
{
    if (Level == 1){
        bufferSize = 100000;
    } else
    if (Level == 9) {
        bufferSize = 100000;
    } else {
        bufferSize = 100000;
    }


    
    counter = 0;
    next_code = 258;
    max_code = 4294967293;


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

    // added to make levels (end of buffer)
    StrToCodeDict.insert(std::make_pair("", eob));
    CodeToStrDict.insert(std::make_pair(eob, ""));

}



template<typename In, typename Out>
void LZW<In, Out>::Code(In *&is, Out *&os, bool is_in_symb, bool is_out_symb) {

    // remember - u int - 4 bytes - 32 bits

    // TODO :
    //    1 - redo << for bytes  
    

    std::vector<char> input(bufferSize);
    std::vector<unsigned int> output(bufferSize);
    unsigned int counter = 0;

    while (*is) {
        // why here INIT?? because buffered and we put eob
        InitDict();
        (*is).read(input.data(), input.capacity());
        unsigned int readSize = (*is).gcount(); 

        if (readSize) {
            
            std::string current_str = "";
            for(size_t i = 0; i < readSize; i++) {
                current_str += input[i];
                if (StrToCodeDict.find(current_str) == StrToCodeDict.end()) {
                    if (next_code <= std::numeric_limits<unsigned int>::max())
                        StrToCodeDict[current_str] = next_code++;

                    std::cout << "to dict : " << StrToCodeDict[current_str]  << " " << current_str << std::endl;
                    current_str.pop_back();
                    // put number to output buffer
                    output[counter++] = StrToCodeDict[current_str];
                    std::cout << "put: "<< StrToCodeDict[current_str] << " " << current_str<< std::endl;
                    
                    current_str = input[i];

                    if (counter == bufferSize) {
                        if (is_out_symb){
                            (*os) << output;
                        } else {
                            (*os).write(reinterpret_cast<char*>(&output), sizeof(unsigned int) * counter);
                        }
                        counter = 0;
                    }
                }
            }

            if ( current_str.size() ){
                std::cout << "Put: "<< StrToCodeDict[current_str] << " " << current_str << std::endl;
                if (is_out_symb){
                    (*os) << StrToCodeDict[current_str];
                } else {
                    (*os).write(reinterpret_cast<char*>(&StrToCodeDict[current_str]), sizeof(unsigned int));
                }
                
            }
            
            // put end of buffer
            if (is_out_symb) {
                (*os) << eob2;
            } else {
                (*os).write(&eob2, sizeof(char));
            }
  
        } 
    }// end while 

}


template<typename In, typename Out>
void LZW<In, Out>::Decode(In *&is, Out *&os, bool is_in_symb, bool is_out_symb) {
    
    
    std::vector<unsigned int> input(bufferSize);
    unsigned int buffSize = 0;
    InitDict();
    while(true) { // WARNING
        
        while (buffSize < bufferSize && (*is)){
            long long checking = 0;
            (*is) >> checking;
            if (checking == 0) break; // EOF 
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