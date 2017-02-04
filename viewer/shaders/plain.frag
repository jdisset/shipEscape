varying vec2 UV;
varying vec4 fragColor;
uniform vec4 color1;
uniform vec4 color2;

void main() {
	gl_FragColor =  mix(color1,color2,UV.x);
}

