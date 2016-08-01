#ifdef _MSC_VER
#pragma once
#endif

#ifndef _RADIO_BUTTON_GROUP_H_
#define _RADIO_BUTTON_GROUP_H_

#include <cstdio>
#include <vector>

#include <QtWidgets/qwidget.h>
#include <QtWidgets/qgroupbox.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qradiobutton.h>

class RadioButtonGroup : public QWidget {
    Q_OBJECT

public:
    explicit RadioButtonGroup(QWidget *parent = nullptr)
        : QWidget(parent) {
        layout = new QVBoxLayout(this);
        layout->setAlignment(Qt::AlignTop);
        setLayout(layout);
        
        groupBox = new QGroupBox(this);
        layout->addWidget(groupBox);
        
        groupLayout = new QVBoxLayout(groupBox);
        groupBox->setLayout(groupLayout);
    }
    
    RadioButtonGroup(const QString& title, QWidget *parent = nullptr)
        : RadioButtonGroup(parent) {
        setTitle(title);
    }
    
    virtual ~RadioButtonGroup() {
        for (int i = 0; i < radioButtons.size(); i++) {
            delete radioButtons[i];
        }
        delete groupLayout;
        delete groupBox;
        delete layout;
    }
    
    void setTitle(const QString& title) {
        groupBox->setTitle(title);
    }
    
    void addRadioButton(const QString& title, bool isChecked = false) {
        QRadioButton *button = new QRadioButton(title, groupBox);
        button->setChecked(isChecked);
        groupLayout->addWidget(button);
        radioButtons.push_back(button);
        
        connect(button, SIGNAL(toggled(bool)), this, SLOT(OnRadioButtonToggled(bool)));
    }

signals:
    void selectionChanged(int);
    
private slots:
    void OnRadioButtonToggled(bool) {
        for (int i = 0; i < radioButtons.size(); i++) {
            if (radioButtons[i]->isChecked()) {
                emit selectionChanged(i);
                break;
            }
        }
    }
    
private:
    QVBoxLayout* layout;
    QGroupBox* groupBox;
    QVBoxLayout* groupLayout;
    std::vector<QRadioButton*> radioButtons;
};

#endif  // _RADIO_BUTTON_GROUP_H_
