#version 330 core

in vec4 fs_in_colors;
in vec2 fs_in_tex_coords;

out vec4 result_color;
uniform sampler2D fp_in_texture;

varying lowp mat4 fs_in_crop_area;

void main()
{
    if(gl_FragCoord.x < fs_in_crop_area[0][0] || gl_FragCoord.x > fs_in_crop_area[0][1] || gl_FragCoord.y < fs_in_crop_area[1][0] || gl_FragCoord.y > fs_in_crop_area[1][1])
        discard;

    result_color = vec4(texture(fp_in_texture, fs_in_tex_coords) * fs_in_colors);
}
