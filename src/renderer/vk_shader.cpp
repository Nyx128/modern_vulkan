#include "vk_shader.hpp"
#include "../utils/fileio.hpp"

VkShader::VkShader(std::string _vertex_filename, std::string _fragment_filename, VkCtx& _ctx)
:vertex_filename(_vertex_filename), fragment_filename(_fragment_filename), ctx(_ctx){

	auto device = ctx.get_device();

	vk::ShaderCreateFlagsEXT flags = vk::ShaderCreateFlagBitsEXT::eLinkStage;
	vk::ShaderStageFlags next_stage = vk::ShaderStageFlagBits::eFragment;

	std::vector<char> vertex_src = readSPIRV(vertex_filename);
	vk::ShaderCodeTypeEXT code_type = vk::ShaderCodeTypeEXT::eSpirv;
	const char* entry = "main";

	vk::ShaderCreateInfoEXT vertex_ci;
	vertex_ci.setPCode(vertex_src.data());
	vertex_ci.setCodeSize(vertex_src.size());
	vertex_ci.setFlags(flags);
	vertex_ci.setStage(vk::ShaderStageFlagBits::eVertex);
	vertex_ci.setNextStage(next_stage);
	vertex_ci.setCodeType(code_type);
	vertex_ci.setPName(entry);

	std::vector<char> fragment_src = readSPIRV(fragment_filename);
	vk::ShaderCreateInfoEXT fragment_ci;
	fragment_ci.setFlags(flags);
	fragment_ci.setStage(vk::ShaderStageFlagBits::eFragment);
	fragment_ci.setCodeType(code_type);
	fragment_ci.setCodeSize(fragment_src.size());
	fragment_ci.setPCode(fragment_src.data());
	fragment_ci.setPName(entry);

	auto result = device.createShaderEXT(vertex_ci);
	if (result.result != vk::Result::eSuccess) {
		throw std::runtime_error("failed to create vertex shader: " + vertex_filename);
	}
	vertex_shader = result.value;

	result = device.createShaderEXT(fragment_ci);
	if (result.result != vk::Result::eSuccess) {
		throw std::runtime_error("failed to create fragment shader: " + fragment_filename);
	}
	fragment_shader = result.value;
}

VkShader::~VkShader(){
	auto device = ctx.get_device();
	device.destroyShaderEXT(vertex_shader);
	device.destroyShaderEXT(fragment_shader);
}
