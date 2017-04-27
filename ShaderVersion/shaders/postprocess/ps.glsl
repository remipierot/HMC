//Postprocess Pixel/Fragment shader

#version 120

uniform sampler2D TexColor;
uniform sampler2D TexDepth;
uniform float screen_width;
uniform float screen_height;
uniform float slider_6;
uniform vec3 cam_pos;
uniform vec3 light_pos;
uniform mat4 prev_V; 
uniform mat4 prev_P;
uniform mat4 V_light;
uniform mat4 P_light;
uniform vec4 bg_color;
uniform float elapsed;

in vec4 gl_FragCoord;

float LinearizeDepth(float z)
{
	float n = 0.5;
	float f = 10000.0;
	return ( 2.0 * n ) / ( f + n - z * ( f - n) );
}

// --- Perlin noise by inigo quilez - iq/2013   https://www.shadertoy.com/view/XdXGW8
vec2 hash(vec2 p)
{
	p = vec2( dot(p,vec2(127.1,311.7)),
			  dot(p,vec2(269.5,183.3)) );

	return -1. + 2.*fract(sin(p)*43758.5453123);
}

float noise(vec2 p)
{
    vec2 i = floor( p );
    vec2 f = fract( p );
	
	vec2 u = f*f*(3.-2.*f);

    mat2  R = mat2(1,0,0,1);

    if (mod(i.x+i.y,2.0)==0.0) R=-R;

    return 2.0*mix( mix( dot( hash( i + vec2(0,0) ), (f - vec2(0,0))*R ), 
                        dot( hash( i + vec2(1,0) ),-(f - vec2(1,0))*R ), u.x),
                   mix( dot( hash( i + vec2(0,1) ),-(f - vec2(0,1))*R ), 
                        dot( hash( i + vec2(1,1) ), (f - vec2(1,1))*R ), u.x), u.y);
}

vec4 early_turb( vec2 uv )
{
	uv = uv * 3.0/2.0;
    vec4 N;
    mat2 m = mat2( 1.6,  1.2, -1.2,  1.6 );
    float level = 1.;
    N[0] = noise(uv); 
    uv = m*uv; level++;
    N[1] = noise(uv); 
    uv = m*uv; level++;
    N[2] = noise(uv);
    uv = m*uv; level++;
    N[3] = noise(uv);
    uv = m*uv; level++;
    return N;
}

vec2 CS( float a )
{
	vec2 cs = vec2(cos(a),sin(a));
	return cs;
}  

mat2 rot( float a)
{
	mat2 rot = mat2(cos(a),-sin(a),sin(a),cos(a));
	return rot;
} 

void main (void)
{
	vec2 pixSize = vec2(1 / screen_width, 1 / screen_height);
	

	vec4 color = texture2D( TexColor , vec2( gl_TexCoord[0] ) );	
	float depth = texture2D( TexDepth , vec2( gl_TexCoord[0] ) ).r;	

	float lum = 0.333*color.r + 0.333*color.g + 0.333*color.b;

	depth = LinearizeDepth(depth);

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

	//Couleur du fond
	if(depth >= 0.5)
	{
 		color.rgb = vec3(0, 0, 0);

 		float 	t = mod(elapsed, 6.283), 
				Kt = 0.0,
				radius = 2.0;
		vec2	R = vec2(screen_width, screen_height),
				uv = gl_FragCoord.xy / R.y, 
				uvl,
				Pl;

		color.rgb -= color.rgb;

		for (float k=-1.0; k<3.0; k++) 
		{ 
			float l,K,v;

			if (k<0.0) 
			{ 
				uvl = uv; 
				Pl = vec2(0); 
				K = 0.3; 
				v = 0.0;
			}
			else 
			{                                     
				Pl  = radius*CS(2.1*k - 0.1*elapsed), 
				uvl = uv-Pl;                       
				l = length(uvl),
				K = exp(-0.5*l*l/radius/radius),            
				v = 3.0/(0.01+l);                           
			}
			Kt += K*K;

			for (float i=0.0; i<3.0; i++) 
			{  
				float ti = t+ 6.283/3.0*i,
				wi = K* (0.5-0.5*cos(ti))/1.5;

				vec2 uvi = uvl*rot(0.3*(-0.5+fract(ti/6.283))*v); 
				color.rgb += vec3(early_turb (0.5 + uvi )  * wi);
			}
		}
		color.rgb /= sqrt(Kt); 
	}

	color.rgb = sqrt(color.rgb);

	gl_FragColor =  vec4(color.rgb,1.0);

	//God rays
	//Ne fonctionne pas (je n'arrive pas à récupérer correctement la position du soleil en screen space)
	/*
	float density = 1.0;
    float weight = 0.01;
    float decay = 1.0;
    float exposure = 1.0;
    int numSamples = 100;
    vec4 screenSpaceLightPos = P_light * V_light * vec4(light_pos, 1.0);
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

