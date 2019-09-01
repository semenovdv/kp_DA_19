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
    const int num = 13;

    void CodeSymb(int i, Out *&os);
    void SendBit(bool bit, Out *&os);

    std::vector<unsigned long> frequency; // char to uint
    std::vector<unsigned long> cum_frequency;
    std::vector<unsigned long> buffer;
    unsigned long bufferSize;
    int countbits = 63;
    /*
        {zero, '1','2','3','4','5','6','7','8','9','0',' ', end}
        ' ' is special
        end is special
    */
    unsigned long l;
    unsigned long h;
    unsigned long counter;
    unsigned int N; // do we need it?

    const unsigned long leftmost = (1UL<<63);
    const unsigned long Nminus2 = (1UL<<62);
    

};

/*
    TODO:
        Redo the system from 0-9 to 0-maxint
*/

template<typename In, typename Out>
ARIFM<In, Out>::ARIFM(int level) {

    if (level == 1){
        bufferSize = 100000;
    } else { /* 9 */
        bufferSize = 100000;
    }

    buffer.resize(bufferSize);

    counter = 0;
    l = 0;
    h = std::numeric_limits<unsigned long>::max(); // 2**N - 1 //change for level?
    N = 64;
    frequency.resize(num);
    cum_frequency.resize(num);
    /*
        Fiiling tables
        0 - common 
        1~11 - for 0~9
        12 - EOF
    */
    int co = 12;
    cum_frequency[0] = co--;
    for (int i = 1; i < num; i++){
        frequency[i] = 1;
        cum_frequency[i] = co--;
    }

}

template<typename In, typename Out>
void ARIFM<In, Out>::Code(In *&is, Out *&os, bool is_in_symb, bool is_out_symb) {

    char c;
    unsigned int i;

    if ( !is.get(c) )
        return false;
    i = c & 0xff;
    if ( !is.get(c) )
        return false;
    i |= (c & 0xff) << 8;
    if ( i == eob )
        return false;
    else
        return true;
    }

}

template<typename In, typename Out>
void ARIFM<In, Out>::Decode(In *&is, Out *&os, bool is_in_symb, bool is_out_symb){
    char a;
    while ((*is).get(a)) {
        std::cout << a;
        (*os) << a;
    }
}

/*
template<typename In, typename Out>
void ARIFM<In, Out>::Code(In *&is, Out *&os) {

    for(int q = 0; q < num; q++){
            std::cout << frequency[q] << " ";
        }
        std::cout << std::endl;
        for(int q = 0; q < num; q++){
            std::cout << cum_frequency[q] << " ";
        }
        std::cout << std::endl;

    char a; // symbol
    while ((*is).get(a)) {
        int i = a - '0';
        if (i == -16) {
            i = 11; // this is ' ' 
        }
        std::cout << "got: " << i  << std::endl;
        
        CodeSymb(i, os);
        frequency[i]++;
        for(int j = i-1; j>=0; j--) {
            cum_frequency[j]++;
        }
        for(int q = 0; q < num; q++){
            std::cout << frequency[q] << " ";
        }
        std::cout << std::endl;
        for(int q = 0; q < num; q++){
            std::cout << cum_frequency[q] << " ";
        }
        std::cout << std::endl;
    }
    CodeSymb(12, os); //  12 is end
    counter++;
    SendBit((leftmost & h) ? 1 : 0, os);
    std::cout << "buffer: " << buffer[0] << std::endl;
}

template<typename In, typename Out>
void ARIFM<In, Out>::CodeSymb(int i, Out *&os){
    
    unsigned long tmp = l;
    l = tmp + round(((h-tmp+1)*cum_frequency[i])/cum_frequency[0]);
    h = tmp + round(((h-tmp+1)*cum_frequency[i-1])/cum_frequency[0]) - 1;

    std::cout << "L" << l << ", H" << h << std::endl;

    
    do {
        if ((leftmost & l) == (leftmost & h)) {
            std::cout << "got bits" << std::endl;
            if (leftmost & h)
                SendBit(1, os);
            else 
                SendBit(0, os);
            l = 2*l;
            h = 2*h+1;
        } else if (h-l < cum_frequency[0]) {
            std::cout << "h-l < [0]" << std::endl;
            l = 2*(l-Nminus2);
            h = 2*(h-Nminus2) +1;
            counter++;
        }
    } while (((leftmost & l) == (leftmost & h)) or (h-l < cum_frequency[0])); 
}

template<typename In, typename Out>
void ARIFM<In, Out>::SendBit(bool bit, Out *&os) {
    //os << bit;
    buffer[0] &= (bit ? (1UL<<countbits) : (0UL<<countbits));
    countbits--;
    if (countbits < 0) countbits = 63;
    std::cout << "got bit " << (bit ? 1 : 0) << std::endl;
    

    while(counter > 0){
        if (!bit) {
            //os << 1;
            //std::cout << 1 << std::endl;
            buffer[0] &= (1UL<<countbits);
        } else {
            //os << 0;
            //std::cout << 0 << std::endl;
            buffer[0] &= (1UL<<countbits);
        }
        counter--;
    }
}
*/
#endif