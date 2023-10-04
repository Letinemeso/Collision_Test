#version 330 core

in vec4 fs_in_colors;
in vec2 fs_in_tex_coords;

out vec4 result_color;
uniform sampler2D input_texture;

void main()
{
    result_color = vec4(texture(input_texture, fs_in_tex_coords) * fs_in_colors);
}
