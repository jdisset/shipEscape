attribute vec3 vertex;

varying highp vec2 UV;
uniform highp mat4 model;
uniform highp mat4 view;

void main(){
	UV = (vertex.xy+vec2(1.0,1.0))*0.5;
	gl_Position = view*model*vec4(vertex.x, vertex.y, vertex.z, 1.0);
}
