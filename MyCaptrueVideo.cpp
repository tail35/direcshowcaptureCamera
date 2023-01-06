
#include "MyCaptrueVideo.h"
#include "common.h"




MyCaptrueVideo::MyCaptrueVideo() {


}
MyCaptrueVideo::~MyCaptrueVideo() {

}

HRESULT MyCaptrueVideo::InitCaptureGraphBuilder()
{
  HRESULT hr;
  // 创建IGraphBuilder接口
  hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&mIGraphBuilder);
  // 创建ICaptureGraphBuilder2接口
  hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC,IID_ICaptureGraphBuilder2, (void**)&mICaptureGraphBuilder2);
  if (FAILED(hr))return hr;
  mICaptureGraphBuilder2->SetFiltergraph(mIGraphBuilder);
  hr = mIGraphBuilder->QueryInterface(IID_IMediaControl, (void**)&mIMediaControl);
  if (FAILED(hr))return hr;
  return hr;
}


bool MyCaptrueVideo::EnumDevices()
{
  int id = 0;
  //枚举视频扑捉设备
  ICreateDevEnum* LICreateDevEnum=NULL;
  HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&LICreateDevEnum);
  if (hr != NOERROR) return false;
  IEnumMoniker* LIEnumMoniker=NULL;
 
  hr = LICreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &LIEnumMoniker, 0);
  if (hr != NOERROR) return false;
  LIEnumMoniker->Reset();
  ULONG cFetched;
  IMoniker* LIMoniker = NULL;
  while (hr = LIEnumMoniker->Next(1, &LIMoniker, &cFetched), hr == S_OK)
  {
   IPropertyBag* pBag;
   hr = LIMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pBag);
   if (SUCCEEDED(hr))
   {
    DevicesTuple tp = std::make_tuple(L"",L"");
    VARIANT var;
    var.vt = VT_BSTR;
    hr = pBag->Read(L"FriendlyName", &var, NULL);
    if (hr == NOERROR)
    {
     //char str[2048] = {0};     
     //WideCharToMultiByte(CP_ACP, 0, var.bstrVal, -1, str, 2048, NULL, NULL);     
     //std::string stridx = std::to_string(id);
     //std::string name = str;
     //std::wstring stridx = std::to_wstring(id);
     std::wstring name = std::wstring(var.bstrVal);

     std::get<0>(tp) = name;
     SysFreeString(var.bstrVal);
    }
    hr = pBag->Read(L"DevicePath", &var, NULL);
    if (hr == NOERROR)
    {
     std::wstring path = std::wstring(var.bstrVal);
     std::get<1>(tp) = path;
     SysFreeString(var.bstrVal);
    }

    mDevicesName[id++] = tp;
    pBag->Release();
   }
   LIMoniker->Release();
  }
  return id;
}

void MyCaptrueVideo::InitParameters(IRecieve* rc) {
  mIRecieve = rc;
  int mWidth = 1280;
  int mHeight = 720;
  mTemp = NULL;
  mSrcTemp = NULL;
  mAVFormatContext = NULL;
  mAVIOContext = nullptr;
  mAvioCtxBuffer = nullptr;
  mAvioCtxBufferSize = 1 * 1024 * 1024;//1m
  isHasDecodeInfo = false;
  mVindex = -1;
  mAVCodecContext = NULL;
  mAVCodec = NULL;
  mAVCodecContext = NULL;
  mAVPacket = NULL;
  mAVFrameSrc = NULL;
  mAVFrameDst = NULL;
}

bool MyCaptrueVideo::Init(IRecieve* rc) {

  InitParameters(rc);
  if (FAILED(CoInitialize(NULL))) 
  {   
    return false;
  }
  mIGraphBuilder = NULL;
  mICaptureGraphBuilder2 = NULL;
  mIBaseFilterCamera = NULL;
  mIMediaControl = NULL;

  HRESULT hr;
  hr = InitCaptureGraphBuilder();
  if (FAILED(hr)) {
   return false;
  }

  EnumDevices();
  //if (!BindFilter(iDeviceID, &m_pBF))return false;

  mAVFrameSrc = av_frame_alloc();
  mAVFrameDst = av_frame_alloc();
  return true;
}
bool MyCaptrueVideo::Release() {

 if(mCaptureFilter){
  mCaptureFilter->Stop();
 }
 //mCaptureFilter->Release();
 if (mIMediaControl) {
  mIMediaControl->Stop();
 }

 std::lock_guard<std::mutex> guard(mReceveAndStopMutex);

  // free context and avFrame
  if (mAVPacket) {
   av_packet_unref(mAVPacket);
   av_packet_free(&mAVPacket);
  }
  if (mAVCodecContext) {
   avcodec_free_context(&mAVCodecContext);
  }
  if (mAVFrameSrc) {
   av_frame_free(&mAVFrameSrc);
  }
  if (mAVFrameSrc) {
   av_frame_free(&mAVFrameDst);
  } 

  mIRecieve = NULL;
  SAFE_RELEASE(mICaptureGraphBuilder2);
  SAFE_RELEASE(mIMediaControl);  
  SAFE_RELEASE(mIGraphBuilder);
  SAFE_RELEASE(mIBaseFilterCamera);
  CoUninitialize();
 return true;
}

