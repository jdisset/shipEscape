#version 330 core
in vec3 vertex;

out vec2 UV;
uniform mat4 model;
uniform mat4 view;

void main(){
	gl_Position = view*model*vec4(vertex.x, vertex.y, vertex.z, 1.0);
	UV = (vertex.xy+vec2(1.0,1.0))*0.5;
}
