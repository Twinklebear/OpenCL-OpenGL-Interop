#ifndef TINYCL_H
#define TINYCL_H

#include <string>
#include <iostream>
#include <stdexcept>
#include <CL/opencl.h>
#include <CL/cl.hpp>
#include <glvertexbuffer.h>
#include <glvertexarray.h>

namespace Util {
	std::string ReadFile(const std::string &file);
}


namespace CL {
	enum DEVICE { CPU = CL_DEVICE_TYPE_CPU, GPU = CL_DEVICE_TYPE_GPU };
	enum MEM { READ_ONLY = CL_MEM_READ_ONLY, WRITE_ONLY = CL_MEM_WRITE_ONLY, 
		READ_WRITE = CL_MEM_READ_WRITE };
	/**
	* A lightweight wrapper for dealing with simple OpenCL contexts
	*/
	class TinyCL {
	public:
		/**
		* Setup the system to run on the device type specified
		* @param dev Device type to look for
		* @param interop True if an OpenGL interop context is desired, default false
		*/
		TinyCL(DEVICE dev, bool interop = false);
		//Load a Program for use
		cl::Program LoadProgram(std::string file);
		//Load a kernel from a program
		cl::Kernel LoadKernel(const cl::Program &prog, std::string kernel);
		/*
		* Create a cl::Buffer and pass some data to it if desired
		* @param mem The type of memory the buffer is (read, write, read&write)
		* @param dataSize Size of buffer to create
		* @param data The data to write to the buffer, default is nullptr in which case
		*       the buffer will be made but no data written
		* @return The created buffer
		*/
		cl::Buffer Buffer(MEM mem, size_t dataSize, void *data = nullptr);
		/*
		* Create a cl::BufferGL and pass some data to it if desired
		* @param mem The type of memory the buffer is (read, write, read&write)
		* @param vbo The vbo to use as the data
		* @return The created buffer
		*/
		cl::BufferGL BufferGL(MEM mem, GL::VertexBuffer &vbo);
		/*
		* Create a cl::Image2D and pass some data to it if desired, use of the default 
		* parameters specifies that no write is desired, note that all default params must
		* be used if this is the case
		* @param mem The type of memory the image is (read, write)
		* @param format The format of the image data
		* @param w Image width
		* @param h Image height
		* @param origin Starting location to read data from, default empty
		* @param region Size of region of data to read, default empty
		* @param pixels The pixel data to read, default is nullptr in which case
		*       the image2d is created but no data written
		* @return The created Image2D
		*/
		cl::Image2D Image2d(MEM mem, cl::ImageFormat format, int w, int h, cl::size_t<3> origin = cl::size_t<3>(),
			cl::size_t<3> region = cl::size_t<3>(), void *pixels = nullptr);
		/*
		* Create an image from a GL::Texture
		* The ImageGL is in OpenCL 1.2 only (I think)
		* @param mem Type of memory the image is (read/write)
		* @param texture The texture to use
		* @return the cl::Image2DGL corresponding to the texture
		*/
#ifdef CL_VERSION_1_2
		cl::ImageGL ImageFromTexture(MEM mem, GL::Texture &tex);
#else
		cl::Image2DGL ImageFromTexture(MEM mem, GL::Texture &tex);
#endif
		/*
		* Write some data to a buffer
		* @param buf The buffer to write too
		* @param dataSize Size of data to write
		* @param data The data to write
		*/
		void WriteData(const cl::Buffer &b, size_t dataSize, void *data);
		/*
		* Write some data to an Image2d
		* @param img The image2d to write too
		* @param origin The point to start reading values from
		* @param region The size of region to read data from
		* @param pixels The data to read
		*/
		void WriteData(const cl::Image &img, cl::size_t<3> origin, cl::size_t<3> region, void *pixels);
		/*
		* Read the data from some buffer into the block of memory pointed to by data
		* @param buf The device buffer to read from
		* @param dataSize The size of the data to read
		* @param data Where to put the data
		*/
		void ReadData(const cl::Buffer &buf, size_t dataSize, void *data);
		/*
		* Read the image data from some kernel image argument into data
		* @param img The device image data to read from
		* @param origin The point to start reading from
		* @param region The size of the region to read (or is it the coord to read too?)
		* @param pixels The location to read the data too
		*/
		void ReadData(const cl::Image &img, cl::size_t<3> origin, cl::size_t<3> region, void *pixels);
		/*
		* Run the kernel
		* @param local The local dimensions
		* @param numGroups The global dimensions
		* @param offset The initial offset for each id dimension, default is NullRange, ie no offset
		*/
		void RunKernel(const cl::Kernel &kernel, cl::NDRange local, cl::NDRange global, cl::NDRange offset = cl::NullRange);
		/*
		* Get the preferred work group size for some kernel on the device
		* @param kernel The kernel to get preferred size of
		* @return Preferred work group size
		*/
		int PreferredWorkSize(const cl::Kernel &kernel);

	private:
		/**
		* Select the device to be used and setup the context and command queue
		* @param dev Device type to look for
		*/
		void SelectDevice(DEVICE dev);
		/**
		* Selecte the device to be used and setup the context and cmd queue for 
		* an OpenGL interop context
		* @param dev Device type to get
		*/
		void SelectInteropDevice(DEVICE dev);

	public:
		std::vector<cl::Platform> mPlatforms;
		std::vector<cl::Device> mDevices;
		cl::Context mContext;
		cl::CommandQueue mQueue;
	};
}

#endif