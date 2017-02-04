#version 330 core
in vec2 UV;
out vec4 fragColor;
uniform sampler2D tex;
uniform vec3 addedColor;

void main() {
	fragColor = texture(tex,UV);
	fragColor += fragColor.a > 0.05 ? vec4(addedColor,0) : vec4(0);
} 
