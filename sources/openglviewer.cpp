#include "openglviewer.h"

#include <iostream>
#include <string>
#include <vector>

#include <QtGui/qevent.h>
#include <QtGui/qopenglcontext.h>
#include <QtGui/qopenglextrafunctions.h>
#include <QtGui/qmatrix4x4.h>
#include <QtGui/qvector3d.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "glutils.h"
#include "common.h"

static constexpr int POSITION_LOC = 0;
static constexpr int NORMAL_LOC = 1;

static const int SHADOWMAP_SIZE = 2048;

static const QVector3D LIGHT_POS = QVector3D(-10.0f, 15.0f, 6.0f);

static const float floorPos[] = {
    -15.0f, -5.0f, -15.0f,
    -15.0f, -5.0f,  15.0f,
     15.0f, -5.0f, -15.0f,
     15.0f, -5.0f,  15.0f,
};

static const float floorNrm[] = {
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f
};

static const unsigned int floorIds[] = {
    0, 1, 3, 0, 3, 2
};

static const float rectPos[] = {
    -1.0f, -1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f,
     1.0f, -1.0f, 0.0f,
     1.0f,  1.0f, 0.0f
};

static const unsigned int rectIds[] {
    0, 3, 1, 0, 2, 3
};

OpenGLViewer::OpenGLViewer(QWidget *parent)
    : QOpenGLWidget(parent)
    , params(80.0, 7.0, 10.0) {

    // Setup timer
    animTimer = std::make_unique<QTimer>(this);
    connect(animTimer.get(), SIGNAL(timeout()), this, SLOT(OnAnimate()));
    animTimer->start();
    
    // Prepare arcball controller
    arcball = std::make_unique<ArcballController>(this);
    
    QMatrix4x4 mMat, vMat;
    mMat.setToIdentity();
    vMat.lookAt(QVector3D(0.0f, 0.0f, 7.0f), QVector3D(0.0f, 0.0f, 0.0f), QVector3D(0.0f, 1.0f, 0.0f));
    arcball->initModelView(mMat, vMat);
}

OpenGLViewer::~OpenGLViewer() {
}

void OpenGLViewer::initializeGL() {
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    // Load OBJ file.
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string errMsg;
    std::string filename = std::string(DATA_DIRECTORY) + "teapot.obj";
    bool success = tinyobj::LoadObj(shapes, materials, errMsg, filename.c_str());
    if (!errMsg.empty()) {
        std::cerr << "[ERROR] " << errMsg << std::endl;
    }
    
    if (!success) {
        std::cerr << "[ERROR] Failed to load OBJ file!!" << std::endl;
        std::exit(1);
    }
    
    // Prepare VAO for object mesh.
    vao = std::make_unique<QOpenGLVertexArrayObject>(this);
    vao->create();
    vao->bind();
    
    auto& mesh = shapes[0].mesh;
    
    vBuffer = std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);
    vBuffer->create();
    vBuffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
    vBuffer->bind();
    vBuffer->allocate(mesh.positions.size() * sizeof(float) +
                      mesh.normals.size() * sizeof(float));
    
    auto f = QOpenGLContext::currentContext()->extraFunctions();
    f->glEnableVertexAttribArray(POSITION_LOC);
    f->glEnableVertexAttribArray(NORMAL_LOC);

    int offset = 0;
    vBuffer->write(offset, &mesh.positions[0], mesh.positions.size() * sizeof(float));
    f->glVertexAttribPointer(POSITION_LOC, 3, GL_FLOAT, GL_FALSE, 0, (void*)offset);
    
    offset += mesh.positions.size() * sizeof(float);
    vBuffer->write(offset, &mesh.normals[0], mesh.normals.size() * sizeof(float));
    f->glVertexAttribPointer(NORMAL_LOC, 3, GL_FLOAT, GL_FALSE, 0, (void*)offset);
    
    iBuffer = std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::IndexBuffer);
    iBuffer->create();
    iBuffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
    iBuffer->bind();
    iBuffer->allocate(&mesh.indices[0], mesh.indices.size() * sizeof(unsigned int));
    
    vao->release();
    
    // Prepare VAO for floor
    floorVao = std::make_unique<QOpenGLVertexArrayObject>(this);
    floorVao->create();
    floorVao->bind();
    
    floorVBuffer = std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);
    floorVBuffer->create();
    floorVBuffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
    floorVBuffer->bind();
    floorVBuffer->allocate((4 * 3) * 2 * sizeof(float));
    
    f->glEnableVertexAttribArray(POSITION_LOC);
    f->glEnableVertexAttribArray(NORMAL_LOC);
    
    offset = 0;
    floorVBuffer->write(offset, floorPos, (4 * 3) * sizeof(float));
    f->glVertexAttribPointer(POSITION_LOC, 3, GL_FLOAT, GL_FALSE, 0, (void*)offset);
    offset += (4 * 3) * sizeof(float);
    floorVBuffer->write(offset, floorNrm, (4 * 3) * sizeof(float));
    f->glVertexAttribPointer(NORMAL_LOC, 3, GL_FLOAT, GL_FALSE, 0, (void*)offset);
    
    floorIBuffer = std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::IndexBuffer);
    floorIBuffer->create();
    floorIBuffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
    floorIBuffer->bind();
    floorIBuffer->allocate(floorIds, 6 * sizeof(unsigned int));
    
    floorVao->release();
    
    // Prepare VAO for rectangle
    rectVao = std::make_unique<QOpenGLVertexArrayObject>(this);
    rectVao->create();
    rectVao->bind();
    
    rectVBuffer = std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);
    rectVBuffer->create();
    rectVBuffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
    rectVBuffer->bind();
    rectVBuffer->allocate(rectPos, 12 * sizeof(float));
    
    f->glEnableVertexAttribArray(POSITION_LOC);
    f->glVertexAttribPointer(POSITION_LOC, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
    rectIBuffer = std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::IndexBuffer);
    rectIBuffer->create();
    rectIBuffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
    rectIBuffer->bind();
    rectIBuffer->allocate(rectIds, 6 * sizeof(unsigned int));
    
    rectVao->release();
    
    // Shader
    shader = compileShader(std::string(SHADER_DIRECTORY) + "render");
    smShader = compileShader(std::string(SHADER_DIRECTORY) + "shadowmaps");
    blurShader = compileShader(std::string(SHADER_DIRECTORY) + "blur");
    
    // Shadow maps
    QOpenGLFramebufferObjectFormat format;
    format.setMipmap(true);
    format.setTextureTarget(GL_TEXTURE_2D);
    format.setAttachment(QOpenGLFramebufferObject::Attachment::Depth);
    format.setInternalTextureFormat(GL_RGBA32F);
    depthFbo = std::make_unique<QOpenGLFramebufferObject>(SHADOWMAP_SIZE, SHADOWMAP_SIZE, format);
}