void MyCaptrueVideo::GetDevices(MapDevices& devices) {
 devices = this->mDevicesName;
}

#include <windows.h>
void writelog(BYTE* p,int size) {
	 static FILE* fp = NULL;
  if(!fp) {
   int err = fopen_s(&fp, "D:\\1111.rgb", "wb+");
   if (err == 0) {
    printf("The file 'crt_fopen_s.c' was opened\n");
   }else{
    printf("The file 'crt_fopen_s.c' was not opened\n");
   }
  }
	 if (!fp) {
	  return; 
	 }
	 fwrite(p, size, 1, fp);	
  fflush(fp);
	 return ;
}

//for wirte mpegj one
void writelogone(BYTE* p, int size) {
 FILE* fp2 = NULL;
 static int i = 0;

 std::string  index = std::to_string(i++);
 std::string name = std::string("D:\\mpeg\\") + index+".jpg";
 if (!fp2) {
  int err = fopen_s(&fp2, name.c_str(), "wb+");
  if(err) {
   printf("The file 'crt_fopen_s.c' was not opened\n");
   return ;
  }
 }
 if (!fp2) {
  return;
 }
 fwrite(p, size, 1, fp2);
 fflush(fp2);
 fclose(fp2);
 fp2 = NULL;
 return;
}
//
//#define SUBTYPE MEDIASUBTYPE_MJPG
#define SUBTYPE MEDIASUBTYPE_YUY2

void Rotate(BYTE* src,BYTE* dst,int jiao,int width,int height) {
 int newwidth = width * bitByteNum;
 int i = 0; int j = 0;
 for (; i < height; i++){
  j=0;
  for (; j < newwidth; j++){
   int srcindex = i * newwidth + j;
   int dstindex = (height-1-i) * newwidth + j;
   dst[dstindex] = src[srcindex];
  }
 }
}

int MyCaptrueVideo::FillBuffer(void* opaque, uint8_t* buf, int len)
{
 //MyCaptrueVideo* pThis = (MyCaptrueVideo*)opaque;
 DecodeTuple tp = *(DecodeTuple*) (opaque);
 
 int less = std::get<1>(tp) < len ? std::get<1>(tp) : len;
 memcpy(buf, std::get<0>(tp),less);
 return  less;
}

bool MyCaptrueVideo::GetHeaderCodeInfo(unsigned char* data, unsigned int dataSize)
{
   //av_register_all();
   //avformat_network_init();   
   mAVFormatContext = avformat_alloc_context();
   mAvioCtxBuffer = (unsigned char*)av_malloc(mAvioCtxBufferSize);
   DecodeTuple tp = std::make_tuple((char*)data, dataSize);
   mAVIOContext = avio_alloc_context(mAvioCtxBuffer, mAvioCtxBufferSize, 0, &tp, MyCaptrueVideo::FillBuffer, NULL, NULL);
   mAVFormatContext->pb = mAVIOContext;
   mAVFormatContext->flags = AVFMT_FLAG_CUSTOM_IO;
   mAVFormatContext->probesize = 256 * 1024;
   mAVFormatContext->max_analyze_duration = 350 * 1000;

   int res = avformat_open_input(&mAVFormatContext, NULL, NULL, NULL);//invoke  MyCaptrueVideo::FillBuffer here ,it sync invoke.
   if (res) {    
    return false;
   }
   res = avformat_find_stream_info(mAVFormatContext, NULL);
   if (res) {    
    return false;
   }
   
   for (int i = 0; i < mAVFormatContext->nb_streams; i++){       
     //if (mAVFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
     if (mAVFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
      mVindex = i;
      break;
     }
   }
   if (-1== mVindex) {
    return false;
   }
   mAVCodec = avcodec_find_decoder(mAVFormatContext->streams[mVindex]->codecpar->codec_id);
   if (mAVCodec == nullptr) {
    return false;
   }
   mAVCodecContext = avcodec_alloc_context3(mAVCodec);
   res =avcodec_parameters_to_context(mAVCodecContext, mAVFormatContext->streams[mVindex]->codecpar);
   res = avcodec_open2(mAVCodecContext, mAVCodec, nullptr);
   //lost of strauct ,we only need allocat once.
   mAVPacket = av_packet_alloc();

   return true;
}

