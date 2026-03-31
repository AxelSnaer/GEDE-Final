#include "raymarcher.h"

#include <string>
#include <godot_cpp/core/class_db.hpp>

#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/classes/rd_sampler_state.hpp"
#include "godot_cpp/classes/rd_shader_source.hpp"
#include "godot_cpp/classes/rendering_server.hpp"
#include "godot_cpp/classes/rd_shader_spirv.hpp"
#include "godot_cpp/classes/rd_uniform.hpp"
#include "godot_cpp/classes/render_scene_buffers_rd.hpp"
#include "godot_cpp/classes/render_scene_data.hpp"
#include "godot_cpp/classes/uniform_set_cache_rd.hpp"

using namespace godot;

void Raymarcher::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_shader_code"), &Raymarcher::get_shader_code);
	ClassDB::bind_method(D_METHOD("set_shader_code", "shader_code"), &Raymarcher::set_shader_code);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "shader_code", PROPERTY_HINT_MULTILINE_TEXT), "set_shader_code", "get_shader_code");
}

String Raymarcher::get_shader_code() {
    return m_shader_code;
}

void Raymarcher::set_shader_code(String code) {
    m_mutex->lock();
    m_shader_code = code;
    m_dirty = true;
    m_mutex->unlock();
}

void Raymarcher::_notification(int p_what) const
{
    if (p_what == NOTIFICATION_PREDELETE)
    {
        if (m_sampler.is_valid()) {
            m_rd->free_rid(m_sampler);
        }
        if (m_shader.is_valid()) {
            m_rd->free_rid(m_shader);
        }
    }
}

bool Raymarcher::_check_shader() {
    // Check if our shader has changed and needs to be recompiled.
    if (m_rd == nullptr) {
        return false;
    }

    String new_shader_code;

    // Check if our shader is dirty.
    m_mutex->lock();
    if (m_dirty) {
        new_shader_code = m_shader_code;
        m_dirty = false;
    }
    m_mutex->unlock();

    // We don't have a (new) shader?
    if (new_shader_code.is_empty()) {
        return m_pipeline.is_valid();
    }

    // Apply template.
    auto tmp = String(template_shader);
    new_shader_code = tmp.replace("#COMPUTE_CODE", new_shader_code);

    // Out with the old.
    if (m_shader.is_valid()) {
        m_rd->free_rid(m_shader);
        m_shader = RID();
        m_pipeline = RID();
    }

    // In with the new.
    Ref<RDShaderSource> shader_source;
    shader_source.instantiate();
    shader_source->set_language(RenderingDevice::SHADER_LANGUAGE_GLSL);
    shader_source->set_stage_source(RenderingDevice::SHADER_STAGE_COMPUTE, new_shader_code);
    auto shader_spirv = m_rd->shader_compile_spirv_from_source(shader_source);

    if (!shader_spirv->get_stage_compile_error(RenderingDevice::SHADER_STAGE_COMPUTE).is_empty()) {
        ERR_PRINT(shader_spirv->get_stage_compile_error(RenderingDevice::SHADER_STAGE_COMPUTE));
        ERR_PRINT(String("In: ") + new_shader_code);
        return false;
    }

    m_shader = m_rd->shader_create_from_spirv(shader_spirv);
    if (!m_shader.is_valid()) {
        return false;
    }

    m_pipeline = m_rd->compute_pipeline_create(m_shader);
    return m_pipeline.is_valid();
}

