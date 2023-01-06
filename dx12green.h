#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_dx12green.h"
#include "common.h"


class dx12green : public QMainWindow
{
    Q_OBJECT
    
public:
    dx12green(QWidget *parent = Q_NULLPTR);
    void GetDevices();
    void ComboIndexChange(int index);
    void  ResizePosition();
protected:
 void changeEvent(QEvent* event);
 virtual void resizeEvent(QResizeEvent* event) override;
 void closeEvent(QCloseEvent* event);
public:

 
    MapDevices mDevices;
    int mCurIndex;    
private:
    Ui::dx12greenClass ui;
};
