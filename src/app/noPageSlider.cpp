#include <QKeyEvent>
#include <QStyleOptionSlider>
#include "noPageSlider.h"

NoPageSlider::NoPageSlider(QWidget* parent)
    : QSlider(parent)
{
    defValue = 0;
    snapDefault = false;
    snapDistance = 10; //this is currently in "values", should be in mouse distance
}

void NoPageSlider::keyPressEvent(QKeyEvent* e)
{
    e->ignore();
    return;
}

void NoPageSlider::mouseDoubleClickEvent(QMouseEvent* e)
{
    if (e->button() == Qt::RightButton)
    {
        setSliderPosition(defaultValue());
    }
}

void NoPageSlider::mousePressEvent(QMouseEvent* e)
{
    if (maximum() == minimum() ||
        (e->button() != Qt::LeftButton) ||
        (e->buttons() ^ e->button()))
    {
        e->ignore();
        return;
    }
    e->accept();
    int realValue = valueFromPoint(e->pos());
    if (snapToDefault())
    {
        if (abs(realValue - defaultValue()) < snapDistance)
        {
            realValue = defaultValue();
        }
    }
    setSliderPosition(realValue);
    setSliderDown(true);
}

void NoPageSlider::mouseReleaseEvent(QMouseEvent* e)
{
    if (e->button() != Qt::LeftButton)
    {
        e->ignore();
        return;
    }
    e->accept();
    int realValue = valueFromPoint(e->pos());
    if (snapToDefault())
    {
        if (abs(realValue - defaultValue()) < snapDistance)
        {
            realValue = defaultValue();
        }
    }
    setSliderPosition(realValue);
    setSliderDown(false);
}

void NoPageSlider::mouseMoveEvent(QMouseEvent* e)
{
    if (!hasTracking() || !(e->buttons() & Qt::LeftButton))
    {
        e->ignore();
        return;
    }
    e->accept();
    //d->doNotEmit = true;
    int realValue = valueFromPoint(e->pos());
    if (snapToDefault())
    {
        if (abs(realValue - defaultValue()) < snapDistance)
        {
            realValue = defaultValue();
        }
    }
    setSliderPosition(realValue);

    emit sliderMoved(value());
    //d->doNotEmit = false;
}

int NoPageSlider::valueFromPoint(const QPoint& p)
{
    QStyleOptionSlider opt = getStyleOption();
    QRect gr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, this);
    QRect sr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);

    int sliderMin, sliderMax, sliderLength, point;

    if (orientation() == Qt::Horizontal)
    {
        sliderLength = sr.width();
        sliderMin = gr.x() + sliderLength / 2;
        sliderMax = gr.right() - sliderLength / 2 + 1;
        point = p.x();
    }
    else
    {
        sliderLength = sr.height();
        sliderMin = gr.y() + sliderLength / 2;
        sliderMax = gr.bottom() - sliderLength / 2 + 1;
        point = p.y();
    }
    return QStyle::sliderValueFromPosition(minimum(), maximum(), point - sliderMin,
                                           sliderMax - sliderMin, opt.upsideDown);
}

QStyleOptionSlider NoPageSlider::getStyleOption() const
{
    QStyleOptionSlider opt;
    opt.initFrom(this);
    opt.subControls = QStyle::SC_None;
    opt.activeSubControls = QStyle::SC_None;
    opt.orientation = orientation();
    opt.maximum = maximum();
    opt.minimum = minimum();
    opt.tickPosition = (QSlider::TickPosition)tickPosition();
    opt.tickInterval = tickInterval();
    opt.upsideDown = (orientation() == Qt::Horizontal)
                         ? (invertedAppearance() != (opt.direction == Qt::RightToLeft))
                         : (!invertedAppearance());
    opt.direction = Qt::LeftToRight; // we use the upsideDown option instead
    opt.sliderPosition = pos().x();
    opt.sliderValue = value();
    opt.singleStep = singleStep();
    opt.pageStep = pageStep();
    if (orientation() == Qt::Horizontal)
        opt.state |= QStyle::State_Horizontal;
    return opt;
}
