#include "raymarcher.h"

#include <string>
#include <algorithm>

#include <godot_cpp/core/class_db.hpp>

#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/classes/engine.hpp"
#include "godot_cpp/classes/scene_tree.hpp"
#include "godot_cpp/classes/rd_sampler_state.hpp"
#include "godot_cpp/classes/rd_shader_source.hpp"
#include "godot_cpp/classes/rendering_server.hpp"
#include "godot_cpp/classes/rd_shader_spirv.hpp"
#include "godot_cpp/classes/rd_uniform.hpp"
#include "godot_cpp/classes/render_scene_buffers_rd.hpp"
#include "godot_cpp/classes/render_scene_data.hpp"
#include "godot_cpp/classes/uniform_set_cache_rd.hpp"

using namespace godot;

// https://typhomnt.github.io/teaching/ray_tracing/raymarching_intro/
// https://jamie-wong.com/2016/07/15/ray-marching-signed-distance-functions/#rotation-and-translation
constexpr char template_shader[] = R"(
	#version 450

	// Invocations in the (x, y, z) dimension
	layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

	layout(rgba16f, set = 0, binding = 0) uniform image2D color_image;
	// setup layout for usage of more than 128 bypassing Vulkan limit
	layout(set = 0, binding = 1) uniform sampler2D depth_image;

	layout(set = 0, binding = 2, std430) restrict buffer Matrices {
		mat4 projection_matrix;
		mat4 view_matrix;
	} matrices;

	// Our push constant
	layout(push_constant, std430) uniform Params {
		vec2 raster_size;
	} params;

	struct Ray {
		vec3 origin;
		vec3 direction;
		vec3 color;
	};

	struct MarchResult {
		float depth;
		int steps;
		bool hit;
		vec3 color;
		vec3 hit_pos;
		vec3 normal;
	};

	const vec3 light_pos = vec3(10.0, 10.0, 10.0);
	const float ambient_strength = 0.3;
	const float diffuse_strength = 1.0;
	const float specular_strength = 1.0;
	const float specular_pow = 32.0;

	const vec3 obj_color = vec3(0.5, 0.5, 0.5);
	const vec3 glow_color = vec3(0.0, 0.0, 1.0);
	const float glow_strength = 0.1;

	const float EPSILON = 0.0001;
	const float MAX_DEPTH = 1000.0;
	const int MAX_STEPS = 500;


	mat4 rotate_x(float theta) {
		float c = cos(theta);
		float s = sin(theta);

		return mat4(
			vec4(1, 0, 0, 0),
			vec4(0, c, -s, 0),
			vec4(0, s, c, 0),
			vec4(0, 0, 0, 1)
		);
	}

	mat4 rotate_y(float theta) {
		float c = cos(theta);
		float s = sin(theta);

		return mat4(
			vec4(c, 0, s, 0),
			vec4(0, 1, 0, 0),
			vec4(-s, 0, c, 0),
			vec4(0, 0, 0, 1)
		);
	}

	mat4 rotate_z(float theta) {
		float c = cos(theta);
		float s = sin(theta);

		return mat4(
			vec4(c, -s, 0, 0),
			vec4(s, c, 0, 0),
			vec4(0, 0, 1, 0),
			vec4(0, 0, 0, 1)
		);
	}


	float op_union(float d1, float d2) {
		return min(d1, d2);
	}

	float op_sub(float d1, float d2) {
		return max(d1, -d2);
	}

	float op_intersect(float d1, float d2) {
		return max(d1, d2);
	}

	float op_smooth_union(float d1, float d2, float k) {
		float h = clamp(0.5 + 0.5 * (d2 - d1) / k, 0.0, 1.0);
		return mix(d2, d1, h) - k * h * (1.0 - h);
	}

	float sdf(vec3 p) {
		float d = MAX_DEPTH;

		#SDF_SCENE

		return d;
	}

	Ray make_ray(vec3 origin, vec3 direction, vec3 color) {
		Ray ray;
		ray.origin = origin;
		ray.direction = direction;
		ray.color = obj_color;
		return ray;
	}


	vec3 estimate_normal(vec3 point) {
		return normalize(vec3(
			sdf(point + vec3(EPSILON, 0, 0)) - sdf(point - vec3(EPSILON, 0, 0)),
			sdf(point + vec3(0, EPSILON, 0)) - sdf(point - vec3(0, EPSILON, 0)),
			sdf(point + vec3(0, 0, EPSILON)) - sdf(point - vec3(0, 0, EPSILON))
		));
	}

	vec3 get_eye_pos() {
		return matrices.view_matrix[3].xyz;
	}

	float calculate_lighting(vec3 point, vec3 normal) {
		float ambient = 0.4;
		
		vec3 light_dir = normalize(light_pos - point);
		float diff = max(dot(normal, light_dir), 0.0) * diffuse_strength;
		
		vec3 view_dir = normalize(get_eye_pos() - point);
		vec3 reflect_dir = reflect(-light_dir, normal);
		
		float spec = pow(max(dot(view_dir, reflect_dir), 0.0), specular_pow) * specular_strength;
		
		return diff + ambient + spec;
	}

	Ray calculate_ray(vec2 uv) {
		mat4 inv_proj_matrix = inverse(matrices.projection_matrix);
		vec3 origin = get_eye_pos();
		vec3 dir = (inv_proj_matrix * vec4(uv, 0.0, 1.0)).xyz;
		dir = normalize((matrices.view_matrix * vec4(dir, 0.0)).xyz);
		return make_ray(origin, dir, obj_color);
	}

	MarchResult march_along(in Ray ray, int max_steps, float max_depth) {
		MarchResult result;
		result.depth = 0.0;
		result.steps = 0;
		result.color = vec3(1.0);
		result.hit = false;
		result.normal = vec3(0.0);
		
		for (int i = 0; i < max_steps; i++) {
			result.hit_pos = ray.origin + result.depth * ray.direction;
			float dist = sdf(result.hit_pos);
			
			if (dist < EPSILON) {
				result.hit = true;
				result.normal = estimate_normal(result.hit_pos);
				result.color = ray.color * calculate_lighting(result.hit_pos, result.normal);
				break;
			}
			
			result.depth += dist;
			result.steps = i + 1;
			
			if (result.depth >= max_depth) {
				break;
			}
		}
		
		return result;
	}

	vec2 normalize_uvs(vec2 uv) {
		uv -= 0.5;
		uv *= 2.0;
		uv.y = -uv.y;
		return uv;
	}

	// The code we want to execute in each invocation
	void main() {
		ivec2 iuv = ivec2(gl_GlobalInvocationID.xy);
		ivec2 size = ivec2(params.raster_size);

		vec2 uv = vec2(iuv) / vec2(size);
		uv.y = 1.0 - uv.y;

		if (uv.x >= size.x || uv.y >= size.y) {
			return;
		}

		vec4 color = imageLoad(color_image, iuv);
		float depth = texelFetch(depth_image, iuv, 0).r;
		mat4 inv_proj = inverse(matrices.projection_matrix);
		float linear_depth = 1.0 / (depth * inv_proj[2].w + inv_proj[3].w);

		vec2 nuv = normalize_uvs(uv);
		Ray ray = calculate_ray(nuv);
		MarchResult result = march_along(ray, MAX_STEPS, MAX_DEPTH);
		vec3 col = color.rgb;

		if (result.hit && result.depth < linear_depth) {
			// imageStore(depth_image, iuv, result.depth);
			col = result.color;
		}

		// col += glow_color * float(result.steps) * glow_strength;

		color = vec4(col, 1.0);

		imageStore(color_image, iuv, color);
	}
)";

