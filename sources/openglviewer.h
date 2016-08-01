#ifdef _MSC_VER
#pragma once
#endif

#ifndef _OPENGL_VIEWER_H_
#define _OPENGL_VIEWER_H_

#include <memory>

#include <QtCore/qtimer.h>
#include <QtWidgets/qopenglwidget.h>
#include <QtGui/qopenglshaderprogram.h>
#include <QtGui/qopenglvertexarrayobject.h>
#include <QtGui/qopenglbuffer.h>
#include <QtGui/qopenglframebufferobject.h>

#include "arcballcontroller.h"

enum class SMType : int {
    Normal      = 0x00,
    PCF         = 0x01,
    Variance    = 0x02,
    Convolution = 0x03,
    Exponential = 0x04
};

struct ESMParams {
    ESMParams(float coeff, float ksize, float dark)
        : esmCoeff(coeff)
        , kernelSize(ksize)
        , darkness(dark) {
    }
    
    float esmCoeff, kernelSize, darkness;
};

class OpenGLViewer : public QOpenGLWidget {
    Q_OBJECT

public:
    explicit OpenGLViewer(QWidget *parent = nullptr);
    virtual ~OpenGLViewer();
    
    void setParams(const ESMParams& newParams) {
        params = newParams;
    }
    
    void setAlgorithm(SMType type) {
        smType = type;
    }
    
protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;
    
    void mousePressEvent(QMouseEvent *ev) override;
    void mouseMoveEvent(QMouseEvent *ev) override;
    void mouseReleaseEvent(QMouseEvent *ev) override;
    void wheelEvent(QWheelEvent *ev) override;
    
private slots:
    void OnAnimate();
    
private:
    // Private methods
    void shadowMaps();
    void exponentialShadowMaps();

    // Private parameters
    std::unique_ptr<QOpenGLVertexArrayObject> vao = nullptr;
    std::unique_ptr<QOpenGLBuffer> vBuffer = nullptr;
    std::unique_ptr<QOpenGLBuffer> iBuffer = nullptr;
    
    std::unique_ptr<QOpenGLVertexArrayObject> rectVao = nullptr;
    std::unique_ptr<QOpenGLBuffer> rectVBuffer = nullptr;
    std::unique_ptr<QOpenGLBuffer> rectIBuffer = nullptr;
    
    std::unique_ptr<QOpenGLVertexArrayObject> floorVao = nullptr;
    std::unique_ptr<QOpenGLBuffer> floorVBuffer = nullptr;
    std::unique_ptr<QOpenGLBuffer> floorIBuffer = nullptr;
    
    std::unique_ptr<QOpenGLShaderProgram> shader = nullptr;
    std::unique_ptr<QOpenGLShaderProgram> smShader = nullptr;
    std::unique_ptr<QOpenGLShaderProgram> blurShader = nullptr;

    std::unique_ptr<QOpenGLFramebufferObject> depthFbo = nullptr;
    
    std::unique_ptr<ArcballController> arcball = nullptr;
    
    std::unique_ptr<QTimer> animTimer = nullptr;
    
    SMType smType = SMType::Normal;
    ESMParams params;
};

#endif  // _OPENGL_VIEWER_H_
