#version 330 core

in vec2 v_out_tex_coords;

out vec4 result_color;
uniform sampler2D input_texture;

void main()
{
	result_color = vec4(texture(input_texture, v_out_tex_coords));
}