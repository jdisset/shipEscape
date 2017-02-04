#version 330 core
in vec2 UV;
out vec4 fragColor;
uniform vec3 color1;
uniform vec3 color2;

float sql(vec2 v){
	return dot(v,v);
}
const float factor = 1.17;
void main() {
	float l = sql(UV - vec2(0.5));

	fragColor =  l <=0.25 ? mix(vec4(color1,1.0), vec4(color2,1.0),l/0.25) : vec4(0);
}

