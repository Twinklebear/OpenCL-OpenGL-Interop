#define __CL_ENABLE_EXCEPTIONS

#include <string>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <CL/opencl.h>
#include <CL/cl.hpp>
#include <util.h>
#include "tinycl.h"

CL::TinyCL::TinyCL(DEVICE dev, bool interop){
	if (interop)
		SelectInteropDevice(dev);
	else
		SelectDevice(dev);
}
cl::Program CL::TinyCL::LoadProgram(std::string file){
	cl::Program program;
	try {
		std::string content = Util::ReadFile(file);
		cl::Program::Sources source(1, std::make_pair(content.c_str(), content.size()));
		program = cl::Program(mContext, source);
		program.build(mDevices);
		return program;
	}
	catch (const cl::Error &e){
		std::cout << "Program Error: " << e.what() 
			<< " code: " << e.err() << std::endl;
		if (e.err() == CL_BUILD_PROGRAM_FAILURE){
			std::cout << "Build error occured, error log:\n" 
				<< program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(mDevices.at(0)) 
				<< std::endl;
		}
	}
	throw std::runtime_error("Failed to load program");
}
cl::Kernel CL::TinyCL::LoadKernel(const cl::Program &prog, std::string kernel){
	try {
		cl::Kernel kernel(prog, kernel.c_str());
		std::cout << "Kernel info--\n"
			<< "max work group size: " << kernel.getWorkGroupInfo<CL_KERNEL_WORK_GROUP_SIZE>(mDevices.at(0)) << "\n"
			<< "preferred work group size: " 
			<< kernel.getWorkGroupInfo<CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE>(mDevices.at(0))
			<< std::endl;
		return kernel;
	}
	catch (const cl::Error &e){
		std::cout << "Error getting kernel: " << e.what() 
			<< " code: " << e.err() << std::endl;
	}
	throw std::runtime_error("Failed to load kernel");
}
cl::Buffer CL::TinyCL::Buffer(MEM mem, size_t dataSize, void *data){
	try {
		cl::Buffer buf(mContext, mem, dataSize);
		if (data != nullptr)
			mQueue.enqueueWriteBuffer(buf, CL_TRUE, 0, dataSize, data);
		return buf;
	}
	catch (const cl::Error &e){
		std::cout << "Error creating buffer: " << e.what()
			<< " code: " << e.err() << std::endl;
		throw e;
	}
}
cl::BufferGL CL::TinyCL::BufferGL(MEM mem, GL::VertexBuffer &vbo){
	return cl::BufferGL(mContext, mem, vbo);
}
cl::Image2D CL::TinyCL::Image2d(MEM mem, cl::ImageFormat format, int w, int h, cl::size_t<3> origin,
	cl::size_t<3> region, void *pixels)
{
	try {
		//I think this is the error line since I get it even when passing no pixels
		//Intel error, would this error be because I'm using OpenCL 1.2 vs. 1.1?
		//the 1.1 spec makes no mention of the error CL_INVALID_IMAGE_DESCRIPTOR so I think it may be
		//But it's only when writing to GPU? Very strange
		cl::Image2D img(mContext, mem, format, w, h);
		if (pixels != nullptr)
			mQueue.enqueueWriteImage(img, CL_TRUE, origin, region, 0, 0, pixels);
		return img;
	}
	catch (const cl::Error &e){
		std::cout << "Error creating image2d: " << e.what()
			<< " code: " << e.err() << std::endl;
		throw e;
	}
}
#ifdef CL_VERSION_1_2
cl::ImageGL CL::TinyCL::ImageFromTexture(MEM mem, GL::Texture &tex){
	return cl::ImageGL(mContext, mem, GL_TEXTURE_2D, 0, tex);
}
#else
cl::Image2DGL CL::TinyCL::ImageFromTexture(MEM mem, GL::Texture &tex){
	return cl::Image2DGL(mContext, mem, GL_TEXTURE_2D, 0, tex);
}
#endif
void CL::TinyCL::WriteData(const cl::Buffer &b, size_t dataSize, void *data){
	mQueue.enqueueWriteBuffer(b, CL_TRUE, 0, dataSize, data);
}
void CL::TinyCL::WriteData(const cl::Image &img, cl::size_t<3> origin, cl::size_t<3> region, void *pixels){
	//Is 0, 0 right for row/slice pitch?
	mQueue.enqueueWriteImage(img, CL_TRUE, origin, region, 0, 0, pixels);
}
void CL::TinyCL::ReadData(const cl::Buffer &buf, size_t dataSize, void *data){
	try {
		mQueue.enqueueReadBuffer(buf, CL_TRUE, 0, dataSize, data);
		mQueue.finish();
	}
	catch (const cl::Error &e){
		std::cout << "Error reading buffer: " << e.what()
			<< " code: " << e.err() << std::endl;
	}
}
void CL::TinyCL::ReadData(const cl::Image &img, cl::size_t<3> origin, cl::size_t<3> region, void *pixels){
	try {
		mQueue.enqueueReadImage(img, CL_TRUE, origin, region, 0, 0, pixels);
		mQueue.finish();
	}
	catch (const cl::Error &e){
		std::cout << "Error reading buffer: " << e.what()
			<< " code: " << e.err() << std::endl;
	}
}
void CL::TinyCL::RunKernel(const cl::Kernel &kernel, cl::NDRange local, cl::NDRange global, cl::NDRange offset){
	try {
		mQueue.enqueueNDRangeKernel(kernel, offset, global, local);
		mQueue.finish();
	}
	catch (const cl::Error &e){
		std::cout << "Error running nd range kernel: " << e.what()
			<< " code: " << e.err() << std::endl;
	}
}
void CL::TinyCL::SelectDevice(DEVICE dev){
	try {
		//We assume only the first device and platform will be used
		//This is after all a lazy implementation
		cl::Platform::get(&mPlatforms);
		//Query the devices for the type desired
		mPlatforms.at(0).getDevices(dev, &mDevices);
		std::cout << "Device info--\n" << "Name: " << mDevices.at(0).getInfo<CL_DEVICE_NAME>()
			<< "\nVendor: " << mDevices.at(0).getInfo<CL_DEVICE_VENDOR>() 
			<< "\nDriver Version: " << mDevices.at(0).getInfo<CL_DRIVER_VERSION>() 
			<< "\nDevice Profile: " << mDevices.at(0).getInfo<CL_DEVICE_PROFILE>() 
			<< "\nDevice Version: " << mDevices.at(0).getInfo<CL_DEVICE_VERSION>()
			<< std::endl;
		mContext = cl::Context(mDevices);
		mQueue = cl::CommandQueue(mContext, mDevices.at(0));
	}
	catch (const cl::Error &e){
		std::cout << "Error selecting device: " << e.what() 
			<< " code: " << e.err() << std::endl;
		throw e;
	}
}
void CL::TinyCL::SelectInteropDevice(DEVICE dev){
	try {
		//We assume only the first device and platform will be used
		//This is after all a lazy implementation
		cl::Platform::get(&mPlatforms);
		//Query the devices for the type desired
		mPlatforms.at(0).getDevices(dev, &mDevices);
#ifdef _WIN32
		cl_context_properties properties[] = {
			CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
			CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
			CL_CONTEXT_PLATFORM, (cl_context_properties)(mPlatforms[0])(),
			0
		};
#endif
#ifdef __linux__
		cl_context_properties properties[] = {
			CL_GL_CONTEXT_KHR, (cl_context_properties)glXGetCurrentContext(),
			CL_WGL_HDC_KHR, (cl_context_properties)glXGetCurrentDisplay(),
			CL_CONTEXT_PLATFORM, (cl_context_properties)(mPlatforms[0])(),
			0
		};
#endif
		mContext = cl::Context(mDevices, properties);
		//Grab the OpenGL device
		mDevices = mContext.getInfo<CL_CONTEXT_DEVICES>();
		mQueue = cl::CommandQueue(mContext, mDevices.at(0));
		std::cout << "OpenCL Info:" 
			<< "\nName: " << mDevices.at(0).getInfo<CL_DEVICE_NAME>()
			<< "\nVendor: " << mDevices.at(0).getInfo<CL_DEVICE_VENDOR>() 
			<< "\nDriver Version: " << mDevices.at(0).getInfo<CL_DRIVER_VERSION>() 
			<< "\nDevice Profile: " << mDevices.at(0).getInfo<CL_DEVICE_PROFILE>() 
			<< "\nDevice Version: " << mDevices.at(0).getInfo<CL_DEVICE_VERSION>()
			<< std::endl;
	}
	catch (const cl::Error &e){
		std::cout << "Error selecting GL interop device: " << e.what() 
			<< " code: " << e.err() << std::endl;
		throw e;
	}

}
