#version 330 core

layout(location=0) in vec3 inPos;	
layout(location=1) in vec3 inColor;

uniform mat4 modelViewProj;

out vec3 color;


void main()
{
	color = inColor;
	
	gl_Position =  modelViewProj * vec4 (inPos,1.0);
}
