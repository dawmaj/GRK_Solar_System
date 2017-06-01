#version 430 core

uniform vec3 lightDir;

in vec3 interpNormal;
in vec2 vert_Tex_Coord;
uniform sampler2D samp_tex;

void main()
{
	vec3 normal = normalize(interpNormal);
	float diffuse = max(dot(normal, -lightDir), 0.0);
	vec3 objectColor = texture2D(samp_tex, vert_Tex_Coord).rgb;
	gl_FragColor = vec4(objectColor * diffuse, 1.0);
}
