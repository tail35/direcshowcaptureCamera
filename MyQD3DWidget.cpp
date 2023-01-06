#include "MyQD3DWidget.h"
#include <QStyleOption>
#include <QPainter>

MyQD3DWidget::MyQD3DWidget(QWidget *parent)
    : QFrame(parent)
{
    mDestroy = false;
    ui.setupUi(this);

    mMyCaptrueVideo = std::make_unique<MyCaptrueVideo>();
    mMyCaptrueVideo->Init(this);
    //ui.
    //constructor dshowrender
    
    this->setFixedWidth(1920);
    this->setFixedHeight(1080);
    lasViewWith = this->width();
    lasViewHeight = this->height();
    InitD3D12HelloTexture();//you must put shaders.hlsl under exe's idr.or cause crash.
}
bool  MyQD3DWidget::InitD3D12HelloTexture() {
 HWND LHWND = (HWND)this->winId();
 mD3D12HelloTexture = std::make_unique<D3D12HelloTexture>(lasViewWith, lasViewHeight, L"D3D12 Hello Texture", LHWND);
 mD3D12HelloTexture->OnInit();
 return true;
}
MyQD3DWidget::~MyQD3DWidget()
{
  
}


void MyQD3DWidget::paintEvent(QPaintEvent*)
{
 
  if (mD3D12HelloTexture&& !mDestroy)
  {
   std::lock_guard<std::mutex> guard(mOnPainerAndLoadAssetMutex);
    mD3D12HelloTexture->OnUpdate();
    mD3D12HelloTexture->OnRender();
  }
}

void MyQD3DWidget::GetDevices(MapDevices& devices) {
  return mMyCaptrueVideo->GetDevices(devices);
}

void MyQD3DWidget::StartCaptrue(int index) {
 mMyCaptrueVideo->StartCaptrue(index);
}
void MyQD3DWidget::StopCaptrue() {
 mMyCaptrueVideo->StopCaptrue();
}

void MyQD3DWidget::Release() {
 mDestroy = true;
 mD3D12HelloTexture->OnDestroy();
 mMyCaptrueVideo->Release();
}


void MyQD3DWidget::ResizeSize() {
 int w = this->width();
 int h = this->height();
 mD3D12HelloTexture->ResetViewport(w,h);
 QWidget::update();//onpain.
}
void MyQD3DWidget::OnReceive(BYTE* ptr, int size, int width, int height) { 
 std::lock_guard<std::mutex> guard(mOnPainerAndLoadAssetMutex);
 //texture.
 mD3D12HelloTexture->LoadVideoTexture(ptr, size, width, height);
 QWidget::update();
 //mD3D12HelloTexture->UpdateRgbData(ptr,size,width,height);
 }

void MyQD3DWidget::changeEvent(QEvent* event) {
 int ev = event->type();
 //if (ev == QEvent::ActivationChange) {
 // static bool init = false;
 // if (init) return;
 // this->InitD3D12HelloTexture();
 // init = true;
 //}
}