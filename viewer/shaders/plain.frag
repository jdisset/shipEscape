#version 330 core
in vec2 UV;
out vec4 fragColor;
uniform vec3 color1;
uniform vec3 color2;

void main() {
	fragColor = mix(vec4(color1,1),vec4(color2,1),UV.x);
}

