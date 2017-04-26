rem Maintained by Cutter.
rem Used only for building an executible based on a non Pixar Devkit .c or .cpp
rem source code file.
rem Malcolm Kesson Mar 19 2015
rem
cl -nologo -MT -TP -DWIN32 _OTHER_INCLUDE_DIRS_ -c _OTHER_SRCS_ _SRC_FILENAME_
link -nologo -out:_EXE_NAME_.exe _EXE_NAME_.obj _OTHER_OBJS_
del *.obj