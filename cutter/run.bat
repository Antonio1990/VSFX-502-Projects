set PYTHONPATH=%PYTHONPATH%;%RMANTREE\bin
set RMS_SCRIPT_PATHS=%MAYA_APP_DIR%\projects\RfM_ini
set MAYA_USER_DIR=%MAYA_APP_DIR%
path=%PATH%;%RMSTREE%\bin
java -Xms512m -Xmx512m -classpath .;cutter.jar Cutter
