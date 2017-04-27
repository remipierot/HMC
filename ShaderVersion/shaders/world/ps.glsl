//World Pixel/Fragment shader

#version 400

in vec3 normal;
in vec4 color;
in vec3 wPos;
in vec4 worldPos;
flat in int type;
in vec2 uvModel;


out vec4 p_color;

uniform sampler2D TexCustom_1;
uniform sampler2D TexReflec;

uniform vec4 sun_color;
uniform vec3 light_pos;
uniform float slider_0;
uniform float elapsed;
uniform vec3 cam_pos;
uniform float slider_5;
uniform float screen_width;
uniform float screen_height;

uniform float slider_1;
uniform float slider_2;
uniform float slider_3;

uniform int reflexionMapPass;

#define CUBE_HERBE 0.0
#define CUBE_TERRE 1.0
#define CUBE_EAU 4.0

float noiseWater(vec2 v)
{
	return (-5 + ((sin(v.x/30.0 + elapsed*3.0) + (sin((v.x+v.y)/15 + elapsed*2.0)/2.0) + (sin((v.y)/7.5 + elapsed*1.2)/4.0))* 2));
}

void main()
{
	vec3 camDir = normalize(cam_pos - worldPos.xyz);
	vec3 lightDir = normalize(light_pos - cam_pos);
	vec3 goodNormal = normal;
	vec4 goodColor = color;

	if(type == CUBE_EAU)
	{
		vec4 A = worldPos;
		vec4 B = worldPos + vec4(1,0,0,0);
		vec4 C = worldPos + vec4(0,1,0,0);

		A.z += noiseWater(A.xy);
		B.z += noiseWater(B.xy);
		C.z += noiseWater(C.xy);

		vec3 AB = normalize(B.xyz - A.xyz);
		vec3 AC = normalize(C.xyz - A.xyz);

		goodNormal = normalize(cross(AB,AC));


		vec4 normTexWater = texture2D( TexCustom_1 , uvModel + vec2(elapsed,0) );	
		goodNormal = mix(goodNormal,normalize(normTexWater.xyz),slider_5/10.0);

		vec2 uvReflec = vec2( (gl_FragCoord.x / screen_width) , (gl_FragCoord.y / screen_height) );
		uvReflec.y = 1 - uvReflec.y;
		uvReflec -= goodNormal.xy/15;
		vec3 colReflec = texture2D( TexReflec , uvReflec ).xyz;	

		goodColor.xyz = mix(goodColor.xyz, colReflec, 0.5);

		float fresnel = max(0,dot(goodNormal, camDir));
		fresnel = 1 - fresnel;
		fresnel = pow(fresnel,10 * slider_3);

		goodColor.a = 0.3 * ( goodColor.a * fresnel ) + 0.7;
	}

	float diffuse = max(0,dot(goodNormal,lightDir));

	vec3 halfVec = normalize(camDir + lightDir);
	float specular = max(0,dot(halfVec, goodNormal));
	specular = 10 * slider_2 * pow(specular,500* slider_1);

	if(specular > 0.6) 
		specular = 1;
	else if(specular > 0.1) 
		specular = 0.4;
	
	if(type != CUBE_EAU)
		specular = 0;

	p_color = (diffuse + (1.0-diffuse) * slider_0/4.0) * goodColor;
	p_color += specular * sun_color;

	p_color.a = goodColor.a;
}