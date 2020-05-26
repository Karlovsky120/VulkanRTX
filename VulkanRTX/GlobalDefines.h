#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#ifdef ENABLE_DEBUG_MARKERS
	#define NAME_OBJECT(object, type, name)	\
		VulkanContext::nameObject(object, type, name);
#else
	#define NAME_OBJECT(object, type, name)
#endif