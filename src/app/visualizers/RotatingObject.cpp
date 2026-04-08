#include "RotatingObject.h"

static double invSqrt(const double s) { return 1.0 / std::sqrt(s); }

RotatingObject::RotatingObject() {
    m_enabled = true;
    setLightDir(0.8, 0.8, 1.0);
    m_fillColor = QColor(0, 200, 0);
    m_wireColor = Qt::white;
    m_wireWidth = 1;
    m_wireframeEnabled = true;
    m_shade = ShadeMode::LambertSpecular;
    m_focalLen = 140.0;
    m_rotationSpeed = Point3D(0.05, 0.05, 0.05);
    m_orbitEnabled = false;
    m_nearPolicy = NearPolicy::ClampInFront;
    m_nearMargin = 12.0;

    initCube(40.0);
}

void RotatingObject::setLightDir(const double x, const double y, const double z) {
    m_lightDir = Point3D(x, y, z);
    normalize(m_lightDir);
}

void RotatingObject::initCube(const double size) {
    m_vertices.clear();
    m_quads.clear();

    const double s = size;

    // 8 corners of a cube (centered at origin)
    m_vertices.reserve(8);
    m_vertices.push_back(Point3D(-s, -s, s)); // 0
    m_vertices.push_back(Point3D(s, -s, s)); // 1
    m_vertices.push_back(Point3D(-s, s, s)); // 2
    m_vertices.push_back(Point3D(s, s, s)); // 3
    m_vertices.push_back(Point3D(-s, s, -s)); // 4
    m_vertices.push_back(Point3D(s, s, -s)); // 5
    m_vertices.push_back(Point3D(-s, -s, -s)); // 6
    m_vertices.push_back(Point3D(s, -s, -s)); // 7

    // 6 quads (consistent winding)
    m_quads = {
        {0, 1, 3, 2}, // front (z = +s)
        {2, 3, 5, 4}, // right
        {4, 5, 7, 6}, // back (z = -s)
        {6, 7, 1, 0}, // left
        {1, 7, 5, 3}, // top
        {6, 0, 2, 4} // bottom
    };

    m_boundRadius = size * 1.8;

    // keep hysteresis state sized to face count
    m_faceFront.assign(m_quads.size(), 1);
}

void RotatingObject::initModels() {
    if (m_model == "cube") { initCube(model_size); }

    if (m_model == "sphere") { initSphere(20, 40, model_size); }

    if (m_model == "pyramid") { initPyramid(model_size); }
}

void RotatingObject::initPyramid(const double size) {
    m_vertices.clear();
    m_quads.clear();

    const double s = size;
    // base z = -s, apex z = +s
    m_vertices.reserve(5);
    m_vertices.push_back(Point3D(-s, -s, -s)); // v0
    m_vertices.push_back(Point3D(s, -s, -s)); // v1
    m_vertices.push_back(Point3D(s, s, -s)); // v2
    m_vertices.push_back(Point3D(-s, s, -s)); // v3
    m_vertices.push_back(Point3D(0, 0, s)); // v4 apex

    // base CW so normal ~ -Z (toward camera)
    m_quads.push_back({0, 3, 2, 1});
    // sides (triangles padded to quads by repeating apex)
    m_quads.push_back({0, 1, 4, 4});
    m_quads.push_back({1, 2, 4, 4});
    m_quads.push_back({2, 3, 4, 4});
    m_quads.push_back({3, 0, 4, 4});

    m_boundRadius = size * 1.8;
    m_faceFront.assign(m_quads.size(), 1);
}

