#include "MainWindow.hpp"
#include "ui_MainWindow.h"

#include "BitmapFont.hpp"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <QGraphicsSimpleTextItem>
#include <QImage>
#include <QMimeData>
#include <QStandardPaths>
#include <QUrl>

namespace
{

QList<int> IgnoreChars = {
    ('~' + 1),
    ('~' + 2), // Shield icon
    ('~' + 3),
    158, // Lamp (tech) icon
    169,       170, 171, 172,
};

} // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , mUi(new Ui::MainWindow)
    , mScale(4)
{
    mUi->setupUi(this);

    setWindowTitle(QGuiApplication::applicationDisplayName());
    mUi->actionOpen->setShortcut(QKeySequence::Open);
    mUi->actionReload->setShortcut(QKeySequence::Refresh);
    mUi->actionExit->setShortcut(QString("Ctrl+Q"));

    connect(mUi->actionOpen, &QAction::triggered, this, &MainWindow::browseFile);
    connect(mUi->actionReload, &QAction::triggered, this, &MainWindow::reload);

    mScene = new QGraphicsScene(this);
    mUi->graphicsView->setScene(mScene);
    mUi->graphicsView->setTransform(QTransform::fromScale(mScale, mScale));

    const QStringList docLocations = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
    mDialogPath = !docLocations.isEmpty() ? docLocations.first() : QDir::homePath();
}

MainWindow::~MainWindow()
{
    delete mUi;
}

void MainWindow::browseFile()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Open a bitmap font"),
        mDialogPath,
        tr("Bitmap font Files (*.png *.bmp)"));
    mDialogPath = fileName;

    openFile(fileName);
}

void MainWindow::openFile(const QString &filePath)
{
    setWindowFilePath(filePath);

    QFileInfo info(filePath);
    mFont = std::make_unique<BitmapFont>(filePath);
    setWindowTitle(info.fileName() + " - " + QGuiApplication::applicationDisplayName());
    redrawTable();
}

void MainWindow::reload()
{
    if (mFont)
    {
        mFont->reload();
        redrawTable();
    }
}

void MainWindow::redrawTable()
{
    mScene->clear();

    if (!mFont)
        return;

    const BitmapFont &font = *mFont;
    int pxPerChar = 30;
    int charsPerRow = (mUi->graphicsView->viewport()->width() - 32) / mScale / pxPerChar;

    constexpr int firstChar = '0';
    constexpr int maxChar = 256 - firstChar;
    for (int i = 0; i < maxChar; ++i)
    {
        int charCode = i + firstChar;
        const qreal xCenter = 20 + (i % charsPerRow) * pxPerChar;
        int line = i / charsPerRow;
        const qreal baseY = line * (48 + font.height()) + 20;

        QGraphicsItem* pText = mScene->addSimpleText(QChar(charCode));
        pText->setPos(xCenter - pText->boundingRect().width() / 2, baseY);
        QGraphicsItem* pBitmapChar = mScene->addPixmap(font.getChar(charCode));
        pBitmapChar->setPos(xCenter - pBitmapChar->boundingRect().width() / 2, baseY + 16);
        QGraphicsItem* pText2 = mScene->addSimpleText(QString::number(charCode));
        const qreal xText = xCenter - pText2->boundingRect().width() / 2;
        const qreal yText = pBitmapChar->pos().y() + font.height();
        pText2->setPos(xText, yText);
    }
}
void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if (event == nullptr)
        return;

    if (const QMimeData* mime = event->mimeData(); mime != nullptr && mime->hasUrls())
    {
        const QList<QUrl> urls = mime->urls();
        for (const QUrl &url : urls)
        {
            if (url.isLocalFile())
            {
                event->acceptProposedAction();
                return;
            }
        }
    }
}

void MainWindow::dropEvent(QDropEvent* event)
{
    if (event == nullptr)
        return;

    const QMimeData* mime = event->mimeData();
    if (mime == nullptr || !mime->hasUrls())
        return;

    const QList<QUrl> urls = mime->urls();
    for (const QUrl &url : urls)
    {
        if (!url.isLocalFile())
            continue;

        const QString filePath = url.toLocalFile();
        if (!filePath.isEmpty())
        {
            openFile(filePath);
            event->acceptProposedAction();
            return;
        }
    }
}
void MainWindow::resizeEvent(QResizeEvent* event)
{
    redrawTable();
}
