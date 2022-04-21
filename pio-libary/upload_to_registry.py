import json
import shutil
import os
import time

shutil.rmtree("include/")
shutil.rmtree("src/")

os.mkdir("include")
shutil.copytree("../arduino-cpp-library/src", "src")
shutil.move("src/ftSwarm.h", "include/")

lib = json.load(open("library.json"))
version = lib["version"]

major = version.split("-")[0]
vid = int(time.time())

lib["version"] = f"{major}-{vid}"
json.dump(lib, open("library.json", "w"))

os.system("pio package publish")