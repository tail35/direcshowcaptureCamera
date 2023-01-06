#pragma once

//render
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif

#include <windows.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include "dshowRender/d3dx12.h"

//opencv
#include <iostream>
#include <opencv2/opencv.hpp> //貌似只能把D:\install\opencv4.6.0\opencv\build\include 放再vc常规目录才能引用。有可能是读权限问题。
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/cvstd_wrapper.hpp>


#include<fstream>
#include <string>
#include <vector>
#include <wrl.h>
#include <shellapi.h>
//end

#include <mutex>
#include <list>
#include <string>
#include <memory>
#include <map>
#include <tuple>

using DevicesTuple=std::tuple<std::wstring/*name*/, std::wstring/*path*/ >;
using MapDevices = std::map<int, DevicesTuple>;
using MapPair = std::pair<int, DevicesTuple>;
using DecodeTuple = std::tuple<char*, int>;

#define SAFE_RELEASE(x) if(NULL!=x) { x->Release();x=NULL;}

#define bitByteNum  4
#define DST_FORMAT AV_PIX_FMT_RGBA
//#define DST_FORMAT AV_PIX_FMT_RGB24

class IRecieve
{
public:
  virtual void OnReceive(BYTE* ptr,int size,int width,int height) = 0;
};