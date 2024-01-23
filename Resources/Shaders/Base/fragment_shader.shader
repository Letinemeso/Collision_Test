in vec4 fs_in_colors;
in vec2 fs_in_tex_coords;

out vec4 result_color;
uniform sampler2D fp_in_texture;

void process_color()
{
    result_color = vec4(texture(fp_in_texture, fs_in_tex_coords) * fs_in_colors);
}
