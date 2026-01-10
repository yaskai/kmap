#version 330

in vec2 frag_texcoord;
in vec3 frag_worldpos;
in vec3 frag_normal;

uniform sampler2D texture0;

out vec4 final_color;

void main() {
	final_color = vec4(1.0, tex_color.a);
}

