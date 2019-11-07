#ifndef LZW_H
#define LZW_H

#include "includes.h"

/*
 Coding is awful check == end() in decode
*/


template<typename In, typename Out>
class LZW {
public:

    LZW(int level);
    
    void Code(In *&is, Out *&os, bool is_in_symb, bool is_out_symb = false); // is io symbolic or binary
    void Decode(In *&is, Out *&os, bool is_in_symb, bool is_out_symb);

protected:

    // changes for level in constructor     
    size_t bufferSize;
    unsigned short int eob = 256; // end of buffer 
    unsigned short int next_code;
    unsigned short int const_next_code = 257;
    unsigned short int max_code; // change for level 
    
    // both unordered map to uniform code
    std::unordered_map< std::basic_string<char>, unsigned short int> StrToCodeDict;
    std::unordered_map<unsigned short int,  std::basic_string<char>> CodeToStrDict;


    void InitDict();


};


template<typename In, typename Out>
LZW<In, Out>::LZW (int level){


    bufferSize = 140000;

    next_code = const_next_code;
    max_code = std::numeric_limits<unsigned short int>::max();
}

template<typename In, typename Out>
void LZW<In, Out>::InitDict() {

    StrToCodeDict.clear();
    CodeToStrDict.clear();
    
    for (unsigned short int ch = 0; ch < eob; ch++) {

        auto str =  std::basic_string<char>(1, static_cast<char>(ch)); // ??
        StrToCodeDict.insert(std::make_pair(str, ch));
        CodeToStrDict.insert(std::make_pair(ch, str));
    }

    StrToCodeDict.insert(std::make_pair("", eob));
    CodeToStrDict.insert(std::make_pair(eob, ""));

}



template<typename In, typename Out>
void LZW<In, Out>::Code(In *&is, Out *&os, bool is_in_symb, bool is_out_symb) {


    std::vector<char> input(bufferSize);
    std::vector<unsigned short int> output;

    while (*is) {
        // why here INIT ? because buffered and we put eob
        InitDict();
        (*is).read(input.data(), input.capacity());
        unsigned int readSize = (*is).gcount(); 
        input.resize(readSize);
        next_code = const_next_code;

        if (readSize) {
            
            std::string current_str = "";
            for(size_t i = 0; i < readSize; i++) {

                current_str += input[i];

                // if we cant find how to code string we append it to dict
                // if found do noting! -> continue

                if (StrToCodeDict.find(current_str) == StrToCodeDict.end()) {
                    if (next_code < max_code) {
                        StrToCodeDict[current_str] = next_code++;
                    } else {
                        // I think we`ll never get here

                        // upd: we will never get here because of max_code changed to const

                        std::cout << "PANIC!!!" << std::endl;
                        exit(0);

                    }
                

                    //std::cout << "d:" << StrToCodeDict[current_str]  << ":" << current_str << std::endl;
                    current_str.pop_back();
                    
                    // put number to output buffer
                    output.push_back(StrToCodeDict[current_str]);
                    //std::cout << "p:"<< StrToCodeDict[current_str] << ":" << current_str<< std::endl;
                    
                    current_str = input[i];

                    // it is time to flush buffer
                    if (output.size() == bufferSize) {
                        
                        if (is_out_symb){
                            for(unsigned int i = 0; i < output.size(); i++)
                                (*os) << output[i];
                            output.clear();

                        } else {
                            (*os).write(reinterpret_cast<char*>(&output[0]), sizeof(unsigned short int) * output.size());
                            output.clear();
                        }
                    }
                }
            }// end for i in readSize

            // Now we at the end of input buffer
            // time to flush buffer and 'end of buffer'

            if ( current_str.size() ){
                //std::cout << "P:"<< StrToCodeDict[current_str] << ":" << current_str << std::endl;
                if (!is_out_symb)
                    output.push_back(StrToCodeDict[current_str]);
                else 
                    (*os) << StrToCodeDict[current_str];
            }

            if (!is_out_symb){
                output.push_back(eob);
            }
            // if symbol symbolic we put eob later

            // flushing 
            if (is_out_symb){
                for(unsigned int i = 0; i < output.size(); i++)
                    (*os) << output[i];
                output.clear();
                (*os) << eob;

            } else {
                (*os).write(reinterpret_cast<char*>(&output[0]), sizeof(unsigned short int)*output.size());
                output.clear();
            }
  
        } // end if(readSize)

    }// end while 
   

}


template<typename In, typename Out>
void LZW<In, Out>::Decode(In *&is, Out *&os, bool is_in_symb, bool is_out_symb) {
    

    std::vector<std::basic_string<char>> output;
    std::vector<unsigned short int> input;

    InitDict();
    std::string prev_str = "";
    size_t i;
    while(*is) {
        input.clear();
        char * inbuf = new char [bufferSize*2];
        (*is).read(inbuf, bufferSize*2);
        unsigned int readSize = (*is).gcount();

        // makeing unsigned shorts from bytes
        for(unsigned int j = 0; j < readSize; j+=2) {
            unsigned short int Cha;
            Cha = inbuf[j] & 0xff;
            Cha |= (inbuf[j+1] & 0xff) << 8;
            input.push_back(Cha);
        } 

        if (readSize) {

            
            unsigned int code; 
            
            for(i = 0; i < input.size(); i++) {
                
                code = input[i];
                
                if (code == eob) {
                    InitDict();
                    prev_str = ""; 
                    next_code = const_next_code;
                    continue;
                    // we break from 'for' and make 'is.read()'
                }

                if ( CodeToStrDict.find(code) == CodeToStrDict.end() ){
                    CodeToStrDict[code] = prev_str + prev_str[0];
                    //std::cout << "d:" << code  << ":" << CodeToStrDict[code] << std::endl;

                }
                //std::cout << "p:" << code << ":" <<  CodeToStrDict[code] << std::endl;
                output.push_back(CodeToStrDict[code]);

                // it is time to flush buffer
                if (output.size() == bufferSize) {
                    for(unsigned int i = 0; i < output.size(); i++)
                        (*os) << output[i];
                    output.clear();
                }


                if ( prev_str.size() && (next_code <= max_code) ){
                    std::string newstr = prev_str;
                    auto a = CodeToStrDict[code][0];
                    newstr += a;
                    CodeToStrDict.insert(std::make_pair(next_code++, newstr));
                    //std::cout << "D:" << next_code -1  << ":" << CodeToStrDict[next_code-1] << std::endl;
                    
                }
                prev_str = CodeToStrDict[code];
            } // end for
            
            
        }  // end if readSize
        
        for(unsigned int i = 0; i < output.size(); i++)
            (*os) << output[i];
        output.clear();
    }

}



#endif