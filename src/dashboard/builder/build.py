#!/usr/bin/env python3

import gzip
import hashlib
import os
import shutil

from halo import Halo


def gzip_file(file, dest):
    """
    Gzips a file and writes it to a destination
    :param file:  The file to gzip
    :param dest:  The destination to write the gzipped file to
    """
    with open(file, 'rb') as f_in:
        with gzip.open(dest, 'wb') as f_out:
            shutil.copyfileobj(f_in, f_out)


def get_file_size(_filename):
    """
    Returns the size of a file in bytes
    :param _filename:  The file to get the size of
    :return:  The size of the file in bytes
    """
    return os.stat(_filename).st_size


def get_file_hash(_filename):
    """
    Returns the SHA1 hash of a file
    :param _filename: The file to get the hash of
    :return: The SHA1 hash of the file
    """
    with open(_filename, 'rb') as f:
        return hashlib.sha1(f.read()).hexdigest()


def get_sfs_defs(sfs_file):
    """
    Returns the SFS file definitions for a given SFS file
    :param sfs_file: The SFS file to get the definitions for
    :return: The SFS file definitions for the given SFS file
    """
    with open(sfs_file, 'rb') as f:
        file_contents = f.read(get_file_size(sfs_file))
        literals = []
        for char in file_contents:
            literals.append(f"0x{char:02x}")

        # Split the literals into chunks of 16
        chunks = [literals[i:i + 16] for i in range(0, len(literals), 16)]

        lines = []
        for chunk in chunks:
            lines.append("    " + ", ".join(chunk) + ",")

        definition = """
const char sfs""" + str(sfs_file.replace("gzipped", "").replace(os.path.sep, "_").replace(".", "_").replace("-", "_")) + """[] = {
"""

        definition += "\n".join(lines)
        definition = definition[:-1]
        definition += """
};
"""

    return definition


def get_file_appendix(sfs_file):
    """
    Returns the appendix for a given SFS file
    :param sfs_file: The SFS file to get the appendix for
    :return: The appendix for the given SFS file
    """
    return """
    if (strcmp(filename, \"""" + sfs_file[7:].replace("\\", "/") + """\") == 0) {
        *length = """ + str(get_file_size(sfs_file)) + """;
        return sfs""" + str(
        str(sfs_file).replace("gzipped", "").replace(os.path.sep, "_").replace(".", "_").replace("-", "_")) + """;
    } else """


consoleloader = Halo(text='Preparing...', spinner="arc")
consoleloader.start()

# Initial Cleanup
if os.path.exists("deployment"):
    shutil.rmtree("deployment")
os.mkdir('deployment')
shutil.copytree('dist', os.path.join('deployment', 'base'))

os.chdir('deployment')
os.mkdir('gzipped')

consoleloader.succeed('Prepared')
consoleloader = Halo('Compressing...', spinner="arc").start()

# Compress the files
for i in os.walk('base', topdown=True, onerror=None, followlinks=False):
    dirname = i[0][5:]
    if not os.path.exists(os.path.join("gzipped", dirname)):
        os.mkdir(os.path.join("gzipped", dirname))
    for filename in i[2]:
        gzip_file(os.path.join(i[0], filename), os.path.join('gzipped', dirname, filename))

consoleloader.succeed('Compressed')
consoleloader = Halo('Building to files...', spinner="arc").start()

# Build the files
sfs_files_h = """
/**
 * @file sfs_files.h
 * @brief File list for the Web Server
 * @long This file contains the list of files that are available on the Web Server. It is generated by the build script 
 *       in /dashboard. It is not intended to be modified by the user.
 */

#pragma once
#include <stdint.h>

extern const char *sfs_get_file( char *filename, uint32_t *length );

"""

sfs_files_cpp = """
/**
 * @file sfs_files.cpp
 * @brief File list for the Web Server
 * @long This file contains the list of files that are available on the Web Server. It is generated by the build script 
 *       in /dashboard. It is not intended to be modified by the user.
 */
 
#include "sfs_files.h"
#include <stdint.h>
#include <string.h>

"""

sfs_files_cpp_appedix = """
const char *sfs_get_file( char *filename, uint32_t *length ) {
"""

# Get the SFS file definitions
for i in os.walk('gzipped', topdown=True, onerror=None, followlinks=False):
    dirname = i[0][8:]
    for filename in i[2]:
        sfs_files_cpp += get_sfs_defs(os.path.join(i[0], filename))
        sfs_files_cpp_appedix += get_file_appendix(os.path.join(i[0], filename))
        sfs_files_h += "\n"
        sfs_files_h += f"// /{dirname}/{filename}  SHA1:{get_file_hash(os.path.join(i[0], filename))}\n"
        sfs_files_h += "extern const char sfs" + str(
            os.path.join(i[0], filename).replace("gzipped", "").replace(os.path.sep, "_").replace(".", "_").replace("-",
                                                                                                                    "_")) + "[];\n"
        sfs_files_h += "#define SFS_" + str(
            filename.replace("gzipped", "").replace(os.path.sep, "_").replace(".", "_").replace("-",
                                                                                                "_")) + "_len " + str(
            get_file_size(os.path.join(i[0], filename))) + "\n"

sfs_files_cpp += sfs_files_cpp_appedix
sfs_files_cpp += """{
        *length = 0;
        return "";
    }
}

"""

consoleloader.succeed('Built')
consoleloader = Halo('Writing files...', spinner="arc").start()

# Write the files
with open("sfs_files.h", "w") as f:
    f.write(sfs_files_h)

with open("sfs_files.cpp", "w") as f:
    f.write(sfs_files_cpp)

consoleloader.succeed('Done writing files')
consoleloader = Halo('Cleaning up...', spinner="arc").start()

# Cleanup
shutil.rmtree('gzipped')
shutil.rmtree('base')

# Warn user about encoding headers
consoleloader.warn("Please be sure to provide the HTTP header 'Content-Encoding: gzip' to the client.")
consoleloader.succeed("Cleaned up")
consoleloader = Halo("Integrating...")

if os.path.exists("../../pio-develop/src/sfs_files.cpp"):
    os.remove("../../pio-develop/src/sfs_files.cpp")
if os.path.exists("../../pio-develop/src/sfs_files.h"):
    os.remove("../../pio-develop/src/sfs_files.h")

shutil.copy("sfs_files.cpp", "../../pio-develop/src/")
shutil.copy("sfs_files.h", "../../pio-develop/src/")

consoleloader.succeed("Integrated")
