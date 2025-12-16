#include "MainWindow.hpp"
#include "ui_MainWindow.h"

#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <QGraphicsSimpleTextItem>
#include <QImage>

namespace {

QList<int> IgnoreChars = {
    ('~' + 1),
    ('~' + 2), // Shield icon
    ('~' + 3),
    158, // Lamp (tech) icon
    169,
    170,
    171,
    172,
};

} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , mUi(new Ui::MainWindow)
{
    mUi->setupUi(this);

    mUi->actionOpen->setShortcut(QKeySequence::Open);
    mUi->actionExit->setShortcut(QKeySequence::Quit);

    mScene = new QGraphicsScene(this);
    mUi->graphicsView->setScene(mScene);
}

MainWindow::~MainWindow()
{
    delete mUi;
}

void MainWindow::browseFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open a bitmap font"),
                                                    "/home/user",
                                                    tr("Bitmap font Files (*.png *.bmp)"));

    openFile(fileName);
}

class BitmapFont
{
public:
    explicit BitmapFont(const QString &filePath) { load(filePath); }

    bool load(const QString &filePath);

    QPixmap getChar(int index) const;

private:
    QImage sourceImage;
    QList<quint8> charWidth;
};

bool BitmapFont::load(const QString &filePath)
{
    sourceImage = QImage(filePath);
    if (sourceImage.isNull())
        return false;

    constexpr QRgb separatorColor = qRgb(255, 255, 255);
    charWidth.clear();

    const int bottomY = sourceImage.height() - 1;
    int currentWidth = 0;

    for (int x = 0; x < sourceImage.width(); ++x) {
        const QRgb px = sourceImage.pixel(x, bottomY);
        if (px == separatorColor) {
            charWidth.append(currentWidth);
            currentWidth = 0;
        } else {
            ++currentWidth;
        }
    }

    return true;
}

QPixmap BitmapFont::getChar(int index) const
{
    if (index > charWidth.count())
        return {};

    int fromX = 0;
    for (int i = 0; i < index; ++i)
        fromX += charWidth.at(i);

    return {};
}

void MainWindow::openFile(const QString &filePath)
{
    setWindowFilePath(filePath);

    BitmapFont fnt(filePath);

    for (int i = 40; i < 45; ++i) {
        QGraphicsItem *pText = mScene->addSimpleText(QChar(i));
        pText->setPos(20 + i * 30, 20);
        QGraphicsItem *pBitmapChar = mScene->addPixmap(QPixmap());
        pBitmapChar->setPos(20 + i * 30, 20 + 30);
    }
}
