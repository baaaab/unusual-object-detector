/*
 Chris Cummings
 This is wraps up the camera system in a simple StartCamera, StopCamera and ReadFrame api to read
 data from the feed. Based on parts of raspivid, and the work done by Pierre Raus at
 http://raufast.org/download/camcv_vid0.c to get the camera feeding into opencv. It
 */

#include "CCamera.h"
#include "CCameraOutput.h"
#include <stdio.h>

// Standard port setting for the camera component
#define MMAL_CAMERA_PREVIEW_PORT 0
#define MMAL_CAMERA_VIDEO_PORT 1
#define MMAL_CAMERA_CAPTURE_PORT 2

static CCamera* GCamera = NULL;

CCamera* CCamera::StartCamera(int width, int height, int framerate, int num_levels, bool do_argb_conversion)
{
	//can't create more than one camera
	if (GCamera != NULL)
	{
		printf("Can't create more than one camera\n");
		return NULL;
	}

	//create and attempt to initialize the camera
	GCamera = new CCamera();
	if (!GCamera->Init(width, height, framerate, num_levels, do_argb_conversion))
	{
		//failed so clean up
		printf("Camera init failed\n");
		delete GCamera;
		GCamera = NULL;
	}
	return GCamera;
}

void CCamera::StopCamera()
{
	if (GCamera)
	{
		GCamera->Release();
		delete GCamera;
		GCamera = NULL;
	}
}

CCamera::CCamera()
{
	CameraComponent = NULL;
	SplitterComponent = NULL;
	VidToSplitConn = NULL;
	memset(Outputs, 0, sizeof(Outputs));
}

CCamera::~CCamera()
{

}

void CCamera::CameraControlCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
	GCamera->OnCameraControlCallback(port, buffer);
}

MMAL_COMPONENT_T* CCamera::CreateCameraComponentAndSetupPorts()
{
	MMAL_COMPONENT_T *camera = 0;
	MMAL_ES_FORMAT_T *format;
	MMAL_PORT_T *preview_port = NULL, *video_port = NULL, *still_port = NULL;
	MMAL_STATUS_T status;

	//create the camera component
	status = mmal_component_create(MMAL_COMPONENT_DEFAULT_CAMERA, &camera);
	if (status != MMAL_SUCCESS)
	{
		printf("Failed to create camera component\n");
		return NULL;
	}

	//check we have output ports
	if (!camera->output_num)
	{
		printf("Camera doesn't have output ports");
		mmal_component_destroy(camera);
		return NULL;
	}

	//get the 3 ports
	preview_port = camera->output[MMAL_CAMERA_PREVIEW_PORT];
	video_port = camera->output[MMAL_CAMERA_VIDEO_PORT];
	still_port = camera->output[MMAL_CAMERA_CAPTURE_PORT];

	// Enable the camera, and tell it its control callback function
	status = mmal_port_enable(camera->control, CameraControlCallback);
	if (status != MMAL_SUCCESS)
	{
		printf("Unable to enable control port : error %d", status);
		mmal_component_destroy(camera);
		return NULL;
	}

	//  set up the camera configuration
	{
		MMAL_PARAMETER_CAMERA_CONFIG_T cam_config;
		cam_config.hdr.id = MMAL_PARAMETER_CAMERA_CONFIG;
		cam_config.hdr.size = sizeof(cam_config);
		cam_config.max_stills_w = Width;
		cam_config.max_stills_h = Height;
		cam_config.stills_yuv422 = 0;
		cam_config.one_shot_stills = 0;
		cam_config.max_preview_video_w = Width;
		cam_config.max_preview_video_h = Height;
		cam_config.num_preview_video_frames = 3;
		cam_config.stills_capture_circular_buffer_height = 0;
		cam_config.fast_preview_resume = 0;
		cam_config.use_stc_timestamp = MMAL_PARAM_TIMESTAMP_MODE_RESET_STC;
		mmal_port_parameter_set(camera->control, &cam_config.hdr);
	}

	// setup preview port format - QUESTION: Needed if we aren't using preview?
	format = preview_port->format;
	format->encoding = MMAL_ENCODING_OPAQUE;
	format->encoding_variant = MMAL_ENCODING_I420;
	format->es->video.width = Width;
	format->es->video.height = Height;
	format->es->video.crop.x = 0;
	format->es->video.crop.y = 0;
	format->es->video.crop.width = Width;
	format->es->video.crop.height = Height;
	format->es->video.frame_rate.num = FrameRate;
	format->es->video.frame_rate.den = 1;
	status = mmal_port_format_commit(preview_port);
	if (status != MMAL_SUCCESS)
	{
		printf("Couldn't set preview port format : error %d", status);
		mmal_component_destroy(camera);
		return NULL;
	}

	//setup video port format
	format = video_port->format;
	format->encoding = MMAL_ENCODING_I420;
	format->encoding_variant = MMAL_ENCODING_I420;
	format->es->video.width = Width;
	format->es->video.height = Height;
	format->es->video.crop.x = 0;
	format->es->video.crop.y = 0;
	format->es->video.crop.width = Width;
	format->es->video.crop.height = Height;
	format->es->video.frame_rate.num = FrameRate;
	format->es->video.frame_rate.den = 1;
	status = mmal_port_format_commit(video_port);
	if (status != MMAL_SUCCESS)
	{
		printf("Couldn't set video port format : error %d", status);
		mmal_component_destroy(camera);
		return NULL;
	}

	//setup still port format
	format = still_port->format;
	format->encoding = MMAL_ENCODING_OPAQUE;
	format->encoding_variant = MMAL_ENCODING_I420;
	format->es->video.width = Width;
	format->es->video.height = Height;
	format->es->video.crop.x = 0;
	format->es->video.crop.y = 0;
	format->es->video.crop.width = Width;
	format->es->video.crop.height = Height;
	format->es->video.frame_rate.num = 1;
	format->es->video.frame_rate.den = 1;
	status = mmal_port_format_commit(still_port);
	if (status != MMAL_SUCCESS)
	{
		printf("Couldn't set still port format : error %d", status);
		mmal_component_destroy(camera);
		return NULL;
	}

	//apply all camera parameters
	raspicamcontrol_set_all_parameters(camera, &CameraParameters);

	//enable the camera
	status = mmal_component_enable(camera);
	if (status != MMAL_SUCCESS)
	{
		printf("Couldn't enable camera\n");
		mmal_component_destroy(camera);
		return NULL;
	}

	return camera;
}

