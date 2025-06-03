#pragma once
#include "../core/vk_ctx.hpp"

class VkShader {
public:
	VkShader(std::string _vertex_filename, std::string _fragment_filename, VkCtx& _ctx);
	~VkShader();

	VkShader(const VkShader&) = delete;
	VkShader& operator=(const VkShader&) = delete;
	VkShader(VkShader&&) = default;
	VkShader& operator=(VkShader&&) = default;

	vk::ShaderEXT& get_vertex_shader() { return vertex_shader; }
	vk::ShaderEXT& get_fragment_shader() { return fragment_shader; }

private:
	VkCtx& ctx;
	std::string vertex_filename;
	std::string fragment_filename;

	vk::ShaderEXT vertex_shader;
	vk::ShaderEXT fragment_shader;
};