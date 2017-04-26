rem Sets Houdini environment variables to recognize users own VEX and RENDERMAN shaders
rem The setx utility is available as a free download from MicroSoft.
rem Note A: Edit the drive identifier. This script assumes you are using H:
rem Note B: Edit the last part of HOUDINI_RI_SHADERPATH. This script assumes your
rem         Renderman shaders at located at H:/rman/shaders.
rem Thanks to Matthew Parrott and Jesse Kretshmer for writing this bat file.
setx HOUDINI_PATH H:/;"&"
setx HOUDINI_VEX_PATH H:/SHOP/surface;H:/SHOP/displace;H:/SHOP/light;H:/SHOP/fog;H:/SHOP/shadows;"&"
setx HOUDINI_RI_SHADERPATH ".:&:$HFS/houdini/ri_shaders:H:/rman/shaders"
pause
