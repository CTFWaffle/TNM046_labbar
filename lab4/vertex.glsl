# version 330 core
layout(location=0) in vec3 Position;
layout(location=1) in vec3 Normal;
layout(location=2) in vec2 TexCoord;

out vec3 interpolatedNormal;
out vec2 st;


//uniform mat4 B;
uniform mat4 MV;
uniform mat4 P;

void main(){
    gl_Position = MV* vec4(Position , 1.0); //Special, required output
    vec3 transformedNormal =  mat3(MV) *  Normal;
    interpolatedNormal = normalize(transformedNormal);
    //interpolatedNormal = Normal; //Will be interpolated across the triangle
    st = TexCoord; // Will also be interpolated across the triangle
}

/* Gammal innan 2.3.5
layout ( location = 0) in vec3 Position ;
layout (location = 1) in vec3 Color;
out vec3 interpolatedColor;
uniform float time;
//uniform mat4 T;
//uniform mat4 A;


void main () {
    interpolatedColor = Color;
    //gl_Position = vec4 ( Position , ) ;
    // Transform (x,y,z) vertex coordinates with a 4x4 matrix T
    gl_Position = B * vec4 ( Position, 1) ;

}
*/