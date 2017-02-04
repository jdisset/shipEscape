#version 330 core
in vec2 UV;
out vec4 fragColor;
uniform vec3 color;

void main() {
	fragColor = vec4(color,1);
} 

