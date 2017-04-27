//Postprocess Pixel/Fragment shader

#version 120

uniform sampler2D TexColor;
uniform sampler2D TexDepth;
uniform float screen_width;
uniform float screen_height;
uniform float slider_6;
uniform vec3 cam_pos; //Position de la camera
uniform vec3 light_pos; //Position du soleil
uniform mat4 prev_V; //matrice vue de la passe 4
uniform mat4 prev_P; //matrice projection de la passe 4
uniform mat4 V_light; //Matrice vue de la lumière, utilisée lors du rendu de la shadow map
uniform mat4 P_light; //Matrice projection de la lumière, utilisée lors du rendu de la shadow map
uniform vec4 bg_color; //Couleur du fond (ciel)
uniform float elapsed;

in vec4 gl_FragCoord; //que fragment shader, position du fragment à l'écran origine bas gauche, en pixels.

float LinearizeDepth(float z)
{
	float n = 0.5;
	float f = 10000.0;
	return ( 2.0 * n ) / ( f + n - z * ( f - n) );
}

void main (void)
{
	vec2 pixSize = vec2(1 / screen_width, 1 / screen_height);
	

	vec4 color = texture2D( TexColor , vec2( gl_TexCoord[0] ) );	
	float depth = texture2D( TexDepth , vec2( gl_TexCoord[0] ) ).r;	

	float lum = 0.333*color.r + 0.333*color.g + 0.333*color.b;

	depth = LinearizeDepth(depth);

	//DETECTION CONTOUR
	float depthAccum = 0;
	float distContour = 3;
	float nb = 0;	

	for(float x = -distContour; x < distContour; x++)
	{
		for(float y = -distContour; y < distContour ; y++)
		{	
			float d = texture2D( TexDepth , vec2( gl_TexCoord[0]) + vec2(x, y)*pixSize  ).r;	

			depthAccum -= LinearizeDepth(d);
			nb++;
		}
	}

	depthAccum += nb*depth;
	depthAccum = pow(clamp(depthAccum*30,0,1),1+2*slider_6);
	
	color.rgb -= vec3(depthAccum,depthAccum,depthAccum);
 	//FIN CONTOUR

	if(depth >= 0.5)
 		color.rgb = vec3(0, 0, 0);

	//Correction gamma
	color.rgb = sqrt(color.rgb);

	gl_FragColor =  vec4(color.rgb,1.0);

	//God rays
	/*
	float density = 1.0;
    float weight = 0.01;
    float decay = 1.0;
    float exposure = 1.0;
    int numSamples = 100;
    vec4 screenSpaceLightPos = P_light * V_light * vec4(light_pos, 1.0) * 10;
    vec2 textCoo = gl_TexCoord[0].xy;
	vec2 deltaTextCoord = vec2(textCoo.xy - screenSpaceLightPos.xy);
	deltaTextCoord *= (1.0 /  float(numSamples)) * density;
	float illuminationDecay = 1.0;

	for(int i=0; i < numSamples ; i++)
	{
		textCoo -= deltaTextCoord;
		vec4 sample = texture2D(TexColor, textCoo);
		sample *= illuminationDecay * weight;
		gl_FragColor += sample;
		illuminationDecay *= decay;
	}
	gl_FragColor *= exposure;
	*/
}