void MyCaptrueVideo::MJPEGToRGB(unsigned char* data, unsigned int dataSize, unsigned char* outBuffer)
{
 mAVPacket->size = dataSize;
 mAVPacket->data = data;
 //解码
 auto ret = avcodec_send_packet(mAVCodecContext, mAVPacket);
 if (ret >= 0) {  
  ret = avcodec_receive_frame(mAVCodecContext, mAVFrameSrc);
  if (ret >= 0) {
   struct SwsContext* convertCxt = sws_getContext(
    mAVFrameSrc->width, mAVFrameSrc->height, mAVCodecContext->pix_fmt,//need use source data's fmt.
    mAVFrameSrc->width, mAVFrameSrc->height, DST_FORMAT,
    SWS_POINT, NULL, NULL, NULL
   );
   av_image_fill_arrays(
    mAVFrameDst->data, mAVFrameDst->linesize, outBuffer,
    DST_FORMAT, mAVFrameSrc->width, mAVFrameSrc->height,
    1
   );
   sws_scale(convertCxt, mAVFrameSrc->data, mAVFrameSrc->linesize, 0, mAVFrameSrc->height, mAVFrameDst->data, mAVFrameDst->linesize);
   sws_freeContext(convertCxt);
  }  
 }

}

void MyCaptrueVideo::YUV2ToRGB(unsigned char* data, unsigned int dataSize, unsigned char* outBuffer)
{

 AVCodecContext* avctx= avcodec_alloc_context3(NULL);

 // Determine required buffer size and allocate buffer
 avctx->width = mWidth;
 avctx->height = mHeight;
 avctx->pix_fmt = AV_PIX_FMT_YUYV422;//src format;
 //src mAVFrameSrc
 
 //int numBytes = avpicture_get_size(AV_PIX_FMT_YUYV422, avctx->width, avctx->height);
 int numBytes = av_image_get_buffer_size(AV_PIX_FMT_YUYV422, avctx->width, avctx->height, 1);//1 align对齐

 //avpicture_fill((AVPicture*)mAVFrameSrc, data, AV_PIX_FMT_YUYV422, avctx->width, avctx->height);
 av_image_fill_arrays(mAVFrameSrc->data, mAVFrameSrc->linesize, data, AV_PIX_FMT_YUYV422, avctx->width, avctx->height, 1);

 mAVFrameSrc->width = mWidth;
 mAVFrameSrc->height = mHeight;
 mAVFrameSrc->format = AV_PIX_FMT_YUYV422;//src format;
 //dst buffer
 //int dstNum = avpicture_get_size(DST_FORMAT, avctx->width, avctx->height); 
 int dstNum = av_image_get_buffer_size(DST_FORMAT, avctx->width, avctx->height, 1);//1 align对齐

 //avpicture_fill((AVPicture*)mAVFrameDst, outBuffer, DST_FORMAT, avctx->width, avctx->height);
 av_image_fill_arrays(mAVFrameDst->data, mAVFrameDst->linesize, outBuffer, DST_FORMAT, avctx->width, avctx->height,1);

 mAVFrameDst->width = mWidth;
 mAVFrameDst->height = mHeight;
 mAVFrameDst->format = DST_FORMAT;//src format;

 //struct SwsContext* img_convert_ctx = sws_getContext(mWidth,mHeight, AV_PIX_FMT_YUYV422,mWidth,mHeight,DST_FORMAT, SWS_BICUBIC, NULL, NULL, NULL);
 struct SwsContext* img_convert_ctx = NULL;
 img_convert_ctx = sws_getCachedContext(img_convert_ctx, avctx->width,
  avctx->height, avctx->pix_fmt, avctx->width, avctx->height,
  DST_FORMAT, SWS_BICUBIC, NULL, NULL, NULL);
 if (!img_convert_ctx)
 {
  fprintf(stderr, "Cannot initialize sws conversion context\n");
 }

 sws_scale(img_convert_ctx, (const uint8_t* const*)mAVFrameSrc->data,
  mAVFrameSrc->linesize, 0, avctx->height, mAVFrameDst->data,
  mAVFrameDst->linesize);
 
 //return pFrameRGB;
}