MMAL_COMPONENT_T* CCamera::CreateSplitterComponentAndSetupPorts(MMAL_PORT_T* video_output_port)
{
	MMAL_COMPONENT_T *splitter = 0;
	MMAL_PORT_T *input_port = NULL, *output_port = NULL;
	MMAL_STATUS_T status;

	//create the camera component
	status = mmal_component_create(MMAL_COMPONENT_DEFAULT_VIDEO_SPLITTER, &splitter);
	if (status != MMAL_SUCCESS)
	{
		printf("Failed to create splitter component\n");
		goto error;
	}

	//check we have output ports
	if (splitter->output_num != 4 || splitter->input_num != 1)
	{
		printf("Splitter doesn't have correct ports: %d, %d\n", splitter->input_num, splitter->output_num);
		goto error;
	}

	//get the ports
	input_port = splitter->input[0];
	mmal_format_copy(input_port->format, video_output_port->format);
	input_port->buffer_num = 3;
	status = mmal_port_format_commit(input_port);
	if (status != MMAL_SUCCESS)
	{
		printf("Couldn't set resizer input port format : error %d", status);
		goto error;
	}

	for (uint32_t i = 0; i < splitter->output_num; i++)
	{
		output_port = splitter->output[i];
		output_port->buffer_num = 3;
		mmal_format_copy(output_port->format, input_port->format);
		status = mmal_port_format_commit(output_port);
		if (status != MMAL_SUCCESS)
		{
			printf("Couldn't set resizer output port format : error %d", status);
			goto error;
		}
	}

	return splitter;

	error: if (splitter)
		mmal_component_destroy(splitter);
	return NULL;
}

bool CCamera::Init(int width, int height, int framerate, int num_levels, bool do_argb_conversion)
{
	//init broadcom host - QUESTION: can this be called more than once??
	bcm_host_init();

	//store basic parameters
	Width = width;
	Height = height;
	FrameRate = framerate;

	// Set up the camera_parameters to default
	raspicamcontrol_set_defaults(&CameraParameters);

	MMAL_PORT_T *video_port = NULL;
	MMAL_STATUS_T status;

	//create the camera component
	CameraComponent = CreateCameraComponentAndSetupPorts();
	if (!CameraComponent)
		goto error;

	//get the video port
	video_port = CameraComponent->output[MMAL_CAMERA_VIDEO_PORT];
	video_port->buffer_num = 3;

	//create the splitter component
	SplitterComponent = CreateSplitterComponentAndSetupPorts(video_port);
	if (!SplitterComponent)
		goto error;

	//create and enable a connection between the video output and the resizer input
	status = mmal_connection_create(&VidToSplitConn, video_port, SplitterComponent->input[0], MMAL_CONNECTION_FLAG_TUNNELLING | MMAL_CONNECTION_FLAG_ALLOCATION_ON_INPUT);
	if (status != MMAL_SUCCESS)
	{
		printf("Failed to create connection\n");
		goto error;
	}
	status = mmal_connection_enable(VidToSplitConn);
	if (status != MMAL_SUCCESS)
	{
		printf("Failed to enable connection\n");
		goto error;
	}

	//setup all the outputs
	for (int i = 0; i < num_levels; i++)
	{
		Outputs[i] = new CCameraOutput();
		if (!Outputs[i]->Init(Width >> i, Height >> i, SplitterComponent, i, do_argb_conversion))
		{
			printf("Failed to initialize output %d\n", i);
			goto error;
		}
	}

	//begin capture
	if (mmal_port_parameter_set_boolean(video_port, MMAL_PARAMETER_CAPTURE, 1) != MMAL_SUCCESS)
	{
		printf("Failed to start capture\n");
		goto error;
	}

	//return success
	printf("Camera successfully created\n");
	return true;

	error: Release();

	return false;
}

void CCamera::Release()
{
	for (int i = 0; i < 4; i++)
	{
		if (Outputs[i])
		{
			Outputs[i]->Release();
			delete Outputs[i];
			Outputs[i] = NULL;
		}
	}
	if (VidToSplitConn)
		mmal_connection_destroy(VidToSplitConn);
	if (CameraComponent)
		mmal_component_destroy(CameraComponent);
	if (SplitterComponent)
		mmal_component_destroy(SplitterComponent);
	VidToSplitConn = NULL;
	CameraComponent = NULL;
	SplitterComponent = NULL;
}

void CCamera::OnCameraControlCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
	printf("Camera control callback\n");
}

bool CCamera::BeginReadFrame(int level, const void* &out_buffer, int& out_buffer_size)
{
	return Outputs[level] ? Outputs[level]->BeginReadFrame(out_buffer, out_buffer_size) : false;
}

void CCamera::EndReadFrame(int level)
{
	if (Outputs[level])
		Outputs[level]->EndReadFrame();
}

int CCamera::ReadFrame(int level, void* dest, int dest_size)
{
	return Outputs[level] ? Outputs[level]->ReadFrame(dest, dest_size) : -1;
}

