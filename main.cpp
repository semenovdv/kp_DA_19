/*
    Формат запуска должен быть аналогичен формату запуска программы gzip,
    должны быть поддержаны следующие ключи: -c, -d, -k, -l, -r, -t, -1, -9. 
    Должно поддерживаться указание символа дефиса в качестве стандартного ввода.

    -c --stdout --to-stdout
        Write  output  on  standard output; keep original files unchanged.  If there are
        several input files, the output consists of a  sequence  of  independently  com‐
        pressed  members.  To  obtain  better  compression,  concatenate all input files
        before compressing them.

    -d --decompress --uncompress
        Decompress.

    -k --keep
        Keep (don't delete) input files during compression or decompression.

    -r --recursive
        Travel the directory structure recursively. If any of the file  names  specified
        on  the  command  line are directories, gzip will descend into the directory and
        compress all the files it finds there (or decompress them in the case of  gunzip
        ).

*/

/*
    TODO:

        1 - DONE - let -k work
        2 - DONE - make working compression
            2.1 - DONE - add arifmethic coding
            2.2 - DONE - make norm bits io
        4 - NO - make levels of compression
        5 - DONE - see work_c in zip.h
        6 - DONE - make normal thread_stdin() 
        7 - DONE - Redo the ARIFM from 0-9 to 0-maxchar

    TODO later:
        1 - add multithreading (lzw -> arithmetic) using pipes or zmq
        2 - add md5 hash to end ?

*/



#include "zip.h"



std::string progname;

//-c, -d, -k, -r

void usage() {
    std::cout << "usage: "<< progname << " [-cdhkr]... [file...]" << std::endl;
}

void help(){
    usage();
    std::cout << "Compress or uncompress FILEs (by default, compress FILES in-place)." << std::endl <<
            "   -c      write on standard output, keep original files unchanged"       << std::endl << 
            "   -d      decompress"                                                    << std::endl << 
            "   -h      give this help"                                                << std::endl << 
            "   -k      keep (don't delete) input files"                               << std::endl << 
            "   -r      operate recursively on directories"                            << std::endl <<
            "With no FILE, or when FILE is -, read standard input."                    << std::endl;
}

int main (int argc, char **argv) {


    ZIP worker; 

    int optc; // current option 

    progname = argv[0];
    worker.globalArgs.progname = progname;


    //-c, -d, -k, -r 
    while ((optc = getopt(argc, argv, "cdhH?kr")) != -1) {
	switch (optc) {
        case 'c':
            worker.globalArgs.to_stdout = true; 
            break;

        case 'd':
            worker.globalArgs.decompress = true; 
            break;

        case 'h': case 'H': case '?':
            help(); exit(OK); break;

        case 'r':
            worker.globalArgs.recursive = true; 
            break;
        case 'k':
            worker.globalArgs.keep = true;
            break;
            
        default:
            std::cout << "Smth Wrong" << std::endl;
            usage();
            exit(WARNING);
        }
    } /* loop on all arguments */

    worker.globalArgs.filecount = argc - optind;

    // work starts here
    worker.treat(optind, argc, argv);

    exit(OK);
    return OK; /* just to avoid lint warning */
}