void RotatingObject::initSphere(int rings, int segments, const double radius) {
    // sanity
    if (rings < 2) rings = 2;

    if (segments < 3) segments = 3;

    m_vertices.clear();
    m_quads.clear();

    // build interior rings (avoid exact poles: half-step in theta)
    auto idx = [segments](const int i, const int j) {
        return i * segments + j % segments;
    };

    // interior latitude bands
    for (int i = 0; i <= rings; ++i) {
        constexpr double PI = 3.14159265358979323846;
        const double theta = (i + 0.5) / (rings + 1) * PI; // (0, PI)
        const double sinT = std::sin(theta);
        const double cosT = std::cos(theta);

        for (int j = 0; j < segments; ++j) {
            const double phi = static_cast<double>(j) / segments * (2.0 * PI);
            const double sinP = std::sin(phi);
            const double cosP = std::cos(phi);

            const double x = radius * sinT * cosP;
            const double y = radius * cosT;
            const double z = radius * sinT * sinP;
            m_vertices.push_back(Point3D(x, y, z));
        }
    }

    // quads between interior rings
    for (int i = 0; i < rings; ++i) {
        for (int j = 0; j < segments; ++j) {
            const int a = idx(i, j);
            const int b = idx(i, j + 1);
            const int c = idx(i + 1, j + 1);
            const int d = idx(i + 1, j);
            m_quads.push_back({a, b, c, d});
        }
    }

    // add poles and cap with triangle fans
    const int north = static_cast<int>(m_vertices.size());
    m_vertices.push_back(Point3D(0, +radius, 0));
    const int south = static_cast<int>(m_vertices.size());
    m_vertices.push_back(Point3D(0, -radius, 0));

    // top cap (around ring 0)
    for (int j = 0; j < segments; ++j) {
        const int a = idx(0, j);
        const int b = idx(0, j + 1);
        // padded triangle: (north, b, a)
        m_quads.push_back({north, b, a, a});
    }

    // bottom cap (around ring 'rings').
    for (int j = 0; j < segments; ++j) {
        const int a = idx(rings, j);
        const int b = idx(rings, j + 1);
        // padded triangle: (south, a, b)
        m_quads.push_back({south, a, b, b});
    }

    // bounds & cull hysteresis slots
    m_boundRadius = radius * 1.05;
    m_faceFront.assign(m_quads.size(), 1);
}

void RotatingObject::update(const double ampScaledRotation) {
    if (!m_enabled) return;

    // scale rotation speed by amplitude (caller decides amp)
    const double k = std::clamp(ampScaledRotation, 0.0, 1000.0);
    m_rotation.x -= m_rotationSpeed.x * k * 3;
    m_rotation.y -= m_rotationSpeed.y * k * 4;
    m_rotation.z -= m_rotationSpeed.z * k * 5;

    if (m_orbitEnabled) {
        m_orbitAngle += m_orbitSpeed * k * 2.0;
    }
}

