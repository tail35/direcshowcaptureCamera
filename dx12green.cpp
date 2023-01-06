#include "dx12green.h"
#include "common.h"


dx12green::dx12green(QWidget *parent)
    : QMainWindow(parent), mCurIndex(0)
{
    ui.setupUi(this);   
    
    //貌似子控件的边框只能在父控件类代码里面绘制。
    ui.myQD3DWidget->setStyleSheet("border: 1px solid red;");
    ui.myQD3DWidget->GetDevices(mDevices);
    for(MapPair kv: this->mDevices) {
     QString key = QString::fromStdWString(std::to_wstring(kv.first));      
     DevicesTuple tp = kv.second;
     QString name = QString::fromStdWString(std::get<0>(tp));
     ui.comboBoxCamList->addItem(name,key);
    }
    connect(ui.comboBoxCamList, QOverload<int>::of(&QComboBox::currentIndexChanged),
     [=](int index) { 
       mCurIndex = index;
     });
    connect(ui.pushButtonStart, &QPushButton::clicked, [this] {
     ui.myQD3DWidget->InitD3D12HelloTexture();
     this->ui.myQD3DWidget->StartCaptrue(this->mCurIndex);
     
    });
    connect(ui.pushButtonStop, &QPushButton::clicked, [this] {
      
     });

    ResizePosition();

    ////opencv
    //// 读入一张图片（游戏原画）    
    //cv::Mat img = cv::imread("D:\\mpeg1\\55.jpg");
    //// 创建一个名为 "游戏原画"窗口    
    //cv::namedWindow("游戏原画");
    //// 在窗口中显示游戏原画    
    //cv::imshow("游戏原画", img);
    //// 等待6000 ms后窗口自动关闭    
    //cv::waitKey(6000);    
}



void dx12green::ComboIndexChange(int index) { 
}

void  dx12green::ResizePosition() {
 int width = this->width();
 int heidht = this->height();
 int vheight = heidht-100;
 int vwidht = 16*vheight/9;
 int vx = (width - vwidht) / 2;
 int vy = 10;

 QRect rct(vx,vy,vwidht,vheight);
 //ui.myQD3DWidget->setFrameRect(rct);
 ui.myQD3DWidget->move(vx,vy);
 ui.myQD3DWidget->setFixedHeight(vheight);
 ui.myQD3DWidget->setFixedWidth(vwidht);
 //compositon
 int comx = vx;
 int comh = vheight+30;
 ui.comboBoxCamList->move(comx,comh);
 int startx = vx + ui.comboBoxCamList->width() + 30;
 int starty = comh;
 ui.pushButtonStart->move(startx,starty);

 int stopx = startx + ui.pushButtonStart->width() + 30;
 int stopy = comh;
 ui.pushButtonStop->move(stopx, stopy);


}
void dx12green::changeEvent(QEvent* event) {
  int ev = event->type();

  if (event->type() != QEvent::WindowStateChange) {
   return;
  }
  ResizePosition();
 //int st = this->windowState();
 // if (this->windowState() == Qt::WindowMaximized)
 // {
 //  int i = 0;
 //  i++;
 // }
 // if (this->windowState() == Qt::WindowMinimized)
 // {
 //  int i = 0;
 //  i++;
 // }  
}


void dx12green::resizeEvent(QResizeEvent* event) {
 ResizePosition();
 ui.myQD3DWidget->ResizeSize();
  int st = this->windowState();
  if (this->windowState() == Qt::WindowMaximized)
  {
  }
  if (this->windowState() == Qt::WindowMinimized)
  {
  }
}

void dx12green::closeEvent(QCloseEvent* event) {
  ui.myQD3DWidget->StopCaptrue(); 
  ui.myQD3DWidget->Release();
}