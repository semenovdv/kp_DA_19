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

    -l --list
        For each compressed file, list the following fields:

        compressed size: size of the compressed file
        uncompressed size: size of the uncompressed file
        ratio: compression ratio (0.0% if unknown)
        uncompressed_name: name of the uncompressed file

        The uncompressed size is given as -1 for files not in gzip format, such as  com‐
        pressed .Z files. To get the uncompressed size for such a file, you can use:

            zcat file.Z | wc -c

        In  combination  with  the  --verbose option, the following fields are also dis‐
        played:

            method: compression method
            crc: the 32-bit CRC of the uncompressed data
            date & time: time stamp for the uncompressed file

        The compression methods currently supported are deflate, compress, lzh (SCO com‐
        press -H) and pack.  The crc is given as ffffffff for a file not in gzip format.

        With  --name, the uncompressed name,  date and time  are those stored within the
        compress file if present.

        With --verbose, the size totals and compression ratio for all files is also dis‐
        played,  unless some sizes are unknown. With --quiet, the title and totals lines
        are not displayed.

    -r --recursive
        Travel the directory structure recursively. If any of the file  names  specified
        on  the  command  line are directories, gzip will descend into the directory and
        compress all the files it finds there (or decompress them in the case of  gunzip
        ).

    -t --test
        Test. Check the compressed file integrity.

    -# --fast --best
        Regulate the speed of compression using the  specified  digit  #,  where  -1  or
        --fast  indicates  the  fastest  compression method (less compression) and -9 or
        --best indicates the slowest compression method (best compression).  The default
        compression  level is -6 (that is, biased towards high compression at expense of
        speed).


*/

/*
    TODO:
---------------------------
see lzw decode
---------------------

        1 - let -k work
        2 - make working compression
            2.1 - add arifmethic coding
            2.2 - make norm bits input/output
        4 - make levels of compression, append maxinum_value in lzw map to make levels
        5 - see work_c in zip.h
        6 - DONE - make normal thread_stdin() 
        7 - let -l work
        8 - let -t work

    TODO later:
        1 - add multithreading (lzw -> arithmetic) using pipes or zmq

*/



#include "zip.h"





std::string progname;

//-c, -d, -k, -l, -r, -t, -1, -9

void usage() {
    std::cout << "usage: "<< progname << " [-cdhkltr19]... [file...]" << std::endl;
}

void help(){
    usage();
    std::cout << "Compress or uncompress FILEs (by default, compress FILES in-place)." << std::endl <<
            "   -c      write on standard output, keep original files unchanged"       << std::endl << 
            "   -d      decompress"                                                    << std::endl << 
            "   -h      give this help"                                                << std::endl << 
            "   -k      keep (don't delete) input files"                               << std::endl << 
            "   -l      list compressed file contents"                                 << std::endl <<
            "   -r      operate recursively on directories"                            << std::endl <<
            "   -t      test compressed file integrity"                                << std::endl <<
            "   -1      compress faster"                                               << std::endl <<
            "   -9      compress better"                                               << std::endl <<
            "With no FILE, or when FILE is -, read standard input."                    << std::endl;
}

int main (int argc, char **argv) {


    ZIP worker; 

    int optc; // current option 

    progname = argv[0];
    worker.globalArgs.progname = progname;


    //-c, -d, -k, -l, -r, -t, -1, -9. 
    while ((optc = getopt(argc, argv, "cdhH?klrt19")) != -1) {
	switch (optc) {
        case 'c':
            worker.globalArgs.to_stdout = true; 
            std::cout << "to_stdout" << std::endl;
            break;

        case 'd':
            worker.globalArgs.decompress = true; 
            std::cout << "decompress" << std::endl;
            break;

        case 'h': case 'H': case '?':
            help(); exit(OK); break;

        case 'l':
            std::cout << "list" << std::endl;
            worker.globalArgs.list = worker.globalArgs.decompress = worker.globalArgs.to_stdout = true; 
            break;

        case 'r':
            std::cout << "recursive" << std::endl;
            worker.globalArgs.recursive = true; break;

        case 't':
            std::cout << "test" << std::endl;
            worker.globalArgs.test = worker.globalArgs.decompress = worker.globalArgs.to_stdout = true;
            break;
        
        case 'k':
            std::cout << "k means keep files" << std::endl;
            worker.globalArgs.keep = true;
            break;

        case '1': case '9':
            worker.globalArgs.level = optc - '0';
            std::cout << "level: " << worker.globalArgs.level << std::endl;
            break;

        default:
            /* Error message. */
            std::cout << "OMGWFT" << std::endl;
            usage();
            exit(WARNING);
        }
    } /* loop on all arguments */

    worker.globalArgs.filecount = argc - optind;

    /* And get to work */
    worker.treat(optind, argc, argv);

    exit(OK);
    return OK; /* just to avoid lint warning */
}