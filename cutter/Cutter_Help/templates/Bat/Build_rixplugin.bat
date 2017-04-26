rem Maintained by Cutter.
rem For information about customizing the compilation and linking of devkit
rem DSOs and executables go to,
rem	http://www.fundza.com/cutter/devkit/index.html
rem Malcolm Kesson Nov 21 2016
rem
cl -nologo -MT -DWIN32 -EHsc -I"_DEVKIT_PATH_\include" -c _SRC_FILENAME_
link -nologo -DLL -out:_OUT_PATH_ _RIX_PLUGIN_NAME_.obj "_DEVKIT_PATH_\lib\libprman.lib"
del _OUT_OBJ_
del _OUT_LIB_
del _OUT_EXP_
