//
//  SpoutBinding.cpp
//
//  C ABI implementation wrapping Spout SDK for Java FFM bindings.
//
//  Copyright (c) 2025, Lynn Jarvis. All rights reserved.
//  https://spout.zeal.co
//

#include "SpoutBinding.h"

#include <new>          // std::nothrow
#include "../SpoutGL/SpoutSender.h" // sender-only convenience wrapper

// -----------------------------------------------------------------------
// Create a Spout sender object
// -----------------------------------------------------------------------
SPOUTBINDING_API SpoutHandle spCreateSpout(void)
{
	// SpoutSender is a sender-only wrapper around the full Spout class.
	// It exposes only sender-related functions, which is cleaner for this binding.
	SpoutSender* sender = new (std::nothrow) SpoutSender();
	if (!sender) {
		return nullptr;
	}
	return static_cast<SpoutHandle>(sender);
}

// -----------------------------------------------------------------------
// Send an OpenGL FBO via Spout
// -----------------------------------------------------------------------
SPOUTBINDING_API int spSendFrameBufferObject(SpoutHandle handle, unsigned int fbo, unsigned int width, unsigned int height)
{
	if (!handle) {
		return 0;
	}

	SpoutSender* sender = static_cast<SpoutSender*>(handle);

	// bInvert = true flips Y-axis (OpenGL -> DirectX convention)
	bool result = sender->SendFbo(static_cast<GLuint>(fbo), width, height, true);

	return result ? 1 : 0;
}

// -----------------------------------------------------------------------
// Release a Spout sender object
// -----------------------------------------------------------------------
SPOUTBINDING_API void spReleaseSpout(SpoutHandle handle)
{
	if (!handle) {
		return;
	}

	SpoutSender* sender = static_cast<SpoutSender*>(handle);

	// Release the sender before destroying.
	sender->ReleaseSender();

	delete sender;
}
