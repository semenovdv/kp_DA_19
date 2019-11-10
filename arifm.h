#ifndef ARIFM_H
#define ARIFM_H

#include "includes.h"
#include <math.h>

template<typename In, typename Out>
class ARIFM {
public:

    ARIFM(int level);
    
    void Code(In *&is, Out *&os, bool is_in_symb, bool is_out_symb);
    void Decode(In *&is, Out *&os, bool is_in_symb, bool is_out_symb);


protected:
    
    // сколько битов будет использовано для кодирования
    const unsigned int Code_bits = 32; 

    const unsigned int No_of_chars = 256;
    const unsigned int EOF_sybol = No_of_chars + 1;
    const unsigned int No_of_symbols = No_of_chars + 1;
    

    // swap zone for updating tables freq and cum-freq
    std::vector<unsigned long> char_to_index = std::vector<unsigned long>(No_of_chars); 
    std::vector<unsigned char> index_to_char = std::vector<unsigned char>(No_of_symbols+1);

    // main tables of probability
    std::vector<unsigned long> frequency = std::vector<unsigned long>(No_of_symbols+1); 
    std::vector<unsigned long> cum_frequency = std::vector<unsigned long>(No_of_symbols+1);

    // io for Code
    std::vector<unsigned char> input_code;

    // io for Decode
    std::vector<unsigned short int> input_decode;
    std::vector<unsigned char> output_decode;

    unsigned long bufferSize;
    
    /*
        {0, 1, ..., 256, end = 257}
        end is special
    */
    unsigned long l; // left bound
    unsigned long h; // right bound


    unsigned long first_qtr;
    unsigned long half;
    unsigned long third_qtr;

    unsigned long Max_frequency;

    

    void CodeSymbol(unsigned long input_ch, Out *&os);
    unsigned long long DecodeSymbol(In *&is);

    void Update_model(unsigned long input_ch);

    void bit_plus_follow(long bit, Out *&os);
    
    unsigned short int buffer = 0; // to store bits 
    unsigned long long buffercounter = 0; // for next values to dtore in buffer

    int input_bit(In *&is); 
    void output_bit(int bit, Out *&os); 

    unsigned long long bits_to_go;
    unsigned long long ullbufferbits;
    unsigned long long bits_to_follow = 0; // to form output bits
    unsigned long long garbage_bits = 0;
    unsigned long value = 0;

};



template<typename In, typename Out>
ARIFM<In, Out>::ARIFM(int level) {


    bufferSize = 60000;
    input_code.resize(bufferSize);
    input_decode.resize(bufferSize);

    l = 0;
    h = (1UL << Code_bits) - 1; //4294967295 if 32   // 2**N - 1 
    Max_frequency = (1UL << (Code_bits-2)) - 1;

    first_qtr = (h/4 + 1);
    half = 2*first_qtr;
    third_qtr = 3*first_qtr;
    

    // tables of recoding symbols
    for (unsigned int i = 0; i < No_of_chars; i++) { 
        char_to_index[i] = i+1;     
        index_to_char[i+1] = i;
    }

    int cf = No_of_symbols;
    for(unsigned int i = 0; i <= No_of_symbols; i++) {
       cum_frequency[i] = cf--;
       frequency[i] = 1;
    }
   frequency[0]=0;
    
}

template<typename In, typename Out>
void ARIFM<In, Out>::Code(In *&is, Out *&os, bool is_in_symb, bool is_out_symb) {

    bits_to_go = 16;
    buffer = 0;
    bits_to_follow = 0;


    while(*is) {
        (*is).read(reinterpret_cast<char*>(&input_code[0]), bufferSize*sizeof(unsigned char));
        unsigned long long readSize = (*is).gcount(); 

        input_code.resize(readSize);

        if (readSize) {

                /* ОБРАТНЫЙ ВЫВОД ОЧЕНЬ ВАЖНО !!!
                unsigned int a = ((unsigned int)input[i+1] << 8) | input[i];
                */

            for(unsigned long i = 0; i < input_code.size(); i++) {
                
                // std::cout << "input_ch: " << (int)input_code[i] << std::endl;
                unsigned long input_ch = char_to_index[input_code[i]];
                
                // std::cout << "symbol to code " << input_ch << std::endl;
        
                CodeSymbol(input_ch, os);
                Update_model(input_ch);
                
            } // end for   
        } // end if 
    } // end while

    CodeSymbol(EOF_sybol, os);

    // done encoding
    bits_to_follow++;
    if (l < first_qtr) bit_plus_follow(0, os);
    else bit_plus_follow(1, os);
    
    // flush buffer
    buffer = buffer >> bits_to_go;
    (*os).write(reinterpret_cast<char*>(&buffer), sizeof(unsigned short int));
    // std::cout << "put |" << (unsigned long)buffer << "| in file" << std::endl;
}


template<typename In, typename Out>
void ARIFM<In, Out>::CodeSymbol(unsigned long input_ch, Out *&os) {
    unsigned long tmp = l;
    l = tmp + (((h-tmp+1)*cum_frequency[input_ch])/cum_frequency[0]);
    h = tmp + (((h-tmp+1)*cum_frequency[input_ch-1])/cum_frequency[0]) - 1;
    // std::cout << "L " << l << ", H " << h << std::endl;

    for(;;) {
        if (h < half) {
            bit_plus_follow(0, os);
        } else if (l >= half) {
            bit_plus_follow(1, os);
            l -= half;
            h -= half;
        } else if (l >= first_qtr && h < third_qtr) {
            bits_to_follow += 1;
            l -= first_qtr;
            h -= first_qtr;
        } else 
            break;
        l = l*2;
        h = h*2+1;
    }
              
}



