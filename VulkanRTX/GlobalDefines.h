#pragma once

#ifdef RTX
	#define OPTIX_DENOISER
#endif

#ifdef ENABLE_VALIDATION
	#define ENABLE_API_DUMP

	#ifdef ENABLE_API_DUMP
		#define ENABLE_DEBUG_MARKERS
	#endif
#endif

#ifdef ENABLE_DEBUG_MARKERS
	#define NAME_OBJECT(object, type, name)	\
		VulkanContext::nameObject(object, type, name);
#else
	#define NAME_OBJECT(object, type, name)
#endif

#define GLM_FORCE_DEPTH_ZERO_TO_ONE