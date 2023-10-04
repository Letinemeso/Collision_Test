#version 330 core

layout(location = 0) in vec3 vs_in_pos;
layout(location = 1) in vec4 vs_in_colors;
layout(location = 3) in vec2 vs_in_tex_coords;

uniform mat4 projection_matrix;
uniform mat4 transform_matrix;

uniform mat4 vs_in_crop_area;
varying mat4 fs_in_crop_area;

out vec4 fs_in_colors;
out vec2 fs_in_tex_coords;

void main()
{
        gl_Position = projection_matrix * transform_matrix * vec4(vs_in_pos, 1.0f);

        fs_in_colors = vs_in_colors;
        fs_in_tex_coords = vs_in_tex_coords;

        fs_in_crop_area = vs_in_crop_area;
}
