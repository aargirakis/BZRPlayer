#ifndef BZR2_STARFIELD_H
#define BZR2_STARFIELD_H

#include <QPen>

class QPainter;

class StarField {
public:
    enum class Mode : uint8_t { Right, Left, Up, Down, In, Out };

    StarField();

    void setCanvasSize(int w, int h);
    void ensureInited();

    // settings
    void setEnabled(const bool e) { m_enabled = e; }
    bool enabled() const { return m_enabled; }
    void setCount(int n);
    int count() const { return m_count; }
    void setSpeed(const int s) { m_starSpeed = std::max(0, s); }
    int speed() const { return m_starSpeed; }
    void setModeFromString(const QString& dir);
    QString modeToString() const;
    // end settings

    void paint(QPainter* painter);

private:
    struct Star {
        float x{}, y{}, z{};
        float speed{};
        int b;
        float jitter;
        float lumBase;
    };

    void buildPensAndBuckets();
    void reinitAllStars();
    void reinitLinearStar(int i);
    void reinitZoomStar(int i, bool farAway);

    // settings
    bool m_enabled;
    int m_count;
    int m_starSpeed;
    Mode m_mode;
    // end settings

    bool m_inited = false;
    static constexpr int kBuckets = 16;
    int m_w = 0, m_h = 0;
    QVector<Star> m_stars;
    QPen m_pens[kBuckets];
    QVector<QPoint> m_buckets[kBuckets];
    float m_fovFactor = 0.5f;
};


#endif //BZR2_STARFIELD_H
