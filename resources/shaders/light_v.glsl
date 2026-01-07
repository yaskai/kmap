#version 330

in vec3 vertex_position; // 3D world position of vertex
in vec2 vertex_texcoord; // UV texture coordinates
in vec3 vertex_normal;
in vec4 vertex_tangent;

uniform mat4 mvp;			// Model view projection matrix 
uniform mat4 mat_model;

// Outputs to fragment shader
out vec2 frag_texcoord;	// Pass texture coordinates to fragment shader
out vec3 frag_worldpos;
out vec3 frag_normal;
out mat3 tbn;

void main() {
	vec3 vertex_binormal = cross(vertex_normal, vertex_tangent.xyz) * vertex_tangent.w;

	// Forward texture coordinates
	frag_texcoord = vertex_texcoord;
	frag_worldpos = vec3(mat_model * vec4(vertex_position, 1.0));

    mat3 normal_matrix = transpose(inverse(mat3(mat_model)));
    frag_normal = normalize(normal_matrix * vertex_normal);

	vec3 frag_tangent = normalize(normal_matrix * vertex_normal);
	frag_tangent = normalize(frag_tangent - dot(frag_tangent, frag_normal) * frag_normal);

	vec3 frag_binormal = normalize(normal_matrix * vertex_binormal);
	frag_binormal = cross(frag_normal, frag_tangent);

	tbn = transpose(mat3(frag_tangent, frag_binormal, frag_normal));

	gl_Position = mvp * vec4(vertex_position, 1.0);
}
