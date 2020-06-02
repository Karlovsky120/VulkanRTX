#include "Denoiser.h"

#ifdef OPTIX_DENOISER

#include "VulkanContext.h"

#include "OptiX/optix.h"
#include "OptiX/optix_function_table_definition.h"
#include "OptiX/optix_stubs.h"

#include <cuda.h>
#include <cuda_runtime.h>

#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <sstream>

#define OPTIX_CHECK(call)                                                      \
  do {                                                                         \
    OptixResult res = call;                                                    \
    if (res != OPTIX_SUCCESS) {                                                \
      std::stringstream ss;                                                    \
      ss << "Optix call " << #call << " failed with code " << res           \
         << " (" __FILE__ << ":" << __LINE__ << ")\n";                         \
      std::cerr << ss.str().c_str() << std::endl;                              \
      throw std::runtime_error(ss.str().c_str());                              \
    }                                                                          \
  } while (false)

#define CUDA_CHECK(call)                                                       \
  do {                                                                         \
    cudaError_t error = call;                                                  \
    if (error != cudaSuccess) {                                                \
      std::stringstream ss;                                                    \
      ss << "CUDA call " << #call << " failed with code " << error          \
         << " (" __FILE__ << ":" << __LINE__ << ")\n";                         \
      throw std::runtime_error(ss.str().c_str());                              \
    }                                                                          \
  } while (false)

void Denoiser::denoise(
	CommandBuffer& cmdBuffer,
	Image& input,
	vk::ImageLayout oldInputLayout,
	Image& output,
	vk::ImageLayout oldOutputLayout) {

	if (input.m_imageSize != m_imageSize) {
		m_imageSize = input.m_imageSize;
		allocateBuffers();
	}

	cmdBuffer.copyImageToBuffer(
		input.get(),
		input.m_imageSize,
		oldInputLayout,
		m_inputBuffer.buffer->get());

	cmdBuffer.submitAndWait();

	OptixPixelFormat pixelFormat = OPTIX_PIXEL_FORMAT_FLOAT4;
	uint32_t rowStrideInBytes = 4 * sizeof(float) * m_imageSize.width;

	OptixImage2D inputLayer{
		(CUdeviceptr)m_inputBuffer.cudaPtr,
		m_imageSize.width,
		m_imageSize.height,
		rowStrideInBytes,
		0,
		pixelFormat };

	OptixImage2D outputLayer{
		(CUdeviceptr)m_outputBuffer.cudaPtr,
		m_imageSize.width,
		m_imageSize.height,
		rowStrideInBytes,
		0,
		pixelFormat };

	CUstream stream = nullptr;
	OPTIX_CHECK(optixDenoiserComputeIntensity(
		m_denoiser,
		stream,
		&inputLayer,
		m_dIntensity,
		m_dScratch,
		m_dSizes.recommendedScratchSizeInBytes));

	OptixDenoiserParams params{};
	params.denoiseAlpha = 1;
	params.hdrIntensity = m_dIntensity;

	OPTIX_CHECK(optixDenoiserInvoke(
		m_denoiser,
		stream,
		&params,
		m_dState,
		m_dSizes.stateSizeInBytes,
		&inputLayer,
		1,
		0,
		0,
		&outputLayer,
		m_dScratch,
		m_dSizes.recommendedScratchSizeInBytes));

	CUDA_CHECK(cudaStreamSynchronize(nullptr));

	cmdBuffer.reset();

	cmdBuffer.copyBufferToImage(
		m_outputBuffer.buffer->get(),
		output.get(),
		input.m_imageSize,
		oldOutputLayout);
}

Denoiser::Denoiser() {
	CUDA_CHECK(cudaFree(nullptr));

	CUcontext cuCtx;
	
	if (cuCtxGetCurrent(&cuCtx) != CUDA_SUCCESS) {
		throw std::runtime_error("Failed to initialize CUDA");
	}

	OPTIX_CHECK(optixInit());
	OPTIX_CHECK(optixDeviceContextCreate(cuCtx, nullptr, &m_optixDevice));
	OPTIX_CHECK(optixDeviceContextSetLogCallback(m_optixDevice, contextLogCB, nullptr, 4));

	m_dOptions.inputKind = OPTIX_DENOISER_INPUT_RGB;
	m_dOptions.pixelFormat = OPTIX_PIXEL_FORMAT_FLOAT4;
	OPTIX_CHECK(optixDenoiserCreate(m_optixDevice, &m_dOptions, &m_denoiser));
	OPTIX_CHECK(optixDenoiserSetModel(m_denoiser, OPTIX_DENOISER_MODEL_KIND_HDR, nullptr, 0));
}