void Raymarcher::_render_callback(int32_t p_effect_callback_type, RenderData* p_render_data)
{
    CompositorEffect::_render_callback(p_effect_callback_type, p_render_data);
    if (m_rd != nullptr && p_effect_callback_type == EFFECT_CALLBACK_TYPE_PRE_TRANSPARENT && _check_shader()) {
		// Get our render scene buffers object, this gives us access to our render buffers.
		// Note that implementation differs per renderer hence the need for the cast.
		Vector3 cam_pos(0, 0, 0);
		Vector3 cam_forward(0, 0, -1);
		Vector3 cam_up(0, 1, 0);
		// there is no cam_right but we can get right with cross multiplying forward and up
		auto scene = p_render_data->get_render_scene_data();
		if (scene) {
			Transform3D cam_transformation = scene->get_cam_transform();
			cam_pos = cam_transformation.origin;
			cam_forward = -cam_transformation.basis.rows[2];
			cam_up = cam_transformation.basis.rows[1];
		}

		Ref<RenderSceneBuffersRD> render_scene_buffers = p_render_data->get_render_scene_buffers();
		if (render_scene_buffers.is_valid())
		{
			// Get our render size, this is the 3D render resolution!
			auto size = render_scene_buffers->get_internal_size();
			if (size.x == 0 && size.y == 0)
			{
				return;
			}

			// We can use a compute shader here.
			uint16_t x_groups = (size.x - 1) / 8 + 1;
			uint16_t y_groups = (size.y - 1) / 8 + 1;
			uint16_t z_groups = 1;

			// Push constant.
			PackedFloat32Array push_constant;
			push_constant.push_back(size.x);
			push_constant.push_back(size.y);
			push_constant.push_back(0.0);
			push_constant.push_back(0.0);
			// camera pos
			push_constant.push_back(cam_pos.x);
			push_constant.push_back(cam_pos.y);
			push_constant.push_back(cam_pos.z);
			push_constant.push_back(0);
			// camera orientation
			push_constant.push_back(cam_forward.x);
			push_constant.push_back(cam_forward.y);
			push_constant.push_back(cam_forward.z);
			push_constant.push_back(0);
			push_constant.push_back(cam_up.x);
			push_constant.push_back(cam_up.y);
			push_constant.push_back(cam_up.z);
			push_constant.push_back(0);

			// Loop through views just in case we're doing stereo rendering. No extra cost if this is mono.
			auto view_count = render_scene_buffers->get_view_count();
			for (auto i = 0; i < view_count; i++)
			{
				// Projection Matrix
				Projection projection = scene ? scene->get_view_projection(i) : Projection();
				// push to matrix for each value combination
				push_constant.push_back(projection.columns[0].x);
				push_constant.push_back(projection.columns[0].y);
				push_constant.push_back(projection.columns[0].z);
				push_constant.push_back(projection.columns[0].w);

				push_constant.push_back(projection.columns[1].x);
				push_constant.push_back(projection.columns[1].y);
				push_constant.push_back(projection.columns[1].z);
				push_constant.push_back(projection.columns[1].w);

				push_constant.push_back(projection.columns[2].x);
				push_constant.push_back(projection.columns[2].y);
				push_constant.push_back(projection.columns[2].z);
				push_constant.push_back(projection.columns[2].w);

				push_constant.push_back(projection.columns[3].x);
				push_constant.push_back(projection.columns[3].y);
				push_constant.push_back(projection.columns[3].z);
				push_constant.push_back(projection.columns[3].w);


				// Create a uniform set for color and depth

				// Get the RID for our color image, we will be reading from and writing to it.
				auto input_image = render_scene_buffers->get_color_layer(i);
				// This will be cached; the cache will be cleared if our viewport's configuration is changed.
				Ref<RDUniform> color_uniform;
				color_uniform.instantiate();
				color_uniform->set_uniform_type(RenderingDevice::UNIFORM_TYPE_IMAGE);
				color_uniform->set_binding(0);
				color_uniform->add_id(input_image);

				auto depth_image = render_scene_buffers->get_depth_layer(i);
				Ref<RDUniform> depth_uniform;
				depth_uniform.instantiate();
				depth_uniform->set_uniform_type(RenderingDevice::UNIFORM_TYPE_SAMPLER_WITH_TEXTURE);
				depth_uniform->set_binding(1);
				depth_uniform->add_id(m_sampler);
				depth_uniform->add_id(depth_image);

				auto uniform_set = UniformSetCacheRD::get_cache(m_shader, 0, { color_uniform, depth_uniform });

				// Run our compute shader.
				auto compute_list = m_rd->compute_list_begin();
				m_rd->compute_list_bind_compute_pipeline(compute_list, m_pipeline);
				m_rd->compute_list_bind_uniform_set(compute_list, uniform_set, 0);
				m_rd->compute_list_set_push_constant(compute_list, push_constant.to_byte_array(), push_constant.size() * 4);
				m_rd->compute_list_dispatch(compute_list, x_groups, y_groups, z_groups);
				m_rd->compute_list_end();
				// this resizes for the matrix
				push_constant.resize(16);
			}
		}
    }

}

Raymarcher::Raymarcher():  m_shader_code(""), m_dirty(true) {
    m_mutex.instantiate();
    set_effect_callback_type(EFFECT_CALLBACK_TYPE_PRE_TRANSPARENT);
    m_rd = RenderingServer::get_singleton()->get_rendering_device();
    if (m_rd != nullptr) {
        Ref<RDSamplerState> sampler_state;
        sampler_state.instantiate();
        sampler_state->set_min_filter(RenderingDevice::SAMPLER_FILTER_NEAREST);
        sampler_state->set_mag_filter(RenderingDevice::SAMPLER_FILTER_NEAREST);
        m_sampler = m_rd->sampler_create(sampler_state);
    }
    // Initialize any variables here.
}

Raymarcher::~Raymarcher() {
    // Add your cleanup here.
}
