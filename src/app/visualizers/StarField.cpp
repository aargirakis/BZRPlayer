#include "StarField.h"
#include <QPainter>
#include <QRandomGenerator>
#include <algorithm>

StarField::StarField()
        : m_w(1), m_h(1)
        {}

void StarField::setCanvasSize(int w, int h) {
    int W = std::max(1, w);
    int H = std::max(1, h);
    if (W == m_w && H == m_h) return;

    m_w = W; m_h = H;

    // If already initialized, respray current stars into the new bounds
    for (int i = 0; i < (int)m_stars.size(); ++i) {
        if (m_mode == Mode::In || m_mode == Mode::Out) {
            reinitZoomStar(i, /*farAway=*/true);
        } else {
            reinitLinearStar(i);
        }
    }
}

void StarField::setCount(int n) {
    n = std::max(1, n);
    if (m_count == n) return;
    m_count = n;
    m_stars.resize(m_count);
    reinitAllStars();
}

void StarField::setModeFromString(const QString& dir) {
    const QString d = dir.toLower();
    if      (d == "right") m_mode = Mode::Right;
    else if (d == "left")  m_mode = Mode::Left;
    else if (d == "up")    m_mode = Mode::Up;
    else if (d == "down")  m_mode = Mode::Down;
    else if (d == "in")    m_mode = Mode::In;
    else if (d == "out")   m_mode = Mode::Out;
    else                   m_mode = Mode::Right;
}

QString StarField::modeToString() const {
    switch (m_mode) {
        case Mode::Right: return "right";
        case Mode::Left:  return "left";
        case Mode::Up:    return "up";
        case Mode::Down:  return "down";
        case Mode::In:    return "in";
        case Mode::Out:   return "out";
    }
    return "right";
}

void StarField::ensureInited() {
    if (m_inited) return;
    buildPensAndBuckets();
    if (m_stars.size() != m_count) m_stars.resize(m_count);
    reinitAllStars();
    m_inited = true;
}

void StarField::buildPensAndBuckets() {
    const int alphaMin = 8;
    const int alphaMax = 255;
    for (int i = 0; i < kBuckets; ++i) {
        float t = float(i) / float(kBuckets - 1);
        t = std::pow(t, 1.2f);
        const int alpha = alphaMin + int(std::round(t * (alphaMax - alphaMin)));
        m_pens[i] = QPen(QColor(255, 255, 255, alpha));
        m_pens[i].setCosmetic(true);
        m_buckets[i].clear();
        m_buckets[i].reserve(m_count / 4 + 8);
    }
}

void StarField::reinitAllStars() {
    for (int i = 0; i < m_count; ++i) {
        if (m_mode == Mode::In)  reinitZoomStar(i, true);
        else if (m_mode == Mode::Out) reinitZoomStar(i, false);
        else reinitLinearStar(i);
    }
}

void StarField::reinitLinearStar(int i) {
    Star& s = m_stars[i];
    auto& rng = *QRandomGenerator::global();

    const int W = std::max(1, m_w);
    const int H = std::max(1, m_h);

    s.x = float(rng.generateDouble() * W * 2.0 - W);
    s.y = float(rng.generateDouble() * H * 2.0 - H);
    s.z = float(rng.generateDouble() * 100.0 + 1.0);

    // Base luminance for linear modes (uniform spread)
    const float minLumL = 0.12f;
    const float maxLumL = 0.96f;
    s.lumBase = minLumL + float(rng.generateDouble()) * (maxLumL - minLumL);

    const float base = (m_starSpeed ? float(m_starSpeed) : 1.0f);
    s.speed = base * (0.4f + 0.8f * float(rng.generateDouble())); // >0

    const float J = 0.10f; // ±10% brightness variance
    const float randVal = float(rng.generateDouble()); // [0,1)
    s.jitter = 1.0f + J * (2.0f * randVal - 1.0f);     // 1 ± J
}

