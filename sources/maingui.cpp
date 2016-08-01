#include "maingui.h"
#include "openglviewer.h"

#include <QtCore/qdebug.h>
#include <QtCore/qelapsedtimer.h>
#include <QtWidgets/qboxlayout.h>

#include "valueslider.h"
#include "radiobuttongroup.h"

class MainGui::Ui : public QWidget {
public:
    explicit Ui(QWidget *parent = nullptr)
        : QWidget(parent) {
        layout = new QVBoxLayout(this);
        layout->setAlignment(Qt::AlignTop);
        setLayout(layout);
        
        radioGroup = new RadioButtonGroup("Algorithm", this);
        radioGroup->addRadioButton("SM", true);
        radioGroup->addRadioButton("PCF", false);
        radioGroup->addRadioButton("VSM", false);
        radioGroup->addRadioButton("CSM", false);
        radioGroup->addRadioButton("ESM", false);
        layout->addWidget(radioGroup);
        
        esmCoeffSlider = new ValueSlider("C", 1.0, 120.0, this);
        esmCoeffSlider->setValue(80.0);
        layout->addWidget(esmCoeffSlider);
        
        ksizeSlider = new ValueSlider("Kernel size", 1.0, 25.0, this);
        ksizeSlider->setValue(7.0);
        layout->addWidget(ksizeSlider);
        
        darknessSlider = new ValueSlider("Darkness", 1.0, 25.0, this);
        darknessSlider->setValue(10.0);
        layout->addWidget(darknessSlider);
    }
    
    virtual ~Ui() {
        delete radioGroup;
        delete esmCoeffSlider;
        delete ksizeSlider;
        delete darknessSlider;
        delete layout;
    }
    
    QVBoxLayout* layout;
    RadioButtonGroup* radioGroup;
    ValueSlider* esmCoeffSlider;
    ValueSlider* ksizeSlider;
    ValueSlider* darknessSlider;
};

MainGui::MainGui(QWidget *parent)
    : QMainWindow(parent) {
    mainWidget = new QWidget(this);
    mainLayout = new QGridLayout(mainWidget);
    mainWidget->setLayout(mainLayout);
    setCentralWidget(mainWidget);
    
    mainLayout->setColumnStretch(0, 4);
    mainLayout->setColumnStretch(1, 1);
    
    view = new OpenGLViewer(this);
    mainLayout->addWidget(view, 0, 0);

    ui = new Ui(this);
    mainLayout->addWidget(ui, 0, 1);
        
    connect(view, SIGNAL(frameSwapped()), this, SLOT(OnFrameSwapped()));
    connect(ui->radioGroup, SIGNAL(selectionChanged(int)), this, SLOT(OnAlgorithmChanged(int)));
    connect(ui->esmCoeffSlider, SIGNAL(valueChanged(double)), this, SLOT(OnParamChanged(double)));
    connect(ui->ksizeSlider, SIGNAL(valueChanged(double)), this, SLOT(OnParamChanged(double)));
    connect(ui->darknessSlider, SIGNAL(valueChanged(double)), this, SLOT(OnParamChanged(double)));
}

MainGui::~MainGui() {
    delete ui;
    delete view;
    delete mainLayout;
    delete mainWidget;
}

void MainGui::OnFrameSwapped() {
    static bool isStarted = false;
    static QElapsedTimer timer;
    static long long lastTime;
    
    if (!isStarted) {
        isStarted = true;
        timer.start();
    } else if (timer.elapsed() > 500) {
        long long currentTime = timer.elapsed();
        double fps = 1000.0 / (currentTime - lastTime);
        setWindowTitle(QString("FPS: %1").arg(QString::number(fps, 'f', 2)));
        
        timer.restart();
    }
    lastTime = timer.elapsed();
}

void MainGui::OnAlgorithmChanged(int type) {
    view->setAlgorithm((SMType)type);
}

void MainGui::OnParamChanged(double) {
    double esmCoeff = ui->esmCoeffSlider->getValue();
    double ksize    = ui->ksizeSlider->getValue();
    double darkness = ui->darknessSlider->getValue();
    
    ESMParams params(esmCoeff, ksize, darkness);
    
    view->setParams(params);
}
