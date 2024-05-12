#!/usr/bin/env python
import json
import optparse
import os
import os.path
import re
import shutil

# --bump-version 0.5.0
# --build-local-libs
# --upload-platformio
# --help

parser = optparse.OptionParser(
    usage='usage: %prog [options]',
    description='Build script for the project'
)

parser.add_option('--bump-version', dest='bump_version', action='store_true')
parser.add_option('--build-local-libs', dest='build_local_libs', action='store_true')
parser.add_option('--upload-platformio', dest='upload_platformio', action='store_true')
parser.add_option('--develop', dest='develop', action='store_true')
parser.add_option('--deploy', dest='deploy', action='store_true')
parser.add_option('--next-version', dest='next_version', action='store_true')
(options, args) = parser.parse_args()

# print help if no options are specified
if not options.bump_version and not options.build_local_libs and not options.upload_platformio and not options.develop and not options.deploy:
    parser.print_help()
    exit()


def eval_current_version():
    # Get the current version from src/ftswarm-core/library.json
    with open(os.path.join("src", "ftswarm-core", "library.json")) as f:
        lines = json.load(f)
        return lines["version"]


def do_libjson_operation(fn, filter_core=True):
    for folder in os.listdir("src"):
        if folder.startswith("ftswarm-") and (not filter_core or folder != "ftswarm-core"):
            library_json = os.path.join("src", folder, "library.json")
            with open(library_json, "r") as f:
                lines = json.load(f)
                fn(lines)

            with open(library_json, "w") as f:
                json.dump(lines, f, indent=4)


def switch_to_develop():
    print('Switching to develop mode')

    def use_version_operation(lines):
        for dep in lines["dependencies"]:
            if dep == "Elektrofuzzis/ftswarm-core":
                lines["dependencies"][dep] = "symlink://../ftswarm-core"

    do_libjson_operation(use_version_operation, True)


def switch_to_deploy():
    print('Switching to deploy mode')
    current_version = eval_current_version()

    def use_version_operation(lines):
        for dep in lines["dependencies"]:
            if dep == "Elektrofuzzis/ftswarm-core":
                lines["dependencies"][dep] = current_version

    do_libjson_operation(use_version_operation, True)


def bump_version():
    print('Bumping version')
    # Get version from src/ftswarm-core/include/SwOS.h (#define SWOSVERSION "0.5.0")
    version_regex = r'#define SWOSVERSION "(.*)"'
    with open(os.path.join("src", "ftswarm-core", "include", "SwOS.h")) as f:
        lines = f.readlines()
        for line in lines:
            match = re.search(version_regex, line)
            if match:
                current_version = match.group(1)
                break

    print("Current version: " + current_version)

    def bump_version_operation(lines):
        lines["version"] = current_version
        for dep in lines["dependencies"]:
            if dep == "Elektrofuzzis/ftswarm-core":
                lines["dependencies"][dep] = current_version

    do_libjson_operation(bump_version_operation, False)

    with open(os.path.join("src", "arduino", "library-template", "library.json")) as f:
        lines = json.load(f)
        lines["version"] = current_version

    with open(os.path.join("src", "arduino", "library-template", "library.json"), "w") as f:
        json.dump(lines, f, indent=4)

    with open(os.path.join("src", "arduino", "library-template", "library.properties")) as f:
        lines = f.readlines()
        for i, line in enumerate(lines):
            if line.startswith("version"):
                lines[i] = "version=" + current_version + "\n"
                break

    with open(os.path.join("src", "arduino", "library-template", "library.properties"), "w") as f:
        f.writelines(lines)


def gen_pio_examples():
    # operation on every dir starting with ftswarm- except ftswarm-core
    for file in os.listdir("src"):
        if file.startswith("ftswarm-") and file != "ftswarm-core":
            # copy the example template to the example folder
            example_template = "examples"
            example_folder = os.path.join("src", file, "examples")
            if os.path.exists(example_folder):
                shutil.rmtree(example_folder)
            shutil.copytree(example_template, example_folder)

            header = ""
            for f in os.listdir(os.path.join("src", file, "include")):
                if f.startswith("ftSwarm") and f.endswith(".h"):
                    header = f
                    break

            print("[pio] For " + file + " found header: " + header)

            # rename all .ino files to .cpp
            for file in os.walk(example_folder):
                for f in file[2]:
                    if f.endswith(".ino"):
                        os.rename(os.path.join(file[0], f), os.path.join(file[0], f.replace(".ino", ".cpp")))

                        with open(os.path.join(file[0], f.replace(".ino", ".cpp"))) as fd:
                            lines = fd.readlines()
                            for i, line in enumerate(lines):
                                if line.startswith("#include <ftSwarm.h>"):
                                    lines[i] = "#include <" + header + ">\n"
                                    break

                        with open(os.path.join(file[0], f.replace(".ino", ".cpp")), "w") as f:
                            f.writelines(lines)


