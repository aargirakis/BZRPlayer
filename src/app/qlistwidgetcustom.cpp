#include "qlistwidgetcustom.h"
#include "qevent.h"
#include <iostream>

QListWidgetCustom::QListWidgetCustom(QWidget* parent)
    : QListWidget(parent)
{
}

void QListWidgetCustom::dragMoveEvent(QDragMoveEvent* event)
{
    std::cout << "drag!" << event->source();
}
