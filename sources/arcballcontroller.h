#ifndef _ARCBALL_CONTROLLER_H_
#define _ARCBALL_CONTROLLER_H_

#include <cmath>

#include <QtWidgets/qwidget.h>
#include <QtGui/qvector3d.h>
#include <QtGui/qmatrix4x4.h>

enum class ArcballMode : int {
    None = 0x00,
    Translate = 0x01,
    Rotate = 0x02,
    Scale = 0x04
};

class ArcballController {
public:
    // Public methods
    ArcballController(QWidget* parent) 
        : parent_(parent) {
    }

    void initModelView(const QMatrix4x4& mMat, const QMatrix4x4& vMat) {
        rotMat_  = mMat;
        lookMat_ = vMat;
        update();
    }
    
    void update() {
        switch (mode_) {
        case ArcballMode::Translate:
            updateTranslate();
            break;
    
        case ArcballMode::Rotate:
            updateRotate();
            break;
        
        case ArcballMode::Scale:
            updateScale();
            break;

        default:
            break;
        }
    
        modelMat_ = rotMat_;
        modelMat_.scale(1.0 - scroll_ * 0.1);
    
        viewMat_ = lookMat_;
        viewMat_.translate(translate_);    
    }
    
    inline QMatrix4x4 modelMat() const { return modelMat_; }
    inline QMatrix4x4 viewMat() const { return viewMat_; }
    inline QMatrix4x4 modelViewMat() const { return viewMat_ * modelMat_; }
    
    inline double scroll() const { return scroll_; }
    
    inline void setMode(ArcballMode mode) { mode_ = mode; }
    inline void setOldPoint(const QPoint& pos) { oldPoint_ = pos; }
    inline void setNewPoint(const QPoint& pos) { newPoint_ = pos; }
    inline void setScroll(double scroll) { scroll_ = scroll; }
    
private:
    // Private methods
    QVector3D getVector(int x, int y) const {
        QVector3D pt( 2.0 * x / parent_->width()  - 1.0,
                     -2.0 * y / parent_->height() + 1.0,
                      0.0);

        const double xySquared = pt.x() * pt.x() + pt.y() * pt.y();
        if (xySquared) {
            pt.setZ(std::sqrt(1.0 - xySquared));
        } else {
            pt.normalized();
        }
    
        return pt;    
    }

    void updateTranslate() {
        const QVector4D u(1.0f, 0.0f, 0.0f, 0.0f);
        const QVector4D v(0.0f, 1.0f, 0.0f, 0.0f);
        const QMatrix4x4 camera2objMat = lookMat_.inverted();
    
        const QVector3D objspaceU = (camera2objMat * u).toVector3D().normalized();
        const QVector3D objspaceV = (camera2objMat * v).toVector3D().normalized();

        const double dx = 10.0 * (newPoint_.x() - oldPoint_.x()) / parent_->width();
        const double dy = 10.0 * (newPoint_.y() - oldPoint_.y()) / parent_->height();

        translate_ += (objspaceU * dx - objspaceV * dy);    
    }

    void updateRotate() {
        static const double Pi = 4.0 * std::atan(1.0);

        const QVector3D u = getVector(newPoint_.x(), newPoint_.y());
        const QVector3D v = getVector(oldPoint_.x(), oldPoint_.y());
    
        const double angle = std::acos(std::min(1.0f, QVector3D::dotProduct(u, v)));
    
        const QVector3D rotAxis = QVector3D::crossProduct(v, u);
        const QMatrix4x4 camera2objMat = rotMat_.inverted();
    
        const QVector3D objSpaceRotAxis = camera2objMat * rotAxis;
    
        QMatrix4x4 temp;
        double angleByDegree = 180.0 * angle / Pi;
        temp.rotate(4.0 * angleByDegree, objSpaceRotAxis);
    
        rotMat_ = rotMat_ * temp;    
    }

    void updateScale() {
        const double dy = 20.0 * (newPoint_.y() - oldPoint_.y()) / parent_->height();
        scroll_ += dy;    
    }
    
    // Private parameters
    QWidget* parent_;
    QMatrix4x4 modelMat_;
    QMatrix4x4 viewMat_;
    double scroll_ = 0.0;
    QPoint oldPoint_ = QPoint(0, 0);
    QPoint newPoint_ = QPoint(0, 0);
    
    ArcballMode mode_ = ArcballMode::None;
    QVector3D translate_ = QVector3D(0.0f, 0.0f, 0.0f);
    QMatrix4x4 lookMat_;
    QMatrix4x4 rotMat_;
};

#endif  // _ARCBALL_CONTROLLER_H_
