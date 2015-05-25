#pragma once

#include "mmalincludes.h"
#include "cameracontrol.h"

class CCamera;

class CCameraOutput
{
public:
	int						Width;
	int						Height;
	MMAL_COMPONENT_T*		ResizerComponent;
	MMAL_CONNECTION_T*		Connection;
	MMAL_BUFFER_HEADER_T*	LockedBuffer;
	MMAL_POOL_T*			BufferPool;
	MMAL_QUEUE_T*			OutputQueue;
	MMAL_PORT_T*			BufferPort;

	CCameraOutput();
	~CCameraOutput();
	bool Init(int width, int height, MMAL_COMPONENT_T* input_component, int input_port_idx, bool do_argb_conversion);
	void Release();
	void OnVideoBufferCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
	static void VideoBufferCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
	int ReadFrame(void* buffer, int buffer_size);
	bool BeginReadFrame(const void* &out_buffer, int& out_buffer_size);
	void EndReadFrame();
	MMAL_POOL_T* EnablePortCallbackAndCreateBufferPool(MMAL_PORT_T* port, MMAL_PORT_BH_CB_T cb, int buffer_count);
	MMAL_COMPONENT_T* CreateResizeComponentAndSetupPorts(MMAL_PORT_T* video_output_port, bool do_argb_conversion);

};