Raymarcher* Raymarcher::m_singleton = nullptr;


void Raymarcher::_bind_methods() {
}

void Raymarcher::_notification(int p_what) const {
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
    	new_shader_code = _generate_shader_code();
        m_dirty = false;
    }
    m_mutex->unlock();

    // We don't have a (new) shader?
    if (new_shader_code.is_empty()) {
        return m_pipeline.is_valid();
    }

    // Apply template.
    new_shader_code = _generate_shader_code();

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

String Raymarcher::_generate_shader_code() {
	String sdf;

	for (auto& compositor : m_compositors) {
		sdf += compositor->gen_sdf();
	}

    return String(template_shader)
		.replace("#SDF_SCENE", sdf);
}

inline void push_matrix(PackedFloat32Array& arr, const Vector4 matrix[4]) {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			arr.push_back(matrix[i][j]);
		}
	}
}

inline PackedFloat32Array create_matrix_from_array(const Vector4 matrix[4]) {
	PackedFloat32Array arr;
	push_matrix(arr, matrix);
	return arr;
}

void Raymarcher::_render_callback(int32_t p_effect_callback_type, RenderData* p_render_data) {
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
		if (render_scene_buffers.is_valid()) {

			// Get our render size, this is the 3D render resolution!
			auto size = render_scene_buffers->get_internal_size();
			if (size.x == 0 && size.y == 0)
				return;

			// We can use a compute shader here.
			uint16_t x_groups = (size.x - 1) / 8 + 1;
			uint16_t y_groups = (size.y - 1) / 8 + 1;
			uint16_t z_groups = 1;

			// Push constant.
			PackedFloat32Array push_constant;
			push_constant.push_back(size.x);
			push_constant.push_back(size.y);
			push_constant.push_back(0.0f);
			push_constant.push_back(0.0f);

			// Loop through views just in case we're doing stereo rendering. No extra cost if this is mono.
			auto view_count = render_scene_buffers->get_view_count();
			for (auto i = 0; i < view_count; i++)
			{
				// Projection Matrix
				Projection projection = scene ? scene->get_view_projection(i) : Projection();
				
				// Derived from https://forum.godotengine.org/t/compositor-problems/83074/2
				Transform3D cam_transform = scene->get_cam_transform();
				PackedFloat32Array view_matrix_data = {
					cam_transform.basis.get_column(0).x, cam_transform.basis.get_column(0).y, cam_transform.basis.get_column(0).z, 0.0, 
					cam_transform.basis.get_column(1).x, cam_transform.basis.get_column(1).y, cam_transform.basis.get_column(1).z, 0.0, 
					cam_transform.basis.get_column(2).x, cam_transform.basis.get_column(2).y, cam_transform.basis.get_column(2).z, 0.0, 
					cam_transform.origin.x, cam_transform.origin.y, cam_transform.origin.z, 1.0, 
				};


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

				PackedFloat32Array matrix_data;
				push_matrix(matrix_data, projection.columns);
				matrix_data.append_array(view_matrix_data);

				auto matrix_buf = m_rd->storage_buffer_create(
					sizeof(Vector4[4]) + sizeof(Vector4[4]),
					matrix_data.to_byte_array()
				);
				
				Ref<RDUniform> matrices;
				matrices.instantiate();
				matrices->set_uniform_type(RenderingDevice::UNIFORM_TYPE_STORAGE_BUFFER);
				matrices->set_binding(2);
				matrices->add_id(matrix_buf);

				auto view_buf = m_rd->storage_buffer_create(sizeof(Vector4[4]), view_matrix_data.to_byte_array());

				Ref<RDUniform> view_matrix;
				view_matrix.instantiate();
				view_matrix->set_uniform_type(RenderingDevice::UNIFORM_TYPE_STORAGE_BUFFER);
				view_matrix->set_binding(3);
				view_matrix->add_id(view_buf);

				auto uniform_set = UniformSetCacheRD::get_cache(m_shader, 0, { color_uniform, depth_uniform, matrices });

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

void godot::Raymarcher::register_compositor(RMCompositor* compositor)
{
	m_compositors.push_back(compositor);

	invalidate_cache();
}

void godot::Raymarcher::unregister_compositor(RMCompositor* compositor)
{
	auto it = std::find(m_compositors.begin(), m_compositors.end(), compositor);
	if (it == m_compositors.end()) {
		return;
	}

	m_compositors.erase(it);

	invalidate_cache();
}

void godot::Raymarcher::invalidate_cache()
{
	m_dirty = true;
}

Raymarcher* godot::Raymarcher::get_singleton()
{
    return m_singleton;
}

Raymarcher::Raymarcher(): m_dirty(true) {
	m_singleton = this;
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
}

Raymarcher::~Raymarcher() {
	m_singleton = nullptr;
}