void Denoiser::allocateBuffers() {
	vk::DeviceSize bufferSize = m_imageSize.width * m_imageSize.height * 4 * sizeof(float);

	m_inputBuffer.buffer = std::make_unique<Buffer>(
		bufferSize,
		vk::BufferUsageFlagBits::eUniformBuffer
		| vk::BufferUsageFlagBits::eTransferDst,
		vk::MemoryPropertyFlagBits::eDeviceLocal,
		"CUDA input buffer",
		vk::MemoryAllocateFlags(),
		true);

	m_outputBuffer.buffer = std::make_unique<Buffer>(
		bufferSize,
		vk::BufferUsageFlagBits::eUniformBuffer
		| vk::BufferUsageFlagBits::eTransferDst
		| vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eDeviceLocal,
		"CUDA output buffer",
		vk::MemoryAllocateFlags(),
		true);
	
	createCudaBuffer(m_inputBuffer);
	createCudaBuffer(m_outputBuffer);

	OPTIX_CHECK(optixDenoiserComputeMemoryResources(
		m_denoiser,
		m_imageSize.width,
		m_imageSize.height,
		&m_dSizes));

	CUDA_CHECK(cudaMalloc((void**)&m_dState, m_dSizes.stateSizeInBytes));
	CUDA_CHECK(cudaMalloc((void**)&m_dScratch, m_dSizes.recommendedScratchSizeInBytes));
	CUDA_CHECK(cudaMalloc((void**)&m_dIntensity, sizeof(float)));
	CUDA_CHECK(cudaMalloc((void**)&m_dMinRGB, 4 * sizeof(float)));

	CUstream stream = nullptr;
	OPTIX_CHECK(optixDenoiserSetup(
		m_denoiser,
		stream,
		m_imageSize.width,
		m_imageSize.height,
		m_dState,
		m_dSizes.stateSizeInBytes,
		m_dScratch,
		m_dSizes.recommendedScratchSizeInBytes));
}

void Denoiser::createCudaBuffer(CudaBuffer& cudaBuffer) {
	vk::MemoryGetWin32HandleInfoKHR handleInfo;
	handleInfo.memory = cudaBuffer.buffer->m_allocId->memory();
	handleInfo.handleType = vk::ExternalMemoryHandleTypeFlagBits::eOpaqueWin32;

	cudaBuffer.handle = VulkanContext::getDevice().getMemoryWin32HandleKHR(handleInfo);

	vk::MemoryRequirements memoryRequirements =
		VulkanContext::getDevice().getBufferMemoryRequirements(
			cudaBuffer.buffer->get());

	cudaExternalMemoryHandleDesc cudaExtMemHandleDesc{};
	cudaExtMemHandleDesc.size = memoryRequirements.size;
	cudaExtMemHandleDesc.type = cudaExternalMemoryHandleTypeOpaqueWin32;
	cudaExtMemHandleDesc.handle.win32.handle = cudaBuffer.handle;

	cudaExternalMemory_t cudaExtMemVertexBuffer{};
	CUDA_CHECK(cudaImportExternalMemory(&cudaExtMemVertexBuffer, &cudaExtMemHandleDesc));

	cudaExternalMemoryBufferDesc cudaExtBufferDesc{};
	cudaExtBufferDesc.offset = cudaBuffer.buffer->m_allocId->offset;
	cudaExtBufferDesc.size = memoryRequirements.size;
	cudaExtBufferDesc.flags = 0;
	CUDA_CHECK(cudaExternalMemoryGetMappedBuffer(
		&cudaBuffer.cudaPtr,
		cudaExtMemVertexBuffer,
		&cudaExtBufferDesc));
}

void Denoiser::contextLogCB(
	unsigned int level,
	const char* tag,
	const char* message,
	void* cbdata) {

	std::cout << "[" << std::setw(2) << level << "][" << std::setw(12) << tag
		<< "]: " << message << "\n";
}

#endif