void MyCaptrueVideo::Receive(bool isVideo, IMediaSample* sample)
{  
  std::lock_guard<std::mutex> guard(mReceveAndStopMutex);

  BYTE* ptr;
  MediaTypePtr mt;
  long roll = 0;
  if (!sample)
   return;
  //rotate
  ComQIPtr<IAMCameraControl> cc(this->mIBaseFilterCamera);
  if (cc) {
   long ccf = 0;
   cc->Get(CameraControl_Roll, &roll, &ccf);
  }
  //end
  //if (sample->GetMediaType(&mt) == S_OK) {
  // if (isVideo) {
  //  videoMediaType = mt;    
  // }else{
  // }
  //}
  int size = sample->GetActualDataLength();
  if (!size)
   return;

  if (FAILED(sample->GetPointer(&ptr)))
   return;

  long long startTime=0, stopTime=0;
  bool hasTime = SUCCEEDED(sample->GetTime(&startTime, &stopTime));  
  //decode  jpg to rgb32
  int outsize = mWidth * mHeight* bitByteNum;//rgba
  //jpeg begin
  if (!isHasDecodeInfo && MEDIASUBTYPE_MJPG ==videoMediaType->subtype) {//MEDIASUBTYPE_MJPG
   isHasDecodeInfo = true;
   GetHeaderCodeInfo(ptr, size);
  }
  if(MEDIASUBTYPE_MJPG == videoMediaType->subtype) {//47504A4D-0000-0010-8000-00AA00389B71
    MJPEGToRGB(ptr,size,mSrcTemp); 
    //writelogone(ptr,size);
  }
  //end jpeg
  if (MEDIASUBTYPE_YUY2 == videoMediaType->subtype) {//32595559-0000-0010-8000-00AA00389B71
   YUV2ToRGB(ptr, size, mSrcTemp);//13 room
   //writelogone(mSrcTemp, outsize);
   //writelogone(ptr, size);
  }
  //writelog(mSrcTemp, outsize);

  Rotate(mSrcTemp,mTemp,180, mWidth, mHeight);
  mIRecieve->OnReceive(mTemp, outsize, mWidth, mHeight);
  //mIRecieve->OnReceive(mSrcTemp, outsize, mWidth, mHeight);
  
  //writelog(ptr,size);
  //MEDIASUBTYPE_RGB24 E436EB7D-524F-11CE-9F53-0020AF0BA770
}

//get fps


