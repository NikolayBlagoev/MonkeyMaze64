#version 460

layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

layout(location = 2) uniform mat4 mvps[6];

out vec4 fragPos; // Fragment position from GS (output per emitted vertex)

void main() {
    for (int face = 0; face < 6; face++) {
        gl_Layer = face; // Built-in variable that specifies to which face we render.
        for(uint i = 0U; i < 3U; i++) { // For each triangle vertex
            fragPos = gl_in[i].gl_Position;
            gl_Position = mvps[face] * fragPos;
            EmitVertex();
        }    
        EndPrimitive();
    }
}
