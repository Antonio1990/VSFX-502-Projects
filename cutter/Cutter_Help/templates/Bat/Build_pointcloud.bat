rem Maintained by Cutter.
rem For information about customizing the compilation and linking of devkit
rem DSOs and executables go to,
rem	http://www.fundza.com/cutter/devkit/index.html
rem Malcolm Kesson Sep 27 2016
rem
cl -nologo -MT -TP -DWIN32 -I"_DEVKIT_PATH_\include" _OTHER_INCLUDE_DIRS_ -c _OTHER_SRCS_ _SRC_FILENAME_
link -nologo  -out:_OUT_PATH_ _POINTCLOUD_EXE_NAME_.obj -LIBPATH:"_DEVKIT_PATH_\lib" libprman.lib
del _OUT_OBJ_