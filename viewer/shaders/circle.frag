varying vec2 UV;
varying vec4 fragColor;
uniform vec4 color1;
uniform vec4 color2;

float sql(vec2 v){
	return dot(v,v);
}

const float factor = 1.17;
void main() {
	float l = sql(UV - vec2(0.5));
	gl_FragColor =  l <= 0.25 ? mix(color1, color2,l/0.25) : vec4(0);
}

