varying vec2 UV;
varying vec4 fragColor;
uniform sampler2D tex;
uniform vec3 addedColor;

void main() {
	gl_FragColor = texture2D(tex,UV);
	gl_FragColor += gl_FragColor.a > 0.05 ? vec4(addedColor,0) : vec4(0);
}
