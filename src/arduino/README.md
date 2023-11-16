# Where are the arduino files?

The arduino library files are no longer available directly. Some time ago, we switched 
to using the [PlatformIO](https://platformio.org/) build system, which is much more
flexible and faster than the Arduino IDE. Since we still support the Arduino IDE, we
built a simple python script for generating the arduino library files from the PlatformIO
content. 

If you just want to use the stable library, head over to the GitHub Releases page and download
the zip file for your hardware type there.

To test the latest 🔪 cutting-edge features, clone the `dev`-branch of this repository and
execute the following command in the root directory of the repository:

```bash
# A valid python3 and platformio installation is required
python3 build.py --build-local-libs
```

This will generate the library files in the `dist` directory. 

<hr>
&copy; Elektofuzzis