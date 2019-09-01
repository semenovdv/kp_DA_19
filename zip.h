#ifndef ZIP_H
#define ZIP_H

#include "includes.h"
#include "lzw.h"
#include "arifm.h"

#define	GZIP_MAGIC     "\037\213" /* Magic header for gzip files, 1F 8B */

namespace fs = std::experimental::filesystem;

const unsigned int EOF_CODE = 256;

struct globalArgs_t {

    bool to_stdout;
    bool decompress;
    bool list;
    bool recursive;
    bool test;
    long level; 
    bool keep;
    std::string progname; 
    std::string filename;
    int filecount;
    fs::path path;
};


class ZIP {
public:
    ZIP(){
        globalArgs.to_stdout = false;
        globalArgs.decompress = false; 
        globalArgs.keep = false;
        globalArgs.level = 1;
        globalArgs.list = false;
        globalArgs.recursive = false;
        globalArgs.test = false;
        globalArgs.progname = "";
        globalArgs.filename = "";
        globalArgs.filecount = 0;
    }

    globalArgs_t globalArgs;

    /* start working from here */
    void treat(int optind, int argc, char **argv); 


    void treat_stdin(); /* input is stdin */ 
    void treat_file(); /* regular file */
    void treat_dir(); /* recursive directory */

    void do_list(int , int);


    template<typename T1, typename T2>
    void work_code(T1 *&In, T2 *&Out, bool is_in_symb, bool is_out_symb);

    template<typename T1, typename T2>
    void work_decode(T1 *&In, T2 *&Out, bool is_in_symb, bool is_out_symb);


   
};


void ZIP::treat(int optind, int argc, char **argv){

    // if we have >= 1 file(s) 

    if (globalArgs.filecount != 0) {

        while (optind < argc) {

            // if "-" as file 
            if (std::string(argv[optind]) == "-"){
                treat_stdin();
                return;
            }

            // forming path to working file 
            fs::path curpath = fs::current_path(); // before working (active) directory 
            fs::path mypath = fs::path(argv[optind]); // after
               

            if (fs::is_directory(mypath)) { 

                std::cout << "directory " << std::string(argv[optind]) << std::endl;

                if (globalArgs.recursive) {
                    std::cout << "works recursievely..." << std::endl;
                    globalArgs.path = curpath/mypath;
                    treat_dir();
                    optind++;
                } else{
                    std::cerr  << globalArgs.progname << ": " << argv[optind] << " is a directory -- ignored " << std::endl;
                    optind++;
                }
                
            } else {
                globalArgs.path = curpath;
                globalArgs.filename = mypath;

                std::cout << "regular file " <<  std::string(argv[optind]) << std::endl;
                optind++;                
                treat_file();
        
            } 

            if (!globalArgs.keep) fs::remove(mypath);
        }   
    }  // 0 input files
    else {
        treat_stdin();
        return;
    }
}


void ZIP::treat_stdin(){

    // Do not send compressed data to the terminal or read it from
	// the terminal. We get here when user invoked the program
	// without parameters, so be helpful. 
    
    if (!globalArgs.list && 
        isatty(fileno((FILE *)(globalArgs.decompress ? stdin : stdout)))){

        std::cerr << globalArgs.progname << ": compressed data not " << 
        (globalArgs.decompress ? "read from" : "written to") << " a terminal." << std::endl <<
        "For help, type: gzip -h" << std::endl;

        return;
    }

    std::istream *in = &std::cin;
    std::ostream *out = &std::cout;
    std::cout << "stdin, stdout" << std::endl;

    if (globalArgs.decompress) {
        work_decode(in, out, true, true);
    } else {
        work_code(in, out, true, true);
    }
}



void ZIP::treat_dir() {
    fs::path dir = globalArgs.path;
    for(auto& p: fs::recursive_directory_iterator(dir))
        if(fs::is_regular_file(p)) {
            globalArgs.path = p.path();
            globalArgs.filename = globalArgs.path.filename();
            std::cout << "file " << globalArgs.path << " works!" << std::endl;

            std::cout << "thread file works!" << std::endl;
            fs::path workdir = fs::current_path();
            fs::current_path(globalArgs.path.parent_path());

            treat_file();
            fs::current_path(workdir);
        }
    
}


