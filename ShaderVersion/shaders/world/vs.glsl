//World Vertex shader

#version 400

uniform mat4 m;
uniform mat4 v;
uniform mat4 p;
uniform mat4 nmat;

uniform float elapsed;
uniform float slider_0;


uniform int reflexionMapPass;
uniform float water_height;

layout(location=0) in vec3 vs_position_in;
layout(location=1) in float vs_type_in;
layout(location=2) in vec3 vs_normal_in;
layout(location=3) in vec2 vs_uv_in;

out vec3 normal;
out vec4 color;
out vec4 worldPos;
out vec2 uvModel;
flat out int type;

#define CUBE_HERBE 0.0
#define CUBE_TERRE 1.0
#define CUBE_EAU 4.0 

float noiseWater(vec2 v )
{
	return (-5 + ((sin(v.x/30.0 + elapsed*3.0) + (sin((v.x+v.y)/15 + elapsed*2.0)/2.0) + (sin((v.y)/7.5 + elapsed*1.2)/4.0)) * 2));
}

void main()
{

	vec4 posIn = vec4(vs_position_in,1.0);
	vec4 posWorld = m * posIn;
	worldPos = posWorld;
	vec4 posView = v * posWorld;

	if(reflexionMapPass > 0)
		gl_ClipDistance[0] = worldPos.z - water_height;
	else
		gl_ClipDistance[0] = 1;


	if(vs_type_in == CUBE_EAU)
		posIn.z = posIn.z + noiseWater(posWorld.xy);

	float disXZ = length(posView.xz);
	posIn.z = posIn.z - pow(disXZ/(10 + 75 * slider_0),2);

	type = int(vs_type_in);

	gl_Position = p * v * m * posIn;

	normal = vec3(nmat * vec4(vs_normal_in,1.0));
	uvModel = vs_uv_in;

	if(vs_type_in == CUBE_HERBE)
		color = vec4(0,1,0,1);
	if(vs_type_in == CUBE_TERRE)
		color = vec4(1,0.6,0.2,1);
	if(vs_type_in == CUBE_EAU)
		color = vec4(0.5,0.5,1,0.9);

}