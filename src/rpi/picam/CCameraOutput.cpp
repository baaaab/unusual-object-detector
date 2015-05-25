/*
Chris Cummings
This is wraps up the camera system in a simple StartCamera, StopCamera and ReadFrame api to read 
data from the feed. Based on parts of raspivid, and the work done by Pierre Raus at 
http://raufast.org/download/camcv_vid0.c to get the camera feeding into opencv. It 
*/

#include "CCameraOutput.h"
#include <stdio.h>

// Standard port setting for the camera component
#define MMAL_CAMERA_PREVIEW_PORT 0
#define MMAL_CAMERA_VIDEO_PORT 1
#define MMAL_CAMERA_CAPTURE_PORT 2

CCameraOutput::CCameraOutput()
{
	memset(this,0,sizeof(CCameraOutput));
}

CCameraOutput::~CCameraOutput()
{

}

bool CCameraOutput::Init(int width, int height, MMAL_COMPONENT_T* input_component, int input_port_idx, bool do_argb_conversion)
{
	printf("Init camera output with %d/%d\n",width,height);
	Width = width;
	Height = height;

	MMAL_COMPONENT_T *resizer = 0;
	MMAL_CONNECTION_T* connection = 0;
	MMAL_STATUS_T status;
	MMAL_POOL_T* video_buffer_pool = 0;
	MMAL_QUEUE_T* output_queue = 0;

	//got the port we're receiving from
	MMAL_PORT_T* input_port = input_component->output[input_port_idx];

	//check if user wants conversion to argb or the width or height is different to the input
	if(do_argb_conversion || width != (int)input_port->format->es->video.width || height != (int)input_port->format->es->video.height)
	{
		//create the resizing component, reading from the splitter output
		resizer = CreateResizeComponentAndSetupPorts(input_port,do_argb_conversion);
		if(!resizer)
			goto error;

		//create and enable a connection between the video output and the resizer input
		status = mmal_connection_create(&connection, input_port, resizer->input[0], MMAL_CONNECTION_FLAG_TUNNELLING | MMAL_CONNECTION_FLAG_ALLOCATION_ON_INPUT);
		if (status != MMAL_SUCCESS)
		{
			printf("Failed to create connection\n");
			goto error;
		}
		status = mmal_connection_enable(connection);
		if (status != MMAL_SUCCESS)
		{
			printf("Failed to enable connection\n");
			goto error;
		}

		//set the buffer pool port to be the resizer output
		BufferPort = resizer->output[0];
	}
	else
	{
		//no convert/resize needed so just set the buffer pool port to be the input port
		BufferPort = input_port;
	}

	//setup the video buffer callback
	video_buffer_pool = EnablePortCallbackAndCreateBufferPool(BufferPort,VideoBufferCallback,3);
	if(!video_buffer_pool)
		goto error;

	//create the output queue
	output_queue = mmal_queue_create();
	if(!output_queue)
	{
		printf("Failed to create output queue\n");
		goto error;
	}

	ResizerComponent = resizer;
	BufferPool = video_buffer_pool;
	OutputQueue = output_queue;
	Connection = connection;

	return true;

error:

	if(output_queue)
		mmal_queue_destroy(output_queue);
	if(video_buffer_pool)
		mmal_port_pool_destroy(resizer->output[0],video_buffer_pool);
	if(connection)
		mmal_connection_destroy(connection);
	if(resizer)
		mmal_component_destroy(resizer);

	return false;
}

void CCameraOutput::Release()
{
	if(OutputQueue)
		mmal_queue_destroy(OutputQueue);
	if(BufferPool)
		mmal_port_pool_destroy(BufferPort,BufferPool);
	if(Connection)
		mmal_connection_destroy(Connection);
	if(ResizerComponent)
		mmal_component_destroy(ResizerComponent);
	memset(this,0,sizeof(CCameraOutput));
}

void CCameraOutput::OnVideoBufferCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
	//to handle the user not reading frames, remove and return any pre-existing ones
	if(mmal_queue_length(OutputQueue)>=2)
	{
		if(MMAL_BUFFER_HEADER_T* existing_buffer = mmal_queue_get(OutputQueue))
		{
			mmal_buffer_header_release(existing_buffer);
			if (port->is_enabled)
			{
				MMAL_STATUS_T status;
				MMAL_BUFFER_HEADER_T *new_buffer;
				new_buffer = mmal_queue_get(BufferPool->queue);
				if (new_buffer)
					status = mmal_port_send_buffer(port, new_buffer);
				if (!new_buffer || status != MMAL_SUCCESS)
					printf("Unable to return a buffer to the video port\n");
			}	
		}
	}

	//add the buffer to the output queue
	mmal_queue_put(OutputQueue,buffer);

	//printf("Video buffer callback, output queue len=%d\n", mmal_queue_length(OutputQueue));
}

void CCameraOutput::VideoBufferCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
	((CCameraOutput*)port->userdata)->OnVideoBufferCallback(port,buffer);
}

int CCameraOutput::ReadFrame(void* dest, int dest_size)
{
	//default result is 0 - no data available
	int res = 0;

	//get buffer
	const void* buffer; int buffer_len;
	if(BeginReadFrame(buffer,buffer_len))
	{
		if(dest_size >= buffer_len)
		{
			//got space - copy it in and return size
			memcpy(dest,buffer,buffer_len);
			res = buffer_len;
		}
		else
		{
			printf("dest_size:%d <= buffer_len:%d\n", dest_size, buffer_len);
			//not enough space - return failure
			res = -1;
		}
		EndReadFrame();
	}

	return res;
}

