# version 330 core
//in vec3 interpolatedColor;

in vec3 interpolatedNormal;
in vec2 st;

//uniform float time;
uniform mat4 B;

out vec4 finalcolor;

void main () {
    vec3 lightDirection = vec3(0.1, 0.5, 0.1);
    float shading = dot(interpolatedNormal, lightDirection);
    shading = max(0.0, shading); //Clamp negative values to 0.0
    
    //finalcolor = vec4(sin(time+270),sin(time+180),sin(time+90),1.0)
    //Gammal innan 2.3.5
    //finalcolor = vec4(interpolatedColor[2], sin(time), interpolatedColor[1], 1.0);

    finalcolor = vec4(vec3(shading), 1.0);
    
    vec3 L = vec3(0.1, 1, 0.1); // vec3 L is the light direction
    vec3 V = vec3(0 , 0, 1); // vec3 V is the view direction - (0 ,0 ,1) in view space
    vec3 N = interpolatedNormal; // vec3 N is the normal
    float n = 3; // float n is the " shininess " parameter
    vec3 ka = vec3(0.1, 0.1, 0.1); // vec3 ka is the ambient reflection color
    vec3 Ia = vec3(0.1, 0.1, 0.1); // vec3 Ia is the ambient illumination color
    vec3 kd = vec3(1, 0.6, 0); // vec3 kd is the diffuse surface reflection color
    vec3 Id = vec3(1, 0.6, 1); // vec3 Id is the diffuse illumination color
    vec3 ks = vec3(1, 1, 1); // vec3 ks is the specular surface reflection color
    vec3 Is = vec3(1, 1, 1); // vec3 Is is the specular illumination color

    vec3 R = 2.0*dot(N,L)*N - L;// vec3 R is the computed reflection direction
    float dotNL = max(dot(N, L), 0.0);
    float dotRV = max(dot(R, V), 0.0);
    if(dotNL == 0.0){
        dotRV = 0.0;
    }
    vec3 shadedColor = Ia*ka + Id*kd*dotNL + Is*ks*pow(dotRV, n);
    finalcolor = vec4(shadedColor, 1.0);
}