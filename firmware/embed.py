#!/usr/bin/env python3

import requests
import os
import time

input_dir = "data"              # Sub folder of webfiles

def write_to_file(sfs_h, sfs_cpp, file, data, dir=""):
    filename, file_extension = os.path.splitext(file)       # Split filename and file extension
    file_extension = file_extension.replace(".","")         # Remove puncuation in file extension

    dir = dir.replace(input_dir,"")                         # Remove the first directory(input_dir)
    dir = dir.replace("\\","/")                             # Chang to /
    sfs_h.write("// " + dir + "\n")                         # Print comment
    sfs_h.write("#define     sfs_" + filename + "_len " + str(data.count('0x')) +"\n")
    # f_output.write("const char* sfs_" + filename + "_" + file_extension + "_path PROGMEM = \""+str(dir)+"\";\n")    # print path
    sfs_h.write("extern const char sfs_" + filename + "_" + file_extension+"[];\n\n")            # print binary data

    sfs_cpp.write("const char sfs_" + filename + "_" + file_extension+"[] = {"+data.upper()+"};\n\n")            # print binary data

def write_to_file2(sfs_cpp, file):
    filename, file_extension = os.path.splitext(file)       # Split filename and file extension
    file_extension = file_extension.replace(".","")         # Remove puncuation in file extension

    sfs_cpp.write( "  if ( strcmp( filename, \"" + filename + "." + file_extension + "\") == 0 ) { \n");
    sfs_cpp.write( "    *length = sfs_" + filename + "_len;\n" );
    sfs_cpp.write( "    return sfs_" + filename + "_" + file_extension + ";\n" );
    sfs_cpp.write( "  } else" );
    
def aschii2Hex(text):
    output_str = "\n\t"
    x = 1
    strLen = len(text)
    for character in text:
        output_str += f"{hex(ord(character)):>4}"

        if (x != strLen):
            output_str += ","
        if (x%16 == 0):
            output_str += "\n\t"
        x += 1
    return output_str + "\n"


with open("sfs_files.h", "w") as sfs_h:
    
    sfs_h.write("/* sfs_files.h\n")
    sfs_h.write(" *\n");
    sfs_h.write(" * Automatically converted files from folder " + input_dir + " to \"const char\" (html, css, and some other stuff)\n")
    sfs_h.write(" */\n");
    sfs_h.write("\n");
    sfs_h.write("#pragma once\n");
    sfs_h.write("\n");
    sfs_h.write("#include <stdint.h>\n");
    sfs_h.write("\n");
    sfs_h.write("extern const char *sfs_get_file( char *filename, uint32_t *length );\n\n");

    with open("sfs_files.cpp", "w") as sfs_cpp:
        sfs_cpp.write("/* sfs_files.h\n")
        sfs_cpp.write(" *\n");
        sfs_cpp.write(" * Automatically converted files from folder " + input_dir + " to \"const char\" (html, css, and some other stuff)\n")
        sfs_cpp.write(" */\n");
        sfs_cpp.write("\n");
        sfs_cpp.write("#include \"sfs_files.h\"\n");
        sfs_cpp.write("#include <stdint.h>\n");
        sfs_cpp.write("#include <string.h>\n");        sfs_cpp.write("\n");
        for root, dirs, files in os.walk(input_dir, topdown=False):
            for name in files:
                print(os.path.join(root, name))
                with open(os.path.join(root, name), 'r') as f:
                    content = f.read()
                    hexified = aschii2Hex(content)
                    write_to_file(sfs_h, sfs_cpp, name, hexified, os.path.join(root, name))
        sfs_cpp.write("const char *sfs_get_file( char *filename, uint32_t *length ) { \n\n");
        for root, dirs, files in os.walk(input_dir, topdown=False):
            for name in files:
                write_to_file2(sfs_cpp, name )
        sfs_cpp.write("{\n    *length=0;\n");
        sfs_cpp.write("    return \"\";\n");
        sfs_cpp.write("  }\n");
        sfs_cpp.write("}\n");
