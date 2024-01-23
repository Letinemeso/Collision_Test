#pragma once

#include <mat2x2.hpp>

#include <Window/Window_Controller.h>
#include <Shader/Shader_Components/Shader_Component.h>


class FS_Component__Crop_Area : public LR::Shader_Component
{
private:
    std::string m_crop_area_rect_uniform_name = "fs_crop_area_rect";

    int m_crop_area_rect_uniform = -1;

public:
    void init(unsigned int _opengl_program_handle) override;

public:
    void set_crop_area_rect(const glm::mat2x2& _matrix) const;
    void reset_crop_area() const;

};



void FS_Component__Crop_Area::init(unsigned int _opengl_program_handle)
{
    Shader_Component::init(_opengl_program_handle);

    m_crop_area_rect_uniform = M_extract_uniform_location(m_crop_area_rect_uniform_name);
}

void FS_Component__Crop_Area::set_crop_area_rect(const glm::mat2x2& _matrix) const
{
    glUniformMatrix2fv(m_crop_area_rect_uniform, 1, false, &_matrix[0][0]);
}

void FS_Component__Crop_Area::reset_crop_area() const
{
    glm::mat2x2 reset_to;
    reset_to[0][0] = 0;
    reset_to[0][1] = LR::Window_Controller::get_window_data().width;
    reset_to[1][0] = 0;
    reset_to[1][1] = LR::Window_Controller::get_window_data().height;

    glUniformMatrix2fv(m_crop_area_rect_uniform, 1, false, &reset_to[0][0]);
}