def gen_arduino_lib(type):
    folder = os.path.join("src", "arduino", "library-" + type)
    folder2 = os.path.join("src", "arduino", "library-" + type, "ftSwarm-" + type)
    print(folder2)
    if os.path.exists(folder):
        shutil.rmtree(folder)
    shutil.copytree(os.path.join("src", "arduino", "library-template"), folder2)
    shutil.copytree("examples", os.path.join(folder2, "examples"))

    # copy core/src to library/src
    shutil.copytree(os.path.join("src", "ftswarm-core", "src"), os.path.join(folder2, "src"))
    shutil.copytree(os.path.join("src", "ftswarm-core", "include"), os.path.join(folder2, "src"), dirs_exist_ok=True)

    # copy ftswarm-type/include to library/src
    shutil.copytree(os.path.join("src", "ftswarm-" + type, "include"), os.path.join(folder2, "src"), dirs_exist_ok=True)

    # modify library.json
    with open(os.path.join(folder2, "library.json")) as f:
        lines = json.load(f)
        lines["name"] = "ftswarm-" + type
        lines["version"] = eval_current_version()

    with open(os.path.join(folder2, "library.json"), "w") as f:
        json.dump(lines, f, indent=4)

    # modify library.properties
    with open(os.path.join(folder2, "library.properties")) as f:
        lines = f.readlines()
        for i, line in enumerate(lines):
            if line.startswith("version"):
                lines[i] = "version=" + eval_current_version() + "\n"
            elif line.startswith("name"):
                lines[i] = "name=ftswarm-" + type + "\n"

    with open(os.path.join(folder2, "library.properties"), "w") as f:
        f.writelines(lines)

    # list library/src files to find the one with ftSwarm{??}.h, get the ??
    ftswarm_header = ""
    for file in os.listdir(os.path.join(folder2, "src")):
        if file.startswith("ftSwarm") and file.endswith(".h"):
            ftswarm_header = file
            break

    print("[ino] For ftswarm-" + type + " found header: " + ftswarm_header)

    # Modify examples to include the correct header (Replace #include <ftSwarm.h> with #include <ftSwarm{??}.h>)
    for example in os.walk(os.path.join(folder2, "examples")):
        for file in example[2]:
            if file.endswith(".ino"):
                with open(os.path.join(example[0], file)) as f:
                    lines = f.readlines()
                    for i, line in enumerate(lines):
                        if line.startswith("#include <ftSwarm.h>"):
                            lines[i] = "#include <" + ftswarm_header + ">\n"
                            break

                with open(os.path.join(example[0], file), "w") as f:
                    f.writelines(lines)

    # Create zip file
    shutil.make_archive(os.path.join("dist", "ftswarm-" + type), 'zip', folder)


def gen_pio_tarball(type):
    # Execute pio pkg pack --force --verbose --out dist/ftswarm-{type}.tar.gz in the library folder
    cwd = os.getcwd()
    os.chdir(os.path.join("src", "ftswarm-" + type))
    os.system("pio pkg pack")
    os.chdir(cwd)


def build_local_libs():
    print('Building local libs')
    gen_pio_examples()

    # create dist folder
    if os.path.exists("dist"):
        shutil.rmtree("dist")

    os.mkdir("dist")

    for lib in os.listdir("src"):
        if lib.startswith("ftswarm-") and lib != "ftswarm-core":
            gen_arduino_lib(lib[8:])
            #gen_pio_tarball(lib[8:])


def upload_platformio():
    print('Uploading')
    # Go to every ftswarm- folder and execute pio package publish
    for lib in os.listdir("src"):
        if lib.startswith("ftswarm-"):
            cwd = os.getcwd()
            os.chdir(os.path.join("src", lib))
            os.system("pio package publish")
            os.chdir(cwd)


if options.deploy:
    switch_to_deploy()

if options.develop:
    switch_to_develop()

if options.bump_version:
    bump_version()

if options.build_local_libs:
    build_local_libs()

if options.upload_platformio:
    upload_platformio()

if options.next_version:
    switch_to_deploy()
    bump_version()
    build_local_libs()
    upload_platformio()
    switch_to_develop()