void RotatingObject::paint(QPainter *p, int canvasW, int canvasH) const {
    if (!m_enabled || m_vertices.empty() || m_quads.empty()) return;

    const OrbitXform xf = computeTransform();
    if (!xf.visible) return;

    // project + also keep camera-space rotated verts
    std::vector<Point2D> screenPts;
    std::vector<Point3D> camPts;
    project(m_vertices, m_rotation, xf.tx, xf.ty, xf.tz, screenPts, camPts);

    const int cx = canvasW / 2;
    const int cy = canvasH / 2;

    // unified projector: use floating-point to avoid rounding mismatches
    auto projF = [&](const Point3D &P) -> QPointF {
        const double s = m_focalLen / (m_focalLen + P.z);
        return QPointF(cx + P.x * s, cy + P.y * s);
    };

    // depth sort faces
    struct FaceItem {
        int idx;
        double depth;
    };
    std::vector<FaceItem> items;
    items.reserve(m_quads.size());


    for (int i = 0; i < static_cast<int>(m_quads.size()); ++i) {
        const auto &f = m_quads[i];
        double zAvg = (camPts[f[0]].z + camPts[f[1]].z + camPts[f[2]].z + camPts[f[3]].z) * 0.25;
        items.push_back({i, zAvg});
    }

    std::sort(items.begin(), items.end(),
              [](const FaceItem &a, const FaceItem &b) { return a.depth > b.depth; });

    p->save();
    p->setRenderHint(QPainter::Antialiasing, false);

    // draw filled faces
    p->setPen(Qt::NoPen);

    for (const FaceItem &it: items) {
        const auto &f = m_quads[it.idx];

        // camera-space verts
        const Point3D &A = camPts[f[0]];
        const Point3D &B = camPts[f[1]];
        const Point3D &C = camPts[f[2]];
        const Point3D &D = camPts[f[3]];

        // 3d normal
        Point3D u(B.x - A.x, B.y - A.y, B.z - A.z);
        Point3D v(C.x - A.x, C.y - A.y, C.z - A.z);
        Point3D n(u.y * v.z - u.z * v.y, u.z * v.x - u.x * v.z, u.x * v.y - u.y * v.x);
        normalize(n);

        // view from centroid
        Point3D cent((A.x + B.x + C.x + D.x) * 0.25,
                     (A.y + B.y + C.y + D.y) * 0.25,
                     (A.z + B.z + C.z + D.z) * 0.25);
        Point3D view(-cent.x, -cent.y, -cent.z);
        normalize(view);

        // conservative 2d orientation (double precision), using unified projector
        const QPointF q0 = projF(A);
        const QPointF q1 = projF(B);
        const QPointF q2 = projF(C);
        const double crossZ = (q1.x() - q0.x()) * (q2.y() - q0.y()) - (q1.y() - q0.y()) * (q2.x() - q0.x());
        const bool back2D = crossZ >= 0.0;

        // 3d facing (sign depends on winding
        const double ndotv = -(n.x * view.x + n.y * view.y + n.z * view.z);

        // hysteresis band around 0 to avoid pops (tune 0.015–0.05)
        constexpr double HYST = 0.03;
        bool frontNow = m_faceFront[it.idx]; // previous state

        if (frontNow) {
            // only flip to back if we're clearly back-facing AND 2d agrees
            if (ndotv > +HYST && back2D) frontNow = false;
        } else {
            // only flip to front if we're clearly front-facing OR 2d disagrees
            if (ndotv < -HYST || !back2D) frontNow = true;
        }
        m_faceFront[it.idx] = frontNow;

        if (!frontNow) continue; // culled

        QColor face = m_fillColor;

        if (!(m_shade == ShadeMode::Flat || m_shade == ShadeMode::None)) {
            const Point3D &Ld = m_lightDir;
            // vector from surface to light (opposite of travel dir)
            Point3D toLight(-Ld.x, -Ld.y, -Ld.z);
            normalize(toLight);

            double ndotl_raw = n.x * toLight.x + n.y * toLight.y + n.z * toLight.z; // [-1..1]
            // half-lambert in [0..1]
            double diffuse = std::clamp(0.5 * ndotl_raw + 0.5, 0.0, 1.0);

            constexpr double ambient = 0.20; // tweak to taste
            double intensity = ambient + (1.0 - ambient) * diffuse;

            double specAdd = 0.0;

            if (m_shade == ShadeMode::LambertSpecular && m_specularStrength > 0.0) {
                // view vector from centroid to camera (camera at origin)
                Point3D cent((A.x + B.x + C.x + D.x) * 0.25,
                             (A.y + B.y + C.y + D.y) * 0.25,
                             (A.z + B.z + C.z + D.z) * 0.25);
                Point3D V(-cent.x, -cent.y, -cent.z);
                normalize(V);

                // phong reflection: R = reflect(-toLight, n) = 2(n·toLight)n - toLight
                double ndotl = std::max(0.0, n.x * toLight.x + n.y * toLight.y + n.z * toLight.z);
                Point3D R(2.0 * ndotl * n.x - toLight.x,
                          2.0 * ndotl * n.y - toLight.y,
                          2.0 * ndotl * n.z - toLight.z);
                normalize(R);

                double rdotov = std::max(0.0, R.x * V.x + R.y * V.y + R.z * V.z);
                specAdd = std::pow(rdotov, m_specularPower) * m_specularStrength;
            }

            auto clamp255 = [](const int v) { return std::min(255, std::max(0, v)); };
            int r = clamp255(static_cast<int>(face.red() * intensity + 255.0 * specAdd));
            int g = clamp255(static_cast<int>(face.green() * intensity + 255.0 * specAdd));
            int b = clamp255(static_cast<int>(face.blue() * intensity + 255.0 * specAdd));
            face.setRgb(r, g, b);
        }

        if (m_shade != ShadeMode::None) {
            const QPointF q3 = projF(D);
            const QPointF polyF[4] = {q0, q1, q2, q3};

            p->setPen(Qt::NoPen);
            p->setBrush(face);
            p->drawConvexPolygon(polyF, 4);
        }
    }

    if (m_wireframeEnabled) {
        QPen pen(m_wireColor);
        pen.setWidthF(m_wireWidth); // fractional width avoids gaps
        pen.setCosmetic(true);
        pen.setJoinStyle(Qt::MiterJoin);
        pen.setCapStyle(Qt::SquareCap);
        pen.setMiterLimit(4.0);

        p->setBrush(Qt::NoBrush);
        p->setPen(pen);

        for (const auto &f: m_quads) {
            const QPointF w0 = projF(camPts[f[0]]);
            const QPointF w1 = projF(camPts[f[1]]);
            const QPointF w2 = projF(camPts[f[2]]);
            const QPointF w3 = projF(camPts[f[3]]);
            const QPointF polyW[4] = {w0, w1, w2, w3};
            p->drawPolygon(polyW, 4);
        }

        p->setRenderHint(QPainter::Antialiasing, false);
    }

    p->restore();
}