void StarField::reinitZoomStar(int i, bool farAway) {
    Star& s = m_stars[i];
    auto& rng = *QRandomGenerator::global();

    const float R = 35.0f;
    const float u = float(rng.generateDouble());
    const float v = float(rng.generateDouble());
    const float r = R * std::sqrt(u);
    const float theta = float(2.0 * M_PI) * v;
    s.x = r * std::cos(theta);
    s.y = r * std::sin(theta);

    s.z = farAway
          ? 60.0f + float(rng.generateDouble()) * 40.0f
          : 1.0f  + float(rng.generateDouble()) * 19.0f;

    const float base = (m_starSpeed ? float(m_starSpeed) : 1.0f);
    s.speed = base * (0.4f + 0.8f * float(rng.generateDouble()));

    // Add jitter at the end
    const float J = 0.10f;
    const float randVal = float(rng.generateDouble());
    s.jitter = 1.0f + J * (2.0f * randVal - 1.0f);
}

void StarField::paint(QPainter* p) {
    if (!m_enabled) return;
    ensureInited();

    for (int b = 0; b < kBuckets; ++b) m_buckets[b].clear();

    const int W = m_w, H = m_h;
    const int halfW = W >> 1, halfH = H >> 1;
    const float scale = m_fovFactor * float(std::min(W, H));

    for (int i = 0; i < m_count; ++i) {
        Star &s = m_stars[i];

        switch (m_mode) {
            case Mode::Right:
                s.x += s.speed;
                if (s.x > W) {
                    reinitLinearStar(i);
                    s.x = -float(QRandomGenerator::global()->bounded(W));
                }
                break;
            case Mode::Left:
                s.x -= s.speed;
                if (s.x < 0) {
                    reinitLinearStar(i);
                    s.x = W + float(QRandomGenerator::global()->bounded(W));
                }
                break;
            case Mode::Up:
                s.y -= s.speed;
                if (s.y < 0) {
                    reinitLinearStar(i);
                    s.y = H + float(QRandomGenerator::global()->bounded(H));
                }
                break;
            case Mode::Down:
                s.y += s.speed;
                if (s.y > H) {
                    reinitLinearStar(i);
                    s.y = -float(QRandomGenerator::global()->bounded(H));
                }
                break;
            case Mode::In:
            case Mode::Out: {

                const float dz = s.speed * 0.35f; // per-star depth speed
                if (m_mode == Mode::In) {
                    s.z -= dz;
                    if (s.z <= 1.0f) { reinitZoomStar(i, true); }
                } else { // Out
                    s.z += dz;
                    if (s.z >= 100.0f) { reinitZoomStar(i, false); }
                }

                const float invZ = 1.0f / s.z;
                const int sx = qRound(s.x * invZ * scale) + halfW;
                const int sy = qRound(s.y * invZ * scale) + halfH;

                if ((unsigned) sx < (unsigned) W && (unsigned) sy < (unsigned) H) {

                    const float zRef = 12.0f; // distance at which stars are fully bright
                    const float pwr  = 1.4f;

                    float lum = std::min(zRef / s.z, 1.0f);
                    lum = std::pow(lum, pwr);

                    lum *= s.jitter;
                    lum = std::clamp(lum, 0.02f, 0.98f);

                    const int b = std::clamp(int(lum * (kBuckets - 1) + 0.5f), 0, kBuckets - 1);
                    m_buckets[b].push_back(QPoint(sx, sy));
                }
                continue;
            }
        }

        if (m_mode == Mode::Left || m_mode == Mode::Right ||
            m_mode == Mode::Up   || m_mode == Mode::Down) {

            if ((unsigned)s.x < (unsigned)W && (unsigned)s.y < (unsigned)H) {

                const float base = (m_starSpeed ? float(m_starSpeed) : 1.0f);
                const float sMin = 0.4f * base;
                const float sMax = 1.2f * base;
                float frac = (s.speed - sMin) / (sMax - sMin);
                frac = std::clamp(frac, 0.0f, 1.0f);

                const float gamma = 0.9f;
                float lum = std::pow(frac, gamma);

                lum *= s.jitter;
                lum = std::clamp(lum, 0.06f, 0.98f);

                const int b = std::clamp(int(lum * (kBuckets - 1) + 0.5f), 0, kBuckets - 1);
                m_buckets[b].push_back(QPoint(int(s.x), int(s.y)));
            }
        }
    }
    p->setRenderHint(QPainter::Antialiasing, false);
    for (int b = 0; b < kBuckets; ++b) {
        if (m_buckets[b].isEmpty()) continue;
        p->setPen(m_pens[b]);
        p->drawPoints(m_buckets[b].constData(), m_buckets[b].size());
    }
}
