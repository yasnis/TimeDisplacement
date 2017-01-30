#version 120

uniform sampler2DRect texture;
uniform sampler2DRect map;
uniform float scale;
uniform vec2 resolution;

void main(){
	vec2 mapPos = gl_FragCoord.xy*scale;
    vec4 mapColor = texture2DRect(map, mapPos);

	float d = mapColor.r * 49.0;
	int index = int(d);
	float diff = d-index;
	int s = 7;

	vec2 offset0 = vec2(mod(index, s), int(index/s))*resolution;
	index += 1;
	vec2 offset1 = vec2(mod(index, s), int(index/s))*resolution;

	vec4 color0 = texture2DRect(texture, gl_FragCoord.xy+offset0);
	//gl_FragColor = texture2DRect(texture, gl_FragCoord.xy+offset0);
	//return;
	vec4 color1 = texture2DRect(texture, gl_FragCoord.xy+offset1);
	vec4 color = vec4(color0.r+(color1.r-color0.r)*diff, color0.g+(color1.g-color0.g)*diff, color0.b+(color1.b-color0.b)*diff, 1.0);
	gl_FragColor = vec4(color.rgb, 1.0);
}