inline void RotatingObject::normalize(Point3D &v) {
    if (const double len2 = v.x * v.x + v.y * v.y + v.z * v.z;
        len2 > 0.0) {
        const double inv = invSqrt(len2);
        v.x *= inv;
        v.y *= inv;
        v.z *= inv;
    }
}

RotatingObject::OrbitXform RotatingObject::computeTransform() const {
    OrbitXform xf{0, 0, 0, true};

    double tz, ty, tx;

    if (!m_orbitEnabled) {
        tx = 0.0;
        ty = m_idleY;
        tz = m_idleCenterZ;
    } else {
        tx = m_orbitRadius * std::cos(m_orbitAngle);
        ty = m_orbitY;
        tz = m_orbitRadius * std::sin(m_orbitAngle) + m_orbitCenterZ;
    }

    // near-plane safety using worst-case vertex tz - bound
    const double minZ = tz - m_boundRadius;

    if (const double denomMin = m_focalLen + minZ;
        denomMin < m_nearMargin) {
        if (m_nearPolicy == NearPolicy::HideWhenTooClose) {
            xf.visible = false;
        } else {
            tz = m_nearMargin - m_focalLen + m_boundRadius;
        }
    }

    xf.tx = tx;
    xf.ty = ty;
    xf.tz = tz;
    return xf;
}

void RotatingObject::project(const std::vector<Point3D> &verts,
                             const Point3D &rot, const double tx, const double ty, const double tz,
                             std::vector<Point2D> &out2D, std::vector<Point3D> &outCam) const {
    out2D.clear();
    outCam.clear();
    out2D.reserve(verts.size());
    outCam.reserve(verts.size());

    // precompute sines/cosines once
    const double sx = std::sin(rot.x), cx = std::cos(rot.x);
    const double sy = std::sin(rot.y), cy = std::cos(rot.y);
    const double sz = std::sin(rot.z), cz = std::cos(rot.z);

    for (size_t i = 0; i < verts.size(); ++i) {
        const double x = verts[i].x;
        const double y = verts[i].y;
        const double z = verts[i].z;

        // rotate (same pipeline you used)
        const double xy = cx * y - sx * z;
        const double xz = sx * y + cx * z;
        double yz = cy * xz - sy * x;
        const double yx = sy * xz + cy * x;
        double zx = cz * yx - sz * xy;
        double zy = sz * yx + cz * xy;

        // translate in camera space
        zx += tx;
        zy += ty;
        yz += tz;

        // store camera-space for normals/depth
        outCam.push_back(Point3D(zx, zy, yz));

        // perspective
        const double scale = m_focalLen / (m_focalLen + yz);
        out2D.push_back(Point2D(
            static_cast<int>(zx * scale),
            static_cast<int>(zy * scale),
            static_cast<int>(-yz),
            scale
        ));
    }
}
