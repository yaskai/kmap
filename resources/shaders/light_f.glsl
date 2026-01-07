#version 330

#define MAX_LIGHTS 16

in vec2 frag_texcoord;
in vec3 frag_worldpos;
in vec3 frag_normal;
in mat3 tbn;

// Uniforms (set from game code)
uniform sampler2D texture0;		// Texture to use
uniform sampler2D texture1;		// Normal map texture
uniform vec4 col_diffuse;		// Base color(white by default)
uniform vec3 light_pos;			// Light position(set in game code)
uniform float light_range;		// How far can light travel
uniform int draw_mode;
uniform vec3 view_pos;

uniform int light_enabled[MAX_LIGHTS];
uniform vec3 light_positions[MAX_LIGHTS];
uniform vec3 light_colors[MAX_LIGHTS];
uniform float light_ranges[MAX_LIGHTS];
uniform int light_count;

uniform float time;
uniform vec3 ambient;

// Output color
out vec4 final_color;

float noise(vec2 uv, float t) {
    return fract(sin(dot(uv, vec2(12.9898, 78.233)) + (t * 0.0005)) * 43758.5453);
}

void main() {
	vec3 normal = normalize(frag_normal);
	vec3 view_dir = normalize(view_pos - frag_worldpos);

	if(draw_mode == 2) {
        vec3 tangent_normal = texture(texture1, vec2(frag_texcoord.x, frag_texcoord.y)).rgb;
		tangent_normal = normalize(tangent_normal * 2.0 - 1.0);
		//tangent_normal.g *= -1.0;
		normal = normalize(tangent_normal * tbn);
	}

	float specco = 0.0;

	// Sample the texture at current UV coordinates
	vec4 tex_color = texture(texture0, frag_texcoord);
	if(draw_mode == 1) tex_color = texture(texture1, frag_texcoord);

	vec4 tint = tex_color;
	vec3 total_light = vec3(0);
	for(int i = 0; i < light_count; i++) {
		if(light_enabled[i] == 1) {
			vec3 light_dir = normalize(light_positions[i] - frag_worldpos);
			float dist = distance(light_positions[i], frag_worldpos);

			float breathe = sin(time * 2.0 + float(i) * 3.14) * 0.05 + 1.0;
			
			float dyn_range = light_ranges[i] * breathe;

			float attenuation = 1.0 - smoothstep(0.0, dyn_range, dist);
			float diffuse = max(dot(normal, light_dir), 0.0);


			float ndotl = max(dot(normal, light_dir), 1.0);
			vec3 light_dot = light_colors[i] * ndotl;

			if(ndotl > 0.0) specco = pow(max(0.0, dot(view_dir, reflect(-light_dir, normal))), 16.0);

			//total_light += light_dot;
			total_light += light_colors[i] * diffuse * attenuation;
			total_light += specco;
		}
	}

	total_light += ambient;

	vec3 lit = tint.rgb * total_light;

	float dither = noise(frag_worldpos.xz, time) * 0.025;
	vec3 quantized = ((lit + dither) * 255.0) / 255.0;

    //finalColor = (texelColor*((tint + vec4(specular, 1.0))*vec4(lightDot, 1.0)));
    //finalColor += texelColor*(vec4(1.0, 1.0, 1.0, 1.0)/40.0)*tint;

	final_color = vec4(quantized, tex_color.a);
}

