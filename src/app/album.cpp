#include "album.h"
#include <QHBoxLayout>
#include <QApplication>
#include <QPainter>

Album::Album(QString id, QWidget* parent): QLabel(parent)
{
    this->id = id;

    this->setGeometry(0, 0, 175, 230);
    this->labelAlbum = new QLabel(this);
    this->labelAlbum->setGeometry(0, 0, 124, 124);
    this->labelAlbum->setScaledContents(true);

    this->labelText = new QLabel(this);
    this->labelText->setAlignment(Qt::AlignLeft);

    this->playButton = new QPushButton(this);
    connect(playButton, &QPushButton::clicked, [this]() { emit clickedAddAlbum(this->path); });

    int size = 175;

    this->installEventFilter(playButton);

    playButton->setVisible(false);
    playButton->setText("Load");

    this->playButton->setGeometry(0, 0, 125, 125);
    this->playButton->setStyleSheet("background-color:rgba(0,0,0,210);font-weight:bold");

    this->labelText->setGeometry(2, static_cast<int>(size - 50), static_cast<int>(size), 60);
}

void Album::putPixmap(const QString& path)
{
    this->labelText->setText(
        "<span style=\"font-size:9pt;color:#fff;font-weight:bold;\">" + title +
        "</span><br><span style=\" font-size:9pt;color:#999;\"></span>");
    this->labelText->setWordWrap(true);
    this->labelText->setTextFormat(Qt::RichText);
    QPixmap pix(path);
    labelAlbum->setPixmap(pix);
}

void Album::enterEvent(QEvent* event)
{
    event->accept();

    playButton->setVisible(true);
}

void Album::leaveEvent(QEvent* event)
{
    event->accept();
    playButton->setVisible(false);
}
