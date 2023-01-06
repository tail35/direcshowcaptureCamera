#pragma once

#include <QFrame>
#include "ui_MyQD3DWidget.h"
#include "MyCaptrueVideo.h"
#include "common.h"
#include "dshowRender/D3D12HelloTexture.h"

//QFrame has border has windows base proterty.in inherrit from QWidget.dhlu
class MyQD3DWidget : public QFrame,public IRecieve
{
    Q_OBJECT

public:
    MyQD3DWidget(QWidget *parent = Q_NULLPTR);
    ~MyQD3DWidget();


    void paintEvent(QPaintEvent*);

    bool InitD3D12HelloTexture();
    void StartCaptrue(int index);
    void StopCaptrue();
    void Release();
    void OnReceive(BYTE* ptr, int size, int width, int height);
    void ResizeSize();
protected:
    void changeEvent(QEvent* event);
public:
 void GetDevices(MapDevices& devices);

private:
 bool mDestroy;//用pain事件和relese  函数保护绘画对象。
 std::unique_ptr<MyCaptrueVideo> mMyCaptrueVideo; 
 std::unique_ptr<D3D12HelloTexture> mD3D12HelloTexture;
 int lasViewWith;
 int lasViewHeight;
 std::mutex mOnPainerAndLoadAssetMutex;
private:
    Ui::MyQD3DWidget ui;
};
