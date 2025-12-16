#include "BitmapFont.hpp"

#include <QPixmap>

BitmapFont::BitmapFont(const QString &filePath)
{
    load(filePath);
}

bool BitmapFont::load(const QString &filePath)
{
    mFilePath = filePath;
    return reload();
}

bool BitmapFont::reload()
{
    mSourceImage = QImage(mFilePath);
    if (mSourceImage.isNull())
        return false;

    constexpr QRgb separatorColor = qRgb(255, 255, 255);
    mCharWidth.clear();

    const int bottomY = mSourceImage.height() - 1;
    int currentWidth = 0;

    for (int x = 0; x < mSourceImage.width(); ++x)
    {
        const QRgb px = mSourceImage.pixel(x, bottomY);
        if (px == separatorColor)
        {
            mCharWidth.append(currentWidth);
            currentWidth = 0;
        }
        else
        {
            ++currentWidth;
        }
    }

    return true;
}

QPixmap BitmapFont::getChar(int index) const
{
    if (mSourceImage.isNull())
        return {};

    if (index < 0 || index >= mCharWidth.count())
        return {};

    int fromX = index;
    for (int i = 0; i < index; ++i)
        fromX += mCharWidth.at(i);

    const int width = mCharWidth.at(index);
    const QImage ch = mSourceImage.copy(fromX, 0, width, mSourceImage.height());
    return QPixmap::fromImage(ch);
}

QString BitmapFont::filePath() const
{
    return mFilePath;
}

int BitmapFont::height() const
{
    return mSourceImage.height();
}

bool BitmapFont::isValid() const
{
    return !mSourceImage.isNull();
}
