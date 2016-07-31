#ifdef _MSC_VER
#pragma once
#endif

#include <QtWidgets/qwidget.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qgridlayout.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qslider.h>

class ValueSlider : public QWidget {
    Q_OBJECT

public:
    // Public methods
    ValueSlider(QWidget* parent = nullptr)
        : QWidget(parent) {
        layout = new QGridLayout(this);
        setLayout(layout);
        layout->setColumnStretch(0, 1);
        layout->setColumnStretch(1, 4);

        titleLabel = new QLabel(this);
        layout->addWidget(titleLabel, 0, 0, 1, 2, Qt::AlignLeft);

        valueText = new QLineEdit(this);
        layout->addWidget(valueText, 1, 0, 1, 1, Qt::AlignLeft);

        slider = new QSlider(this);
        slider->setOrientation(Qt::Horizontal);
        slider->setMaximum(100);
        slider->setTickInterval(1);
        layout->addWidget(slider, 1, 1, 1, 1);

        connect(slider, SIGNAL(valueChanged(int)), this,
                SLOT(OnValueChanged(int)));
    }

    ValueSlider(const QString& title, QWidget* parent = nullptr)
        : ValueSlider(parent) {
        setTitle(title);
    }

    ValueSlider(double vMin, double vMax, QWidget* parent = nullptr) 
        : ValueSlider(parent) {
        setRange(vMin, vMax);
    }

    ValueSlider(const QString& title, double vMin, double vMax,
                QWidget* parent = nullptr)
        : ValueSlider(title, parent) {
        setRange(vMin, vMax);
    }

    virtual ~ValueSlider() {
        delete titleLabel;
        delete valueText;
        delete slider;
        delete layout;
    }

    void setTitle(const QString& title) {
        titleLabel->setText(title);
        titleLabel->repaint();
    }

    void setRange(double vMin, double vMax) {
        minValue = vMin;
        maxValue = vMax;
        updateSlider();
    }

    void setTickCount(int ticks) {
        slider->setMaximum(ticks);
    }

    void setValue(double v) {
        value = v;
        updateSlider();
    }

    double getValue() const {
        return minValue + slider->value() * (maxValue - minValue) / 
               slider->maximum();
    }

private:
    void updateSlider() {
        int ticks = static_cast<int>(slider->maximum() * 
                    (value - minValue) / (maxValue - minValue));
        ticks = std::max(0, std::min(ticks, slider->maximum()));
        slider->setValue(ticks);
    }

signals:
    void valueChanged(double);

private slots:
    void OnValueChanged(int) {
        double val = getValue();
        valueText->setText(QString("%1").arg(val));
        valueText->update();
        valueChanged(val);
    }

private:
    QGridLayout *layout = nullptr;
    QLabel *titleLabel = nullptr;
    QLineEdit *valueText = nullptr;
    QSlider *slider = nullptr;

    double minValue  = 0.0;
    double maxValue  = 1.0;
    double value     = 0.0;

};