HRESULT MyCaptrueVideo::GetCaptureRatio(ComQIPtr<IAMStreamConfig> pam) {
 HRESULT hr;
 //ComQIPtr<IAMStreamConfig> pam;
 //hr = pBuild->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video,
 // pCapFilter, IID_IAMStreamConfig, reinterpret_cast<void**>(&pam)); // 得到媒体控制接口
 int nCount = 0;
 int nSize = 0;
 hr = pam->GetNumberOfCapabilities(&nCount, &nSize);
 ofstream outfile("captureframe.txt"); // 准备写入文件
// 判断是否为视频信息
 if (sizeof(VIDEO_STREAM_CONFIG_CAPS) == nSize) {
  for (int i = 0; i < nCount; i++) {
   VIDEO_STREAM_CONFIG_CAPS scc;
   AM_MEDIA_TYPE* pmmt;
   hr = pam->GetStreamCaps(i, &pmmt, reinterpret_cast<BYTE*>(&scc));
   if (pmmt->formattype == FORMAT_VideoInfo) {
    VIDEOINFOHEADER* pvih = reinterpret_cast<VIDEOINFOHEADER*>(pmmt->pbFormat);
    int nFrame = pvih->AvgTimePerFrame; // 得到采集的帧率
    float fps = 1.0 / (nFrame * 100.0 / 1000000000.0);    
    //LPOLESTR szGUID = NULL;
    char sz[1024] = { 0 };
    //StringFromCLSID(pmmt->subtype, &szGUID);
    char end[16] = {0};
    std::string str="";
    for (int i = 0; i < 8; i++) {
     sprintf_s(end, sizeof(end), "%02X",pmmt->subtype.Data4[i]);     
     str += end;
     if (1 == i) {
      str += "-";
     }
    }
    sprintf_s(sz,sizeof(sz),"%02d %X-%04X-%04X-%s w:%04d h:%04d",(int)fps,pmmt->subtype.Data1, pmmt->subtype.Data2, pmmt->subtype.Data3,str.c_str(), pvih->bmiHeader.biWidth, pvih->bmiHeader.biHeight);
    outfile << sz << endl; // 写入文件
   }
  }
 }
 outfile.close(); // 关闭文件
 return(hr);
}



 bool PinConfigHasMajorType(IPin* pin)
{
 HRESULT hr;
 ComPtr<IAMStreamConfig> config;
 int count, size;

 hr = pin->QueryInterface(IID_IAMStreamConfig, (void**)&config);
 if (FAILED(hr))
  return false;

 hr = config->GetNumberOfCapabilities(&count, &size);
 if (FAILED(hr))
  return false;

 ofstream outfile("captureframe2.txt"); // 准备写入文件
 for (int i = 0; i < count; i++) {
   AM_MEDIA_TYPE* pmmt;
   VIDEO_STREAM_CONFIG_CAPS scc;
  if (SUCCEEDED(config->GetStreamCaps(i, &pmmt, reinterpret_cast<BYTE*>(&scc) )))
   if (pmmt->formattype == FORMAT_VideoInfo) {
    VIDEOINFOHEADER* pvih = reinterpret_cast<VIDEOINFOHEADER*>(pmmt->pbFormat);
    int nFrame = pvih->AvgTimePerFrame; // 得到采集的帧率
    float fps = 1.0 / (nFrame * 100.0 / 1000000000.0);
    outfile << fps << endl; // 写入文件
   }    
 }
 outfile.close(); // 关闭文件
 return false;
}

