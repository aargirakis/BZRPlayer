#ifndef ALBUM_H
#define ALBUM_H

#include <QLabel>
#include <QPushButton>
#include <QEvent>
#include <QMouseEvent>
class Album : public QLabel
{
    Q_OBJECT
public:
    Album(QString,QWidget *parent = nullptr);
    QLabel *labelAlbum;
    QLabel *labelText;
    void putPixmap(const QString&);
    QString artwork;
    QString title;
    QString path;
    QString id;
private:
    QPushButton *playButton;
protected:
    virtual void enterEvent(QEvent *event);
    virtual void leaveEvent(QEvent *event);


signals:
 void clickedAddAlbum(QString);
};

#endif // ALBUM_H
