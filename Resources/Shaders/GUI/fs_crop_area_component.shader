uniform mat2x2 fs_crop_area_rect;

void discard_outside()
{
    float left = fs_crop_area_rect[0][0];
    float right = fs_crop_area_rect[0][1];
    float bottom = fs_crop_area_rect[1][0];
    float top = fs_crop_area_rect[1][1];

    if(gl_FragCoord.x < left)
	discard;
    if(gl_FragCoord.x > right)
	discard;
    if(gl_FragCoord.y < bottom)
	discard;
    if(gl_FragCoord.y > top)
	discard;
}
