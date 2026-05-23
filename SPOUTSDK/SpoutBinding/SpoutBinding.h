/*
	SpoutBinding.h

	C ABI wrapper for Spout SDK - Java FFM compatible binding.

	Exposes a minimal set of Spout sender functions via extern "C"
	so that Java 25's jextract tool can generate Foreign Function &
	Memory (FFM) API bindings.

	Copyright (c) 2025, Lynn Jarvis. All rights reserved.
	https://spout.zeal.co
*/
#pragma once

#ifndef __SpoutBinding__
#define __SpoutBinding__

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
	#if defined(SPOUTBINDING_EXPORTS)
		#define SPOUTBINDING_API __declspec(dllexport)
	#else
		#define SPOUTBINDING_API __declspec(dllimport)
	#endif
#else
	#define SPOUTBINDING_API
#endif

// Opaque handle to the internal Spout sender object.
// Used by all SpoutBinding functions to identify a Spout instance.
typedef void* SpoutHandle;

// -----------------------------------------------------------------------
// spCreateSpout
//
// Creates a Spout sender object and returns an opaque handle.
// The returned handle must be passed to spSendFrameBufferObject and
// eventually to spReleaseSpout for cleanup.
//
// Returns: a non-null SpoutHandle on success, NULL on failure.
// -----------------------------------------------------------------------
SPOUTBINDING_API SpoutHandle spCreateSpout(void);

// -----------------------------------------------------------------------
// spSendFrameBufferObject
//
// Sends an OpenGL framebuffer object (FBO) via Spout.
// The FBO must currently be bound for read (GL_READ_FRAMEBUFFER).
// The texture attached to GL_COLOR_ATTACHMENT0 will be shared.
//
//  handle  - SpoutHandle returned by spCreateSpout (must not be NULL).
//  fbo     - OpenGL FBO ID (GLuint). Pass 0 to send the default framebuffer.
//  width   - Width of the region to send.
//  height  - Height of the region to send.
//
// Returns: 1 on success, 0 on failure.
// -----------------------------------------------------------------------
SPOUTBINDING_API int spSendFrameBufferObject(SpoutHandle handle, unsigned int fbo, unsigned int width, unsigned int height);

// -----------------------------------------------------------------------
// spReleaseSpout
//
// Releases a Spout sender object created by spCreateSpout.
// After this call the handle is invalid and must not be used again.
//
//  handle  - SpoutHandle returned by spCreateSpout.
// -----------------------------------------------------------------------
SPOUTBINDING_API void spReleaseSpout(SpoutHandle handle);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // __SpoutBinding__