void ZIP::treat_file(){
    

    if (globalArgs.decompress){
        std::cout << "decompress"<< std::endl;

        std::ifstream *in = new std::ifstream( globalArgs.filename, std::ios_base::in|std::ios_base::binary);
        
        if (!fs::exists(globalArgs.filename)) {
            std::cout << "ERROR: file not exists1" << std::endl;
            delete in;
            exit(ERROR);
        }

        if (!in) {
            std::cout << "EROOR: iin file ifstream not opened!" << std::endl;
            delete in;
            exit(ERROR);
        }
        std::string newstr(globalArgs.filename, 0, globalArgs.filename.length()- 2);
        
        if (globalArgs.to_stdout){

            std::ostream *out = &std::cout;
            work_decode(in, out, false, true);
        } else{
            std::ofstream *out = new std::ofstream(newstr, std::ios_base::out);
            std::cout << "decoding in " << newstr << std::endl;
            work_decode(in, out, false, true);
            (*out).close();
            delete out;
        }

        (*in).close();
        delete in;

    } else {
        std::cout << "compress" << std::endl;
        std::ifstream *in = new std::ifstream( globalArgs.filename , std::ios_base::in);

        if (!fs::exists(globalArgs.filename)) {
            std::cout << "ERROR: file not exists1" << std::endl;
            delete in;
            exit(ERROR);
        }

        if (!in) {
            std::cout << "EROOR: iin file ifstream not opened!" << std::endl;
            delete in;
            exit(ERROR);
        }
            
                       
        if (globalArgs.to_stdout){
            std::ostream *out = &std::cout;
            std::cout << "ifstream , cout" << std::endl;
            work_code(in, out, true, true);
            
        } else{
            std::ofstream *out = new std::ofstream(globalArgs.filename +".l", std::ios_base::out);
            std::cout << "if, of" << std::endl;
            work_code(in, out, true, false);
            
            (*out).close();
            delete out;
        }

        (*in).close();
        delete in;
        
    }
   
    
}




void ZIP::do_list(int a, int b) {

}



//----------------------------------------------------------//


template<typename T1, typename T2>
void ZIP::work_code(T1 *&In, T2 *&Out, bool is_in_symb, bool is_out_symb) {

    std::ofstream *outfile = new std::ofstream("gzip.tmp",  std::ios::binary);
    if (!(*outfile).is_open()) 
        std::cout << "Cannot open tmp file 1" << std::endl;

    LZW<T1, std::ofstream> lzw(globalArgs.level);
    lzw.Code(In, outfile, is_in_symb);

    (*outfile).close();

    std::ifstream *infile = new std::ifstream("gzip.tmp", std::ios::binary);
    if (!(*infile).is_open()) 
        std::cout << "Cannot open tmp file 2" << std::endl;
    ARIFM<std::ifstream, T2> arifm(globalArgs.level);
    arifm.Code(infile, Out, false, is_out_symb);
    (*infile).close();

    if(!fs::remove("gzip.tmp"))
        std::cout << "ERROR: cannot delete tmp file" << std::endl;
    
}




template<typename T1, typename T2>
void ZIP::work_decode(T1 *&In, T2 *&Out, bool is_in_symb, bool is_out_symb) {

    std::ofstream *outfile = new std::ofstream("gzip.tmp",  std::ios::binary);
    if (!(*outfile).is_open()) 
        std::cout << "Cannot open tmp file 1" << std::endl;

    ARIFM<T1, std::ofstream> arifm(globalArgs.level);
    arifm.Decode(In, outfile, is_in_symb, false);

    (*outfile).close();

    std::ifstream *infile = new std::ifstream("gzip.tmp", std::ios::binary);
    if (!(*infile).is_open()) 
        std::cout << "Cannot open tmp file 2" << std::endl;
    LZW<std::ifstream, T2> lzw(globalArgs.level);
    lzw.Decode(infile, Out, false, is_out_symb);
    (*infile).close();

    if(!fs::remove("gzip.tmp"))
        std::cout << "ERROR: cannot delete tmp file" << std::endl;
   
    
}



//----------------------------------------------------------//




/*
template<typename T>
class input_symbol_stream {
public :
    input_symbol_stream( T &input ) 
        : m_input( input ) {}
    bool operator>>( char &c )
    {
        if ( !m_input.get( c ) )
            return false;
        else
            return true;
    }
private :
    T &m_input;
};

template<typename T>
class output_symbol_stream {
public :
    output_symbol_stream( T &output ) 
        : m_output( output ) {}
    void operator<<( const std::string &s )
    {
        m_output << s;
    }
private :
    T &m_output;
};

// HELP ??? TODO

template<typename T>
class output_code_stream {
public :
    output_code_stream( T &output ) 
        : m_output( output ) {}
    void operator<<( unsigned int i )
    {
        m_output.put( i & 0xff );
        m_output.put( (i>>8) & 0xff);
    }
    ~output_code_stream()
    {
        *this << EOF_CODE;
    }
private :
    T &m_output;
};

template<typename T>
class input_code_stream {
public :
    input_code_stream( T &input ) 
        : m_input( input ) {}
    bool operator>>( unsigned int &i )
    {
        char c;
        if ( !m_input.get(c) )
            return false;
        i = c & 0xff;
        if ( !m_input.get(c) )
            return false;
        i |= (c & 0xff) << 8;
        if ( i == EOF_CODE )
            return false;
        else
            return true;
    }
private :
    T &m_input;
};
*/

#endif