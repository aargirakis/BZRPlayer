#ifndef BZR2_ROTATINGOBJECT_H
#define BZR2_ROTATINGOBJECT_H

#include <QPainter>
#include "Point2D.h"
#include "Point3D.h"

class RotatingObject {
public:
    enum class ShadeMode {
        None,
        Flat,
        Lambert,
        LambertSpecular
    };
    enum class NearPolicy { ClampInFront, HideWhenTooClose };

    RotatingObject();

    void initCube(double size);
    void initPyramid(double size);
    void initSphere(int rings = 10, int segments = 10, double radius = 40.0);
    void initModels();

    // settings
    void setSize(const int size) { model_size = size; initModels();}
    int size() const { return model_size; }
    void setEnabled(const bool on) { m_enabled = on; }
    bool enabled() const { return m_enabled; }
    void setFocalLength(const double f) { m_focalLen = std::max(1.0, f); }
    int focalLength() const { return static_cast<int>(m_focalLen + 0.5); }
    void setOrbitSize(const int v) { m_orbitRadius = v;}
    int orbitSize() const { return m_orbitRadius; }
    void setOrbitSpeed(const int v) {m_orbitSpeed = static_cast<double>(v)/100;}
    int orbitSpeed() const { return m_orbitSpeed*100; }
    void setOrbitEnabled(const bool on) { m_orbitEnabled = on; }
    bool orbitEnabled() const { return m_orbitEnabled; }
    void setFillColor(const QColor c) { m_fillColor = c; }
    QColor fillColor() const { return m_fillColor; }
    void setWireColor(const QColor c) { m_wireColor = c; }
    QColor wireColor() const { return m_wireColor; }
    void setWireframeEnabled(const bool on) { m_wireframeEnabled = on; }
    bool wireframeEnabled() const { return m_wireframeEnabled; }

    void setMaterial(const QString &m) {
        m_shaderString = m;
        if (m == "flat") { m_shade = ShadeMode::Flat; } else if (m == "lambert") { m_shade = ShadeMode::Lambert; } else
            if (m == "blinn") { m_shade = ShadeMode::LambertSpecular; } else if (m == "none") {
                m_shade = ShadeMode::None;
            }
    }

    QString material() const { return m_shaderString; }
    void setModel(const QString &model) { m_model = model;initModels();}
    QString model() const { return m_model; }
    // end settings

    void update(double ampScaledRotation);
    void paint(QPainter* p, int canvasW, int canvasH) const;

    void setLightDir(double x, double y, double z);

private:
    struct OrbitXform { double tx, ty, tz; bool visible; };

    // settings
    bool m_enabled;
    bool m_orbitEnabled;
    double m_orbitSpeed;
    double m_orbitRadius;
    int model_size;
    QColor m_fillColor;
    QColor m_wireColor;
    bool m_wireframeEnabled;
    ShadeMode m_shade;
    double m_focalLen;
    QString m_model;
    // end settings

    std::vector<Point3D>  m_vertices;
    std::vector<std::array<int,4>> m_quads; // quad indices into m_vertices
    mutable std::vector<uint8_t> m_faceFront; // 0/1 per face, persists across frames
    Point3D m_rotation {0,0,0};
    Point3D m_rotationSpeed {0.05, 0.05, 0.05};
    double m_orbitAngle = 0.0;
    double m_orbitCenterZ = 120.0;
    double m_orbitY = 0.0;
    double m_idleCenterZ = 140.0;
    double m_idleY = 0.0;
    QString m_shaderString;
    NearPolicy m_nearPolicy = NearPolicy::ClampInFront;
    double m_nearMargin = 12.0;
    double m_boundRadius = 80.0;
    int m_wireWidth = 1;
    Point3D m_lightDir {0.5, 0.8, 1.0};
    double m_specularPower = 4.0;
    double m_specularStrength = 0.40;

    static inline void normalize(Point3D& v);
    OrbitXform computeTransform() const;

    void project(const std::vector<Point3D>& verts,
                 const Point3D& rot, double tx, double ty, double tz,
                 std::vector<Point2D>& out2D, std::vector<Point3D>& outCam) const;
};

#endif // BZR2_ROTATINGOBJECT_H
