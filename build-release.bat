rem build ftSwarm release

rem delete old stuff
rmdir release /s /q

rem build arduino
mkdir release\arduino\
xcopy src\arduino\* release\arduino\ /f /s
xcopy src\pio-develop\src\*.cpp release\arduino\library\src\* /f /s
xcopy src\pio-develop\src\*.h   release\arduino\library\src\* /f /s
del release\arduino\library\src\main.cpp

rem build pio
mkdir release\pio
mkdir release\pio\library\ftSwarm\include
mkdir release\pio\library\ftSwarm\src
xcopy src\pio\library\ftSwarm\*     release\pio\library\ftSwarm\* /f /s
xcopy src\pio-develop\src\*.cpp     release\pio\library\ftSwarm\src\* /f /s
xcopy src\pio-develop\src\*.h       release\pio\library\ftSwarm\src\* /f /s
xcopy src\pio-develop\src\ftSwarm.h release\pio\library\ftSwarm\include\* /f /s
del release\pio\library\ftSwarm\src\main.cpp
del release\pio\library\ftSwarm\src\ftSwarm.h
del release\pio\library\ftSwarm\src\ftSwarmRS.h
del release\pio\library\ftSwarm\src\ftSwarmControl.h
del release\pio\library\ftSwarm\src\ftSwarmCAM.h
del release\pio\library\ftSwarm\src\ftSwarmPwrDrive.h

rem ftSwarmControl
mkdir release\pio\library\ftSwarmControl\include
mkdir release\pio\library\ftSwarmControl\src
xcopy src\pio\library\ftSwarmControl\*     release\pio\library\ftSwarmControl\* /f /s
xcopy src\pio-develop\src\*.cpp            release\pio\library\ftSwarmControl\src\* /f /s
xcopy src\pio-develop\src\*.h              release\pio\library\ftSwarmControl\src\* /f /s
xcopy src\pio-develop\src\ftSwarmControl.h release\pio\library\ftSwarmControl\include\* /f /s
del release\pio\library\ftSwarmControl\src\main.cpp
del release\pio\library\ftSwarmControl\src\ftSwarm.h
del release\pio\library\ftSwarmControl\src\ftSwarmRS.h
del release\pio\library\ftSwarmControl\src\ftSwarmControl.h
del release\pio\library\ftSwarmControl\src\ftSwarmCAM.h
del release\pio\library\ftSwarmControl\src\ftSwarmPwrDrive.h


rem ftSwarmRS
mkdir release\pio\library\ftSwarmRS\include
mkdir release\pio\library\ftSwarmRS\src
xcopy src\pio\library\ftSwarmRS\*     release\pio\library\ftSwarmRS\* /f /s
xcopy src\pio-develop\src\*.cpp       release\pio\library\ftSwarmRS\src\* /f /s
xcopy src\pio-develop\src\*.h         release\pio\library\ftSwarmRS\src\* /f /s
xcopy src\pio-develop\src\ftSwarmRS.h release\pio\library\ftSwarmRS\include\* /f /s
del release\pio\library\ftSwarmRS\src\main.cpp
del release\pio\library\ftSwarmRS\src\ftSwarm.h
del release\pio\library\ftSwarmRS\src\ftSwarmRS.h
del release\pio\library\ftSwarmRS\src\ftSwarmControl.h
del release\pio\library\ftSwarmRS\src\ftSwarmCAM.h
del release\pio\library\ftSwarmRS\src\ftSwarmPwrDrive.h

rem ftSwarmCAM
mkdir release\pio\library\ftSwarmCAM\include
mkdir release\pio\library\ftSwarmCAM\src
xcopy src\pio\library\ftSwarmCAM\*     release\pio\library\ftSwarmCAM\* /f /s
xcopy src\pio-develop\src\*.cpp        release\pio\library\ftSwarmCAM\src\* /f /s
xcopy src\pio-develop\src\*.h          release\pio\library\ftSwarmCAM\src\* /f /s
xcopy src\pio-develop\src\ftSwarmCAM.h release\pio\library\ftSwarmCAM\include\* /f /s
del release\pio\library\ftSwarmCAM\src\main.cpp
del release\pio\library\ftSwarmCAM\src\ftSwarm.h
del release\pio\library\ftSwarmCAM\src\ftSwarmRS.h
del release\pio\library\ftSwarmCAM\src\ftSwarmControl.h
del release\pio\library\ftSwarmCAM\src\ftSwarmCAM.h
del release\pio\library\ftSwarmCAM\src\ftSwarmPwrDrive.h

rem ftSwarmCAM
mkdir release\pio\library\ftSwarmPwrDrive\include
mkdir release\pio\library\ftSwarmPwrDrive\src
xcopy src\pio\library\ftSwarmPwrDrive\*     release\pio\library\ftSwarmPwrDrive\* /f /s
xcopy src\pio-develop\src\*.cpp             release\pio\library\ftSwarmPwrDrive\src\* /f /s
xcopy src\pio-develop\src\*.h               release\pio\library\ftSwarmPwrDrive\src\* /f /s
xcopy src\pio-develop\src\ftSwarmPwrDrive.h release\pio\library\ftSwarmPwrDrive\include\* /f /s
del release\pio\library\ftSwarmPwrDrive\src\main.cpp
del release\pio\library\ftSwarmPwrDrive\src\ftSwarm.h
del release\pio\library\ftSwarmPwrDrive\src\ftSwarmRS.h
del release\pio\library\ftSwarmPwrDrive\src\ftSwarmControl.h
del release\pio\library\ftSwarmPwrDrive\src\ftSwarmCAM.h
del release\pio\library\ftSwarmPwrDrive\src\ftSwarmPwrDrive.h
