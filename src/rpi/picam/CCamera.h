#pragma once

#include "mmalincludes.h"
#include "cameracontrol.h"

class CCameraOutput;

class CCamera
{
public:

	int ReadFrame(int level, void* buffer, int buffer_size);
	bool BeginReadFrame(int level, const void* &out_buffer, int& out_buffer_size);
	void EndReadFrame(int level);

	static CCamera* StartCamera(int width, int height, int framerate, int num_levels, bool do_argb_conversion=true);
	static void StopCamera();

private:
	CCamera();
	~CCamera();

	bool Init(int width, int height, int framerate, int num_levels, bool do_argb_conversion);
	void Release();
	MMAL_COMPONENT_T* CreateCameraComponentAndSetupPorts();
	MMAL_COMPONENT_T* CreateSplitterComponentAndSetupPorts(MMAL_PORT_T* video_ouput_port);

	void OnCameraControlCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
	static void CameraControlCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);

	int							Width;
	int							Height;
	int							FrameRate;
	RASPICAM_CAMERA_PARAMETERS	CameraParameters;
	MMAL_COMPONENT_T*			CameraComponent;    
	MMAL_COMPONENT_T*			SplitterComponent;
	MMAL_CONNECTION_T*			VidToSplitConn;
	CCameraOutput*				Outputs[4];

	friend CCamera* StartCamera(int width, int height, int framerate, int num_levels, bool do_argb_conversion);
	friend void StopCamera();
};
