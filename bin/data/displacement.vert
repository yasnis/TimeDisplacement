#version 120

//uniform float timeValX = 1.0;
//uniform float timeValY = 1.0;
//uniform vec2 mouse;
varying vec2 vTexCoord;

//void main() {
//    vTexCoord =  gl_Vertex.xy;
//    gl_Position = ftransform();
//}

void main(){
	gl_TexCoord[0] = gl_MultiTexCoord0;
	vec4 pos = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex;
	gl_Position = pos;
	gl_FrontColor =  gl_Color;	
	vTexCoord = gl_Vertex.xy;
}

