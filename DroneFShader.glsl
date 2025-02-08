#version 330

in vec3 fragNormal;
in vec3 fragPosition;
in vec3 fragColor;

out vec4 outColor;

uniform vec3 objectColor;

void main()
{
    

    vec3 result =  fragColor;
    outColor = vec4(result, 1.0);
}
