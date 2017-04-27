//Birds Vertex shader

#version 400

uniform mat4 mvp;
uniform mat4 m;

layout(location=0) in vec3 vs_position_in;

void main()
{
	// Transforming The Vertex


	vec4 vIn = vec4(vs_position_in,1);
	vec4 posWorld = m * vIn;

	vIn.z += sin(posWorld.x);

	gl_Position = mvp * vIn;
}