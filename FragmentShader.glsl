#version 330 core

in vec3 fragColor;
in vec3 fragNormal;
in vec3 fragPosition;
in float fragHeight;
in float noiseValue;

uniform vec3 objectColor;
uniform vec3 lightPosition;
uniform vec3 eyePosition;

out vec4 out_color;

void main()
{
	//vec3 lightDir = normalize(lightPosition - fragPosition);
	//float diff = max(dot(fragNormal, lightDir), 0.0);

	//vec3 viewDir = normalize(eyePosition - fragPosition);
	//vec3 reflectDir = reflect(-lightDir, fragNormal);
	//float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);

	//vec3 ambient = 0.2 * fragColor;
	//vec3 diffuse = 0.7 * diff * fragColor;
	//vec3 specular = 0.5 * spec * vec3(1.0, 1.0, 1.0);
	vec3 colorGreen = vec3(0.2, 0.6, 0.2);
    vec3 colorBrown = vec3(0.4, 0.26, 0.13);
	vec3 finalColor = mix(colorGreen, colorBrown, noiseValue);

    out_color = vec4(finalColor, 1.0);
}