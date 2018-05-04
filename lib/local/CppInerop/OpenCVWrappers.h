///////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2017, Carnegie Mellon University and University of Cambridge,
// all rights reserved.
//
// ACADEMIC OR NON-PROFIT ORGANIZATION NONCOMMERCIAL RESEARCH USE ONLY
//
// BY USING OR DOWNLOADING THE SOFTWARE, YOU ARE AGREEING TO THE TERMS OF THIS LICENSE AGREEMENT.  
// IF YOU DO NOT AGREE WITH THESE TERMS, YOU MAY NOT USE OR DOWNLOAD THE SOFTWARE.
//
// License can be found in OpenFace-license.txt

//     * Any publications arising from the use of this software, including but
//       not limited to academic journal and conference publications, technical
//       reports and manuals, must cite at least one of the following works:
//
//       OpenFace: an open source facial behavior analysis toolkit
//       Tadas Baltru�aitis, Peter Robinson, and Louis-Philippe Morency
//       in IEEE Winter Conference on Applications of Computer Vision, 2016  
//
//       Rendering of Eyes for Eye-Shape Registration and Gaze Estimation
//       Erroll Wood, Tadas Baltru�aitis, Xucong Zhang, Yusuke Sugano, Peter Robinson, and Andreas Bulling 
//       in IEEE International. Conference on Computer Vision (ICCV),  2015 
//
//       Cross-dataset learning and person-speci?c normalisation for automatic Action Unit detection
//       Tadas Baltru�aitis, Marwa Mahmoud, and Peter Robinson 
//       in Facial Expression Recognition and Analysis Challenge, 
//       IEEE International Conference on Automatic Face and Gesture Recognition, 2015 
//
//       Constrained Local Neural Fields for robust facial landmark detection in the wild.
//       Tadas Baltru�aitis, Peter Robinson, and Louis-Philippe Morency. 
//       in IEEE Int. Conference on Computer Vision Workshops, 300 Faces in-the-Wild Challenge, 2013.    
//
///////////////////////////////////////////////////////////////////////////////

// OpenCVWrappers.h

#pragma once

#pragma managed

#include <msclr\marshal_cppstd.h>

#pragma unmanaged

#include "cv.h"
#include "highgui.h"

#include <opencv2/videoio/videoio.hpp>  // Video write
#include <opencv2/videoio/videoio_c.h>  // Video write

#pragma managed

#pragma make_public(cv::Mat)

using namespace System::Windows;
using namespace System::Windows::Threading;
using namespace System::Windows::Media;
using namespace System::Windows::Media::Imaging;

namespace OpenCVWrappers {

	public ref class RawImage : System::IDisposable
	{

	private:

		cv::Mat* mat;

		static int refCount;


	public:

		static int PixelFormatToType(PixelFormat fmt)
		{
			if (fmt == PixelFormats::Gray8)
				return CV_8UC1;
			else if (fmt == PixelFormats::Bgr24)
				return CV_8UC3;
			else if (fmt == PixelFormats::Bgra32)
				return CV_8UC4;
			else if (fmt == PixelFormats::Gray32Float)
				return CV_32FC1;
			else
				throw gcnew System::Exception("Unsupported pixel format");
		}

		static PixelFormat TypeToPixelFormat(int type)
		{
			switch (type) {
			case CV_8UC1:
				return PixelFormats::Gray8;
			case CV_8UC3:
				return PixelFormats::Bgr24;
			case CV_8UC4:
				return PixelFormats::Bgra32;
			case CV_32FC1:
				return PixelFormats::Gray32Float;
			default:
				throw gcnew System::Exception("Unsupported image type");
			}
		}

		static property int RefCount {
			int get() { return refCount; }
		}

		RawImage()
		{
			mat = new cv::Mat();
			refCount++;
		}

		RawImage(const cv::Mat& m)
		{
			mat = new cv::Mat(m.clone());
			refCount++;
		}

		RawImage(RawImage^ img)
		{
			mat = new cv::Mat(img->Mat.clone());
			refCount++;
		}

		RawImage(int width, int height, int type)
		{
			mat = new cv::Mat(height, width, type);
			refCount++;
		}

		RawImage(int width, int height, PixelFormat format)
		{
			int type = RawImage::PixelFormatToType(format);
			mat = new cv::Mat(height, width, type);
			refCount++;
		}

		void Mirror()
		{
			cv::flip(*mat, *mat, 1);
		}

		// Finalizer. Definitely called before Garbage Collection,
		// but not automatically called on explicit Dispose().
		// May be called multiple times.
		!RawImage()
		{
			if (mat)
			{
				delete mat;
				mat = NULL;
				refCount--;
			}
		}

		// Destructor. Called on explicit Dispose() only.
		~RawImage()
		{
			this->!RawImage();
		}

		property int Width
		{
			int get()
			{
				return mat->cols;
			}
		}

		property int Height
		{
			int get()
			{
				return mat->rows;
			}
		}

		property int Stride
		{

			int get()
			{
				return mat->step;
			}
		}

		property PixelFormat Format
		{
			PixelFormat get()
			{
				return TypeToPixelFormat(mat->type());
			}
		}

		property cv::Mat& Mat
		{
			cv::Mat& get()
			{
				return *mat;
			}
		}

		property bool IsEmpty
		{
			bool get()
			{
				return !mat || mat->empty();
			}
		}

		bool UpdateWriteableBitmap(WriteableBitmap^ bitmap)
		{
			if (bitmap == nullptr || bitmap->PixelWidth != Width || bitmap->PixelHeight != Height || bitmap->Format != Format)
				return false;
			else {
				if (mat->data == NULL) {
					cv::Mat zeros(bitmap->PixelHeight, bitmap->PixelWidth, PixelFormatToType(bitmap->Format), 0);
					bitmap->WritePixels(Int32Rect(0, 0, Width, Height), System::IntPtr(zeros.data), Stride * Height * (Format.BitsPerPixel / 8), Stride, 0, 0);
				}
				else {
					bitmap->WritePixels(Int32Rect(0, 0, Width, Height), System::IntPtr(mat->data), Stride * Height * (Format.BitsPerPixel / 8), Stride, 0, 0);
				}
				return true;
			}
		}

		WriteableBitmap^ CreateWriteableBitmap()
		{
			return gcnew WriteableBitmap(Width, Height, 72, 72, Format, nullptr);
		}

	};

	public ref class VideoWriter
	{
	private:
		// OpenCV based video capture for reading from files
		cv::VideoWriter* vc;

	public:

		VideoWriter(System::String^ location, int width, int height, double fps, bool colour)
		{

			msclr::interop::marshal_context context;
			std::string location_std_string = context.marshal_as<std::string>(location);

			vc = new cv::VideoWriter(location_std_string, CV_FOURCC('D', 'I', 'V', 'X'), fps, cv::Size(width, height), colour);

		}

		// Return success
		bool Write(RawImage^ img)
		{
			if (vc != nullptr && vc->isOpened())
			{
				vc->write(img->Mat);
				return true;
			}
			else
			{
				return false;
			}
		}

		// Finalizer. Definitely called before Garbage Collection,
		// but not automatically called on explicit Dispose().
		// May be called multiple times.
		!VideoWriter()
		{
			if (vc != nullptr)
			{
				vc->~VideoWriter();
			}
		}

		// Destructor. Called on explicit Dispose() only.
		~VideoWriter()
		{
			this->!VideoWriter();
		}

	};

}
