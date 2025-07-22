python ..\..\nanopb\generator\nanopb_generator.py ./ftSwarm.proto
xcopy /Y ftSwarm.pb.h ..\ftswarm-core\include
xcopy /Y ftSwarm.pb.c ..\ftswarm-core\src
xcopy /Y include\* ..\ftswarm-core\include
xcopy /Y src\* ..\ftswarm-core\src
pause