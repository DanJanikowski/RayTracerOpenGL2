#version 430 core

in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D tex;

void main() {
	//float gamma = 2.2;
	//vec3 texCol = pow(texture(tex, TexCoords).rgb, vec3(1.0 / gamma));
	vec3 texCol = texture(tex, TexCoords).rgb;
	FragColor = vec4(texCol, 1.0);
}