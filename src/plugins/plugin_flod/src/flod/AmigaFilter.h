#ifndef AMIGAFILTER_H
#define AMIGAFILTER_H

class Sample;

class AmigaFilter
{
public:
    AmigaFilter();

    enum
    {
        AUTOMATIC = 0,
        FORCE_ON = 1,
        FORCE_OFF = -1
    };

    int active;
    int filter;
    void initialize();
    void process(Sample* sample);
    void setModel(int model);
    void setFilter(int filterVal);

private:
    static constexpr double FL = 0.5213345843532200;
    static constexpr double P0 = 0.4860348337215757;
    static constexpr double P1 = 0.9314955486749749;
    double l0;
    double l1;
    double l2;
    double l3;
    double l4;
    double r0;
    double r1;
    double r2;
    double r3;
    double r4;
    int model;
};

#endif // AMIGAFILTER_H
