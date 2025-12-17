#include "MainWindow.hpp"
#include "ui_MainWindow.h"

#include "BitmapFont.hpp"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QFontDatabase>
#include <QFontDialog>
#include <QFontMetrics>
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

enum class FileType
{
    Font,
    Image,
};

std::optional<FileType> getFileType(const QUrl &fileUrl)
{
    const QFileInfo fileInfo(fileUrl.path());
    const QString extension = fileInfo.completeSuffix().toLower();
    if (extension == QLatin1String("ttf"))
        return FileType::Font;

    if (extension == QLatin1String("bmp") || extension == QLatin1String("png"))
        return FileType::Image;

    return std::nullopt;
}

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
    connect(mUi->actionSelectReferenceFont, &QAction::triggered, this, &MainWindow::selectReferenceFont);

    mScene = new QGraphicsScene(this);
    mUi->graphicsView->setScene(mScene);
    mUi->graphicsView->setTransform(QTransform::fromScale(mScale, mScale));

    const QStringList docLocations = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
    mDialogPath = !docLocations.isEmpty() ? docLocations.first() : QDir::homePath();

    mReferenceFont = font();
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
    redrawTable();
}

void MainWindow::openFile(const QString &filePath)
{
    setWindowFilePath(filePath);

    QFileInfo info(filePath);
    mFont = std::make_unique<BitmapFont>(filePath);
    setWindowTitle(info.fileName() + " - " + QGuiApplication::applicationDisplayName());
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

    QFontMetrics referenceFontMetrics(mReferenceFont);
    const BitmapFont &font = *mFont;
    int pxPerChar = 30;
    int charsPerRow = (mUi->graphicsView->viewport()->width() - 32) / mScale / pxPerChar;

    constexpr int firstChar = '!';
    constexpr int maxChar = 256 - firstChar;
    for (int i = 0; i < maxChar; ++i)
    {
        int charCode = i + firstChar;
        const qreal xCenter = 20 + (i % charsPerRow) * pxPerChar;
        int line = i / charsPerRow;
        const qreal baseY = line * (42 + font.height() + referenceFontMetrics.height()) + 20;

        QGraphicsItem* pReferenceCharacter = mScene->addSimpleText(QChar(charCode), mReferenceFont);
        pReferenceCharacter->setPos(xCenter - pReferenceCharacter->boundingRect().width() / 2, baseY);
        QGraphicsItem* pBitmapCharacter = mScene->addPixmap(font.getChar(charCode));
        pBitmapCharacter->setPos(xCenter - pBitmapCharacter->boundingRect().width() / 2, baseY + referenceFontMetrics.height());
        QGraphicsItem* pCode = mScene->addSimpleText(QString::number(charCode));
        const qreal xText = xCenter - pCode->boundingRect().width() / 2;
        const qreal yText = pBitmapCharacter->pos().y() + font.height();
        pCode->setPos(xText, yText);
    }
}

void MainWindow::selectReferenceFont()
{
    mReferenceFont = QFontDialog::getFont(nullptr, mReferenceFont);
    redrawTable();
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
                std::optional<FileType> fileType = getFileType(url);
                if (fileType.has_value())
                {
                    event->acceptProposedAction();
                }
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

        std::optional<FileType> fileType = getFileType(url);
        if (fileType == FileType::Image)
        {
            const QString filePath = url.toLocalFile();
            if (!filePath.isEmpty())
            {
                openFile(filePath);
                event->acceptProposedAction();
            }
        }
        else if (fileType == FileType::Font)
        {
            int fontId = QFontDatabase::addApplicationFont(url.toLocalFile());
            if (fontId >= 0)
            {
                QStringList families = QFontDatabase::applicationFontFamilies(fontId);
                if (!families.isEmpty())
                {
                    mReferenceFont.setFamilies(families);
                    event->acceptProposedAction();
                }
            }
        }
    }

    if (event->isAccepted())
    {
        redrawTable();
    }
}
void MainWindow::resizeEvent(QResizeEvent* event)
{
    redrawTable();
}
