#pragma once

#include <QImage>
#include <QList>
#include <QString>

class BitmapFont
{
public:
    explicit BitmapFont(const QString &filePath);

    bool load(const QString &filePath);
    bool reload();

    QPixmap getChar(int index) const;

    QString filePath() const;
    int height() const;
    bool isValid() const;

private:
    QImage mSourceImage;
    QString mFilePath;
    QList<quint8> mCharWidth;
};
