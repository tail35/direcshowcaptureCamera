#pragma once
#include <dshow.h>
#include "common.h"
#include "obsfilter/capture-filter.hpp"
#include <string>
extern "C"
{
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "libavcodec/avcodec.h"
	

//#include "D:\\installgreen\\ffmpeg-master-latest-win64-gpl-shared\\include\\libavformat\\avformat.h"
//#include "D:\\installgreen\\ffmpeg-master-latest-win64-gpl-shared\\include\\libswscale\\swscale.h"
//#include "D:\\installgreen\\ffmpeg-master-latest-win64-gpl-shared\\include\\libavutil\\imgutils.h"
}


#pragma comment(lib, "Strmiids.lib")
using namespace DShow;

class MyCaptrueVideo
{
public:
 MyCaptrueVideo();
 ~MyCaptrueVideo();
public:
 void InitParameters(IRecieve* rc);
 bool Init(IRecieve* rc);
 bool Release();
 HRESULT InitCaptureGraphBuilder();
 bool EnumDevices();
 void GetDevices(MapDevices& devices);
 
 bool BindFilter(int deviceId, IBaseFilter** pFilterCamera);
 void Receive(bool isVideo, IMediaSample* sample);
 void InitCaptrueFilter();
 void StartCaptrue(int index);
 void StopCaptrue();
 bool GetHeaderCodeInfo(unsigned char* data, unsigned int dataSize);
 void MJPEGToRGB(unsigned char* data, unsigned int dataSize, unsigned char* outBuffer);
 void YUV2ToRGB(unsigned char* data, unsigned int dataSize, unsigned char* outBuffer);
 static int FillBuffer(void* opaque, uint8_t* buf, int len);

 //
 HRESULT GetCaptureRatio(ComQIPtr<IAMStreamConfig> pam); 

private:
  IRecieve* mIRecieve;

  HWND m_hWnd;
  IGraphBuilder* mIGraphBuilder;
  ICaptureGraphBuilder2* mICaptureGraphBuilder2;
  IBaseFilter* mIBaseFilterCamera;
  IMediaControl* mIMediaControl;
  //obs 
  ComPtr<DShow::CaptureFilter> mCaptureFilter;
  MediaType videoMediaType;  ;
  VideoConfig videoConfig;
  int mWidth;
  int mHeight;
  BYTE* mTemp;
  BYTE* mSrcTemp;
  //end obs
  //ffmpeg
  AVFormatContext* mAVFormatContext;
  unsigned char* mAvioCtxBuffer;
  size_t				mAvioCtxBufferSize;
  AVIOContext* mAVIOContext;
  bool isHasDecodeInfo;
  int mVindex = -1;
  AVCodecContext* mAVCodecContext;
  const AVCodec* mAVCodec;  
  AVPacket* mAVPacket;
  AVFrame* mAVFrameSrc;
  AVFrame* mAVFrameDst;
  //

  MapDevices mDevicesName;
  //
  std::mutex mReceveAndStopMutex;//for avoid stop and onreceive crash.
  //
  
};

