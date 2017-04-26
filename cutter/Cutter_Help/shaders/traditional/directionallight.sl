/* The light should be instanced in a rib file as follows,
	TransformBegin
        Rotate -90 1 0 0
        Scale 1 1 -1 # <<-- note the negative z scaling
        LightSource "directional_test" 2 "intensity" 1
    TransformEnd
   For more information refer to,
   	http://fundza.com/rman_shaders/lights/directional/index.html#maya_houdini
*/
light
directional_test(float  intensity = 1;
              	color  	lightcolor = 1;
            )
{
vector direction = vector "shader" (0,0,1);
solar(direction, 0.0) {
    Cl = intensity * lightcolor;
    }
}