bool CCameraOutput::BeginReadFrame(const void* &out_buffer, int& out_buffer_size)
{
	//printf("Attempting to read camera output\n");

	//try and get buffer
	if(MMAL_BUFFER_HEADER_T *buffer = mmal_queue_get(OutputQueue))
	{
		//printf("Reading buffer of %d bytes from output\n",buffer->length);

		//lock it
		mmal_buffer_header_mem_lock(buffer);

		//store it
		LockedBuffer = buffer;

		//fill out the output variables and return success
		out_buffer = buffer->data;
		out_buffer_size = buffer->length;
		return true;
	}
	//no buffer - return false
	return false;
}

void CCameraOutput::EndReadFrame()
{
	if(LockedBuffer)
	{
		// unlock and then release buffer back to the pool from whence it came
		mmal_buffer_header_mem_unlock(LockedBuffer);
		mmal_buffer_header_release(LockedBuffer);
		LockedBuffer = NULL;

		// and send it back to the port (if still open)
		if (BufferPort->is_enabled)
		{
			MMAL_STATUS_T status;
			MMAL_BUFFER_HEADER_T *new_buffer;
			new_buffer = mmal_queue_get(BufferPool->queue);
			if (new_buffer)
				status = mmal_port_send_buffer(BufferPort, new_buffer);
			if (!new_buffer || status != MMAL_SUCCESS)
				printf("Unable to return a buffer to the video port\n");
		}	
	}
}


MMAL_COMPONENT_T* CCameraOutput::CreateResizeComponentAndSetupPorts(MMAL_PORT_T* video_output_port, bool do_argb_conversion)
{
	MMAL_COMPONENT_T *resizer = 0;
	MMAL_PORT_T *input_port = NULL, *output_port = NULL;
	MMAL_STATUS_T status;

	//create the camera component
	status = mmal_component_create("vc.ril.resize", &resizer);
	if (status != MMAL_SUCCESS)
	{
		printf("Failed to create reszie component\n");
		goto error;
	}

	//check we have output ports
	if (resizer->output_num != 1 || resizer->input_num != 1)
	{
		printf("Resizer doesn't have correct ports");
		goto error;
	}

	//get the ports
	input_port = resizer->input[0];
	output_port = resizer->output[0];

	mmal_format_copy(input_port->format,video_output_port->format);
	input_port->buffer_num = 3;
	status = mmal_port_format_commit(input_port);
	if (status != MMAL_SUCCESS)
	{
		printf("Couldn't set resizer input port format : error %d", status);
		goto error;
	}

	mmal_format_copy(output_port->format,input_port->format);
	if(do_argb_conversion)
	{
		output_port->format->encoding = MMAL_ENCODING_RGBA;
		output_port->format->encoding_variant = MMAL_ENCODING_RGBA;
	}
	output_port->format->es->video.width = Width;
	output_port->format->es->video.height = Height;
	output_port->format->es->video.crop.x = 0;
	output_port->format->es->video.crop.y = 0;
	output_port->format->es->video.crop.width = Width;
	output_port->format->es->video.crop.height = Height;
	status = mmal_port_format_commit(output_port);
	if (status != MMAL_SUCCESS)
	{
		printf("Couldn't set resizer output port format : error %d", status);
		goto error;
	}

	return resizer;

error:
	if(resizer)
		mmal_component_destroy(resizer);
	return NULL;
}

MMAL_POOL_T* CCameraOutput::EnablePortCallbackAndCreateBufferPool(MMAL_PORT_T* port, MMAL_PORT_BH_CB_T cb, int buffer_count)
{
	MMAL_STATUS_T status;
	MMAL_POOL_T* buffer_pool = 0;

	//setup video port buffer and a pool to hold them
	port->buffer_num = buffer_count;
	port->buffer_size = port->buffer_size_recommended;
	printf("Creating pool with %d buffers of size %d\n", port->buffer_num, port->buffer_size);
	buffer_pool = mmal_port_pool_create(port, port->buffer_num, port->buffer_size);
	if (!buffer_pool)
	{
		printf("Couldn't create video buffer pool\n");
		goto error;
	}

	//enable the port and hand it the callback
    port->userdata = (struct MMAL_PORT_USERDATA_T *)this;
	status = mmal_port_enable(port, cb);
	if (status != MMAL_SUCCESS)
	{
		printf("Failed to set video buffer callback\n");
		goto error;
	}

	//send all the buffers in our pool to the video port ready for use
	{
		int num = mmal_queue_length(buffer_pool->queue);
		int q;
		for (q=0;q<num;q++)
		{
			MMAL_BUFFER_HEADER_T *buffer = mmal_queue_get(buffer_pool->queue);
			if (!buffer)
			{
				printf("Unable to get a required buffer %d from pool queue\n", q);
				goto error;
			}
			else if (mmal_port_send_buffer(port, buffer)!= MMAL_SUCCESS)
			{
				printf("Unable to send a buffer to port (%d)\n", q);
				goto error;
			}
		}
	}

	return buffer_pool;

error:
	if(buffer_pool)
		mmal_port_pool_destroy(port,buffer_pool);
	return NULL;
}
