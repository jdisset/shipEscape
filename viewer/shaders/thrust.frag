#version 330 core
in vec2 UV;
out vec4 fragColor;
uniform vec3 color;

const float factor = 1.25;
void main() {
	float nL = length(UV - vec2(0.5f,0.5)) *2.0 ;
	nL += factor;
	nL *= nL;
	nL = min(nL - factor, 0.9);
	fragColor =  nL < 1.0 ? vec4(color,0.9-nL) : vec4(0);
}