void OpenGLViewer::paintGL() {
    if (!vao) return;
  
    switch (smType) {
    case SMType::Normal:
    case SMType::PCF:
    case SMType::Variance:
        shadowMaps();
        break;
        
    case SMType::Convolution:
        break;
        
    case SMType::Exponential:
        exponentialShadowMaps();
        break;
        
    default:
        fprintf(stderr, "[ERROR] Unknown SM algorithm: %d\n", (int)smType);
        std::exit(1);
    }
}

void OpenGLViewer::shadowMaps() {
// Shadow mapping
    int vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    glViewport(0, 0, SHADOWMAP_SIZE, SHADOWMAP_SIZE);

    QMatrix4x4 depthMvp;
    {
        smShader->bind();
        depthFbo->bind();
        
        QMatrix4x4 mvMat, pMat;
        mvMat.lookAt(LIGHT_POS, QVector3D(0.0f, 0.0f, 0.0f), QVector3D(0.0f, 1.0f, 0.0f));
        pMat.perspective(45.0f, 1.0f, 0.1f, 100.0f);
        depthMvp = pMat * mvMat;

        smShader->setUniformValue("u_smType", (int)smType);
        smShader->setUniformValue("u_mvpMat", depthMvp);
        
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        
        vao->bind();
        glDrawElements(GL_TRIANGLES, iBuffer->size() / sizeof(unsigned int), GL_UNSIGNED_INT, 0);
        vao->release();
        
        floorVao->bind();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        floorVao->release();
        
        smShader->release();
        depthFbo->release();
    }
    glViewport(vp[0], vp[1], vp[2], vp[3]);
    
    // Render path
    {
        shader->bind();
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        QMatrix4x4 pMat, mvMat;
        pMat.perspective(45.0f, (float)width() / (float)height(), 0.1f, 100.0f);
        mvMat = arcball->modelViewMat();
        
        QVector3D cameraPos = (mvMat.inverted() * QVector4D(0.0f, 0.0f, 0.0f, 1.0f)).toVector3DAffine();
        
        QMatrix4x4 mvpMat = pMat * mvMat;
        shader->setUniformValue("u_mvpMat", mvpMat);
        shader->setUniformValue("u_lightPos", LIGHT_POS);
        shader->setUniformValue("u_cameraPos", cameraPos);
        shader->setUniformValue("u_depthMvp", depthMvp);
        shader->setUniformValue("u_kernelSize", params.kernelSize / SHADOWMAP_SIZE);
        shader->setUniformValue("u_smType", (int)smType);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthFbo->texture());
        shader->setUniformValue("u_depthMap", 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        vao->bind();
        glDrawElements(GL_TRIANGLES, iBuffer->size() / sizeof(unsigned int), GL_UNSIGNED_INT, 0);
        vao->release();
        
        floorVao->bind();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        floorVao->release();
        
        shader->release();
        vao->release();
    }
}

