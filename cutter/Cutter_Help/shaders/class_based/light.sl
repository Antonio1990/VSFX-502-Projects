/* Shader description goes here */class light_test(	float	intensity = 1;			color	lightcolor = 1){public void light(output vector L;				  output color Cl)	{	// Here is an example of a directional light source	vector direction = vector "shader" (0,0,1);	solar(direction, 0.0) {    	Cl = intensity * lightcolor;    	}	}}