#pragma once

#include "GlobalDefines.h"

#ifdef OPTIX_DENOISER

#include "Buffer.h"
#include "CommandBuffer.h"
#include "Image.h"

#include "OptiX/optix_types.h"

#include <memory>

class Denoiser {
public:
	void denoise(
		CommandBuffer& cmdBuffer,
		Image& input,
		vk::ImageLayout oldInputLayout,
		Image& output,
		vk::ImageLayout oldOutputLayout);

	Denoiser();

private:
	struct CudaBuffer {
		std::unique_ptr<Buffer> buffer;
		HANDLE handle = nullptr;
		void* cudaPtr = nullptr;
	};

	void allocateBuffers();
	void createCudaBuffer(CudaBuffer& cudaBuffer);

	vk::Extent2D m_imageSize;

	OptixDenoiser m_denoiser{ nullptr };
	OptixDeviceContext m_optixDevice{};
	OptixDenoiserOptions m_dOptions{};
	OptixDenoiserSizes m_dSizes{};
	CUdeviceptr m_dState{ 0 };
	CUdeviceptr m_dScratch{ 0 };
	CUdeviceptr m_dIntensity{ 0 };
	CUdeviceptr m_dMinRGB{ 0 };

	CudaBuffer m_inputBuffer;
	CudaBuffer m_outputBuffer;

	static void context_log_cb(
		unsigned int level,
		const char* tag,
		const char* message,
		void* /*cbdata */);
};

#endif