void OpenGLViewer::exponentialShadowMaps() {
    // Shadow mapping
    int vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    glViewport(0, 0, SHADOWMAP_SIZE, SHADOWMAP_SIZE);

    QMatrix4x4 depthMvp;
    {
        smShader->bind();
        depthFbo->bind();
        
        QMatrix4x4 mvMat, pMat;
        mvMat.lookAt(LIGHT_POS, QVector3D(0.0f, 0.0f, 0.0f), QVector3D(0.0f, 1.0f, 0.0f));
        pMat.perspective(45.0f, 1.0f, 0.1f, 100.0f);
        depthMvp = pMat * mvMat;

        smShader->setUniformValue("u_esmCoeff", params.esmCoeff);
        smShader->setUniformValue("u_mvpMat", depthMvp);
        smShader->setUniformValue("u_smType", (int)smType);
        
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        
        vao->bind();
        glDrawElements(GL_TRIANGLES, iBuffer->size() / sizeof(unsigned int), GL_UNSIGNED_INT, 0);
        vao->release();
        
        floorVao->bind();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        floorVao->release();
        
        smShader->release();
        depthFbo->release();
    }
    
    // Blur (horizontal)
    {
        unsigned int depthTexId = depthFbo->takeTexture();
        
        blurShader->bind();
        rectVao->bind();
        depthFbo->bind();
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthTexId);
        blurShader->setUniformValue("u_depthMap", 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        blurShader->setUniformValue("u_pixelSize", params.kernelSize / SHADOWMAP_SIZE);
        blurShader->setUniformValue("u_isHorizontal", 1);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glDeleteTextures(1, &depthTexId);
        
        blurShader->release();
        rectVao->release();
        depthFbo->release();
    }
    
    // Blur (vertical)
    {
        unsigned int depthTexId = depthFbo->takeTexture();
        
        blurShader->bind();
        rectVao->bind();
        depthFbo->bind();
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthTexId);
        blurShader->setUniformValue("u_depthMap", 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        blurShader->setUniformValue("u_pixelSize", params.kernelSize / SHADOWMAP_SIZE);
        blurShader->setUniformValue("u_isHorizontal", 0);
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glDeleteTextures(1, &depthTexId);
        
        blurShader->release();
        rectVao->release();
        depthFbo->release();
    }
    glViewport(vp[0], vp[1], vp[2], vp[3]);
    
    // Render path
    {
        shader->bind();
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        QMatrix4x4 pMat, mvMat;
        pMat.perspective(45.0f, (float)width() / (float)height(), 0.1f, 100.0f);
        mvMat = arcball->modelViewMat();
        
        QVector3D cameraPos = (mvMat.inverted() * QVector4D(0.0f, 0.0f, 0.0f, 1.0f)).toVector3DAffine();
        
        QMatrix4x4 mvpMat = pMat * mvMat;
        shader->setUniformValue("u_mvpMat", mvpMat);
        shader->setUniformValue("u_lightPos", LIGHT_POS);
        shader->setUniformValue("u_cameraPos", cameraPos);
        shader->setUniformValue("u_depthMvp", depthMvp);
        shader->setUniformValue("u_esmCoeff", params.esmCoeff);
        shader->setUniformValue("u_darkness", params.darkness);
        shader->setUniformValue("u_smType", (int)smType);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthFbo->texture());
        shader->setUniformValue("u_depthMap", 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        vao->bind();
        glDrawElements(GL_TRIANGLES, iBuffer->size() / sizeof(unsigned int), GL_UNSIGNED_INT, 0);
        vao->release();
        
        floorVao->bind();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        floorVao->release();
        
        shader->release();
        vao->release();
    }
}

void OpenGLViewer::OnAnimate() {
    update();
}

void OpenGLViewer::resizeGL(int w, int h) {
    glViewport(0, 0, width(), height());
}

void OpenGLViewer::mousePressEvent(QMouseEvent *ev) {
    arcball->setOldPoint(ev->pos());
    arcball->setNewPoint(ev->pos());
    if (ev->button() == Qt::LeftButton) {
        arcball->setMode(ArcballMode::Rotate);
    } else if (ev->button() == Qt::RightButton) {
        arcball->setMode(ArcballMode::Translate);
    } else if (ev->button() == Qt::MiddleButton) {
        arcball->setMode(ArcballMode::Scale);
    }
}

void OpenGLViewer::mouseMoveEvent(QMouseEvent *ev) {
    arcball->setNewPoint(ev->pos());
    arcball->update();
    arcball->setOldPoint(ev->pos());
}

void OpenGLViewer::mouseReleaseEvent(QMouseEvent *ev) {
    arcball->setMode(ArcballMode::None);
}

void OpenGLViewer::wheelEvent(QWheelEvent *ev) {
    arcball->setScroll(arcball->scroll() + ev->delta() / 1000.0);
    arcball->update();
}
