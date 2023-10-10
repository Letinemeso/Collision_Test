#version 330 core

in vec4 fs_in_colors;
in vec2 fs_in_tex_coords;

out vec4 result_color;
uniform sampler2D fp_in_texture;

in lowp mat4 fs_in_crop_area;

void test_area(mat4 _matrix)
{
    if(gl_FragCoord.x < _matrix[0][0] || gl_FragCoord.x > _matrix[0][1] || gl_FragCoord.y < _matrix[1][0] || gl_FragCoord.y > _matrix[1][1])
	discard;
}

void main()
{
    test_area(fs_in_crop_area);

    result_color = vec4(texture(fp_in_texture, fs_in_tex_coords) * fs_in_colors);
}