template<typename In, typename Out>
void ARIFM<In, Out>::Decode(In *&is, Out *&os, bool is_in_symb, bool is_out_symb) {
    
    bits_to_go = 0;
    garbage_bits = 0;
    value = 0;

    while(*is) {
        (*is).read(reinterpret_cast<char*>(&input_decode[0]), bufferSize*sizeof(unsigned short int));
        unsigned long long readSize = (*is).gcount(); 
        // divide by 2 bcz unsigned short 2 bytes
        input_decode.resize(readSize/2);
        buffercounter = 0;
        
        value = 0;
        for (unsigned int i = 0; i < Code_bits; i++){
            value = 2*value + input_bit(is);
        }

        
        if (readSize) {
                /* ОБРАТНЫЙ ВЫВОД ОЧЕНЬ ВАЖНО !!!
                unsigned int a = ((unsigned int)input[i+1] << 8) | input[i];
                */

            for(;;) {
                int input_ch = DecodeSymbol(is);
                // std::cout << "input_ch: " << input_ch << std::endl;
                if ((unsigned int)input_ch == EOF_sybol) break;
                unsigned char ch = index_to_char[input_ch];
                (*os).write(reinterpret_cast<char*>(&ch), sizeof(unsigned char));
                output_decode.push_back(ch);
                // std::cout << "put |" << (unsigned long)ch << "| in file" << std::endl;

                Update_model(input_ch);
                
            } // end for   
        } // end if readSize
    } // wnd while

}



template<typename In, typename Out>
unsigned long long ARIFM<In, Out>::DecodeSymbol(In *&is) {

    unsigned long long cum = (((value - l) + 1) * cum_frequency[0] - 1)/((h - l) + 1);
    unsigned long long i = 1; 
    while (cum_frequency[i] > cum) i++;

    unsigned long long tmp = l;
    l = tmp + ((h - tmp + 1)*cum_frequency[i])/cum_frequency[0];
    h = tmp + ((h - tmp + 1)*cum_frequency[i-1])/cum_frequency[0] - 1;

    for(;;) {
        if (h < half) {
            // do nothing
        } else if (l >= half) {
            value -= half;
            l -= half;
            h -= half;
        } else if (l >= first_qtr && h < third_qtr) {
            value -= first_qtr;
            l -= first_qtr;
            h -= first_qtr;
        } else break;

        l = 2*l;
        h = 2*h+1;
        value = 2*value + input_bit(is); 
    }

    return i;
}



template<typename In, typename Out>
void ARIFM<In, Out>::Update_model(unsigned long symbol) {

    // updating model
    int index = 0;
    // now cf[0] is max we divide by 2 all freqs
    if (cum_frequency[0] >= Max_frequency) { 
        int cum = 0;
        for (index = No_of_symbols; index >= 0; index--) {
            frequency[index] = (frequency[index]+1)/2;
            cum_frequency[index] = cum;
            cum += frequency[index];
        }
    }
    
    // finding suitable index
    for (index = symbol; frequency[index]==frequency[index-1]; index--);
    
    // if we found
    // some MAGIC, pls dont kill me, teacher
    if ((unsigned int)index < symbol) {
        unsigned long ch_i = index_to_char[index];
        unsigned long ch_symbol = index_to_char[symbol];
        index_to_char[index] = ch_symbol;
        index_to_char[symbol] = ch_i;
        char_to_index[ch_i] = symbol;
        char_to_index[ch_symbol] = index;
    } 

    // updating tables 
    frequency[index]++; 
    while(index > 0) {
        index -= 1;
        cum_frequency[index] += 1;
    }
}



template<typename In, typename Out>
int ARIFM<In, Out>::input_bit(In *&is) {
    
    int t; // to return

    if (bits_to_go == 0) {
        if (buffercounter == bufferSize){
            (*is).read(reinterpret_cast<char*>(&input_decode[0]), bufferSize*sizeof(unsigned short int));
            unsigned long long readSize = (*is).gcount(); 
            // divide by 2 bcz unsigned short 2 bytes
            input_decode.resize(readSize/2);
            buffercounter = 0;
        }
        // get new value from buffer
        buffer = input_decode[buffercounter++];

        if (static_cast<long long>(buffer) == EOF) { 
            garbage_bits += 1;
            if (garbage_bits > Code_bits - 2) {
                std::cerr << "Ошибка в сжатом файле" << std::endl;;
                return 0;
            }
        }
        
        bits_to_go = 16;
    }
    t = buffer & 1;
    buffer >>= 1;
    bits_to_go -= 1;
    return t;
    
}



template<typename In, typename Out>
void ARIFM<In, Out>::bit_plus_follow(long bit, Out *&os) {
    output_bit(bit, os);
    while(bits_to_follow > 0) {
        output_bit(!bit, os);
        bits_to_follow--;
    }
}

template<typename In, typename Out>
void ARIFM<In, Out>::output_bit(int bit, Out *&os) {
    //std::cout << "Out: " << bit << std::endl;
    buffer >>= 1;
    if(bit)
        buffer |= 0x8000;
    /*std::cout << "buffer ";
    std::cout << std::bitset<64>(buffer);
    std::cout << std::endl;
    */
    bits_to_go -= 1;
    if (bits_to_go == 0) {
        
        (*os).write(reinterpret_cast<char*>(&buffer), sizeof(unsigned short int));
        // std::cout << "put |" << (unsigned long)buffer << "| in file" << std::endl;
 
        bits_to_go = 16;
    }
}


#endif