void MyCaptrueVideo::InitCaptrueFilter() {
 PinCaptureInfo info;
 info.callback = [this](IMediaSample* s) { Receive(true, s); };
 info.expectedMajorType =  MEDIATYPE_Video;
 info.expectedSubType = SUBTYPE;
 mCaptureFilter = new DShow::CaptureFilter(info);


 HRESULT hr;
 //set
 ComPtr<IPin> pin;
 bool success = GetFilterPin(mIBaseFilterCamera, MEDIATYPE_Video, PIN_CATEGORY_CAPTURE,PINDIR_OUTPUT, &pin);
 ComQIPtr<IAMStreamConfig> pinConfig(pin);
 if (pinConfig == NULL) {  
  return ;
 }
 //get support frames;
 GetCaptureRatio(pinConfig);
 //PinConfigHasMajorType(pin);
 //end get 
 MediaTypePtr defaultMT;
 hr = pinConfig->GetFormat(&defaultMT);

 //if (hr == E_NOTIMPL) {
 // //if (!GetPinMediaType(pin, videoMediaType)) {   
 // // return;
 // //}
 // int k = 0;
 // k++;
 //}else if (FAILED(hr)) {  
 // return ;
 //}else {
 // videoMediaType = defaultMT;
 //}
 if (S_OK != hr) {
  return;
 }
 videoMediaType = defaultMT;
 videoMediaType->majortype = MEDIATYPE_Video;
 videoMediaType->subtype = SUBTYPE;
 //videoMediaType->majortype = defaultMT->majortype;
 //videoMediaType->subtype = defaultMT->subtype;

 VIDEOINFOHEADER* vihs = (VIDEOINFOHEADER*)videoMediaType->pbFormat;
 vihs->bmiHeader.biWidth = 1280;//my note book camper max.or will callfailed.
 vihs->bmiHeader.biHeight = 720;
 //vihs->bmiHeader.biWidth = 1920;
 //vihs->bmiHeader.biHeight = 1080;
 //vihs->AvgTimePerFrame = 333333;//30 fps
 vihs->AvgTimePerFrame = 500000;
 //vihs->AvgTimePerFrame = 166666;//60 fps
 //vihs->AvgTimePerFrame = 83333;//120 fps
 
 hr = pinConfig->SetFormat(videoMediaType);
 if (FAILED(hr)) {  
  return ;
 }
 //set camer exposer 曝光器
 //ref https://learn.microsoft.com/zh-cn/windows-hardware/drivers/stream/ksproperty-cameracontrol-exposure

 ComQIPtr<IAMCameraControl> cc(this->mIBaseFilterCamera);
 //建议-7~1
 //cc->Set(CameraControl_Exposure, -7, CameraControl_Flags_Manual);//if less 0,1/(2^n)
 //cc->Set(CameraControl_Exposure, 1, CameraControl_Flags_Auto);
 //long value = 0;
 //long type = CameraControl_Flags_Auto;
 //cc->Get(CameraControl_Exposure, &value,&type);
 //end

 hr = mIGraphBuilder->AddFilter(mCaptureFilter, L"Video Capture Filter");
 hr = mICaptureGraphBuilder2->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, mIBaseFilterCamera, NULL, mCaptureFilter);
 if (FAILED(hr)) {
  hr = mICaptureGraphBuilder2->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, mIBaseFilterCamera, NULL, mCaptureFilter);
  if (FAILED(hr)) {
   return;
  }
 }

 //query cur setting
 AM_MEDIA_TYPE mt;
 ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));
 mt.majortype = MEDIATYPE_Video;
 mt.subtype = SUBTYPE;
 
 mCaptureFilter->GetPin()->ConnectionMediaType(&mt);

 VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*)mt.pbFormat;
 mWidth = vih->bmiHeader.biWidth;
 mHeight = vih->bmiHeader.biHeight; 
 mTemp = new BYTE[mWidth*mHeight* bitByteNum];//rgb24
 mSrcTemp = new BYTE[mWidth * mHeight * bitByteNum];//rgb24

 //log
 ofstream outfile("useing.txt"); // 准备写入文件
 float fps = 1.0 / (vih->AvgTimePerFrame * 100.0 / 1000000000.0);
 //LPOLESTR szGUID = NULL;
 char sz[1024] = { 0 };
 //StringFromCLSID(pmmt->subtype, &szGUID);
 char end[16] = { 0 };
 std::string str = "";
 for (int i = 0; i < 8; i++) {
  sprintf_s(end, sizeof(end), "%02X", mt.subtype.Data4[i]);
  str += end;
  if (1 == i) {
   str += "-";
  }
 }
 sprintf_s(sz, sizeof(sz), "%02d %X-%04X-%04X-%s w:%04d h:%04d", (int)fps, mt.subtype.Data1, mt.subtype.Data2, mt.subtype.Data3, str.c_str(), vih->bmiHeader.biWidth, vih->bmiHeader.biHeight);
 outfile << sz << endl;
 //log end


 FreeMediaType(mt);
 //end query

 hr = mIMediaControl->Run();//开始视频捕捉
 if(FAILED(hr)) { return ; } 
}
void MyCaptrueVideo::StartCaptrue(int index) {
 if(!BindFilter(index,&mIBaseFilterCamera) ) return ;
 HRESULT hr = mIGraphBuilder->AddFilter(mIBaseFilterCamera, L"Capture Filter");

 InitCaptrueFilter();

}
void MyCaptrueVideo::StopCaptrue() {

}

//enum device and get camera IBaseFilter;
bool MyCaptrueVideo::BindFilter(int deviceId, IBaseFilter** pFilterCamera) {
  if (deviceId < 0) return false;

  DevicesTuple tp = mDevicesName[deviceId];
  std::wstring name = std::get<0>(tp);
  std::wstring path = std::get<1>(tp);//we should use name and path compare enum value.maybe id is not equal as before.

  int id = 0;
  //枚举视频扑捉设备
  ICreateDevEnum* LICreateDevEnum = NULL;
  HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&LICreateDevEnum);
  if (hr != NOERROR) return false;
  IEnumMoniker* LIEnumMoniker = NULL;

  hr = LICreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &LIEnumMoniker, 0);
  if (hr != NOERROR) return false;
  LIEnumMoniker->Reset();
  ULONG cFetched;
  IMoniker* LIMoniker = NULL;
  while (hr = LIEnumMoniker->Next(1, &LIMoniker, &cFetched), hr == S_OK, id <= deviceId)//has index <= deviceId
  {
   IPropertyBag* pBag;
   hr = LIMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pBag);
   if (SUCCEEDED(hr))
   {
    VARIANT var;
    var.vt = VT_BSTR;
    hr = pBag->Read(L"FriendlyName", &var, NULL);
    if (hr == NOERROR)
    {
     if(id == deviceId){
      LIMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)pFilterCamera);
     }
     std::wstring name = std::wstring(var.bstrVal);     
     SysFreeString(var.bstrVal);
    }
    pBag->Release();    
   }
   LIMoniker->Release();
   id++;
  }
  return true;

}