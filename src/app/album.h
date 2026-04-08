#ifndef ALBUM_H
#define ALBUM_H

#include <QLabel>
#include <QPushButton>

class Album : public QLabel {
    Q_OBJECT

public:
    Album(const QString &, QWidget *parent = nullptr);

    QLabel *labelAlbum;
    QLabel *labelText;

    void putPixmap(const QString &) const;

    QString artwork;
    QString title;
    QString path;
    QString id;

private:
    QPushButton *playButton;

protected:
    void enterEvent(QEnterEvent *event) override;

    void leaveEvent(QEvent *event) override;


signals:
    void clickedAddAlbum(QString);
};

#endif // ALBUM_H
