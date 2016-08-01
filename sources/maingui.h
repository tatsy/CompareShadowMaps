#ifdef _MSC_VER
#pragma once
#endif

#ifndef _MAINGUI_H_
#define _MAINGUI_H_

#include <QtWidgets/qmainwindow.h>
#include <QtWidgets/qgridlayout.h>

class OpenGLViewer;

class MainGui : public QMainWindow {
    Q_OBJECT
    
public:
    explicit MainGui(QWidget *parent = nullptr);
    virtual ~MainGui();
    
private slots:
    void OnFrameSwapped();
    void OnAlgorithmChanged(int);
    void OnParamChanged(double);
    
private:
    class Ui;
    Ui *ui;
    OpenGLViewer* view;
    
    QWidget* mainWidget;
    QGridLayout* mainLayout;
};

#endif  // _MAINGUI_H_
