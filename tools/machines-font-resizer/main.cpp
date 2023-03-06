#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QImage>
#include <QPainter>

constexpr int c_DefaultScale = 2;
static int c_Scale = c_DefaultScale;

enum class ProcessingType
{
    NoProcessing,
    FontScaling,
    FontRefining,
};

QRgb mixColors(const QRgb &c1, const QRgb &c2, qreal balance)
{
    const int red = qRed(c1) * (1 - balance) + qRed(c2) * balance;
    const int green = qGreen(c1) * (1 - balance) + qGreen(c2) * balance;
    const int blue = qBlue(c1) * (1 - balance) + qBlue(c2) * balance;
    return qRgb(red, green, blue);
}

bool colorIsInRange(const QRgb &c, const QRgb &from, const QRgb &to)
{
    using ComponentGetter = int(*)(QRgb);
    static const ComponentGetter componentGetters[] = { &qRed, &qGreen, &qBlue };
    for (const ComponentGetter &getter : componentGetters) {
        if (getter(c) > (qMax<int>(getter(from), getter(to)))) {
            return false;
        }
        if (getter(c) < (qMin<int>(getter(from), getter(to)))) {
            return false;
        }
    }

    return true;
}

// From smoothedCharIndicex generated on promtfnt_2x
const QVector<int> s_colorableCharIndices = {
    33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 58, 59, 60, 61, 62, 63, 64, 65, 66,
    67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90,
    91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
    112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 129, 130, 131,
    132, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151,
    152, 153, 154, 155, 156, 158, 159, 161, 167, 168, 175, 176, 177, 178, 179, 180, 181, 182, 183,
    184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202,
    203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221,
    222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240,
    241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255
};

QVector<int> s_ignoreChars = {
    ('~' + 1),
    ('~' + 2), // Shield icon
    ('~' + 3),
    158, // Lamp (tech) icon
    169, 170, 171, 172, //
};

void processFont(const QImage &sourceImage, QImage *pDestImage, ProcessingType processing)
{
    if (processing == ProcessingType::NoProcessing) {
        QPainter scaledImagePainter;
        scaledImagePainter.begin(pDestImage);
        scaledImagePainter.drawImage(0, 0, sourceImage, 0, 0, sourceImage.width(), sourceImage.height());
        return;
    }

    QColor backgroundColor = sourceImage.pixel(0, 0);

    const QRgb gradientFrom = qRgb(0xff, 0xff, 0xff);
    const QRgb gradientTo = qRgb(0xff, 0xff, 0xb5 - 0x12); // tune the color one step further
    const QRgb digitsColor = qRgb(0xff, 0xff, 0x00);

    QVector<int> smoothedCharIndicex;
    smoothedCharIndicex.reserve(256);

    int targetX = 0;
    {
        QPainter scaledImagePainter;
        scaledImagePainter.begin(pDestImage);
        scaledImagePainter.setPen(backgroundColor);
        constexpr QRgb separatorColor = qRgb(255, 255, 255);
        constexpr QRgb mainColor = qRgb(0, 0, 0);
        int bottomY = sourceImage.height() - 1;
        int fontCharId = 0;
        bool coloringAllowed = false;
        bool smoothed = false;
        bool shadowFont = true;
        for (int x = 0; x < sourceImage.width(); ++x) {
            const bool currentCharIsDigit = (fontCharId >= '0') && (fontCharId <= '9');

            if (shadowFont) {
                if (processing == ProcessingType::FontRefining) {
                    coloringAllowed = currentCharIsDigit || (s_colorableCharIndices.contains(fontCharId) && fontCharId <= '~');
                }
            } else {
                coloringAllowed = s_colorableCharIndices.contains(fontCharId) && !s_ignoreChars.contains(fontCharId);
            }

            if (coloringAllowed) {
                scaledImagePainter.drawImage(targetX, 0, sourceImage, x, 0, 1, sourceImage.height());
                // The last row is used only for chars delimiter
                int destHeight = pDestImage->height() - 1 * c_Scale;
                for (int i = 0; i < destHeight; ++i) {
                    QRgb px = sourceImage.pixel(x, i);
                    bool applyColorToPx = shadowFont ? (px == mainColor) : colorIsInRange(px, gradientFrom, gradientTo);
                    if (applyColorToPx) {
                        QRgb newColor = px;
                        if (currentCharIsDigit) {
                            newColor = digitsColor;
                        } else {
                            qreal gradientPos = static_cast<qreal>(i) / destHeight;
                            newColor = mixColors(gradientFrom, gradientTo, gradientPos);

                            if (!smoothed) {
                                smoothedCharIndicex.append(fontCharId);
                                smoothed = true;
                                qWarning() << "Smooth gradient for char" << QChar::fromLatin1(fontCharId);
                            }
                        }

                        pDestImage->setPixel(targetX, i, newColor);
                    }
                }
            } else {
                scaledImagePainter.drawImage(targetX, 0, sourceImage, x, 0, 1, sourceImage.height());
            }

            QRgb px = sourceImage.pixel(x, bottomY);
            if (px == separatorColor) {
                for (int i = 1; i < c_Scale; ++i) {
                    scaledImagePainter.drawPoint(targetX, sourceImage.height() -1 - i);
                    // pDestImage->setPixel(targetX, image.height() -1 - i, backgroundColor.rgb());
                }
                if (processing == ProcessingType::FontScaling) {
                    x += (c_Scale - 1);
                }
                fontCharId++;
                smoothed = false;
            }
            targetX++;
        }
    }

    *pDestImage = pDestImage->copy(0, 0, targetX, sourceImage.height());

    // qWarning() << "Smoothed chars array:" << smoothedCharIndicex;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("Machines resource converter");
    QCoreApplication::setApplicationVersion("0.1");

    QCommandLineOption processFontOption("process-font", "Apply the font-specific processing");
    QCommandLineOption refineFontOption("refine-font", "Refine the font-using a map file");
    QCommandLineOption scaleOption("scale", "The output scale", "scale", QString::number(c_DefaultScale));
    QCommandLineOption mapFileOption("map-file", "The refinement source map file", "filename");
    QCommandLineParser parser;
    parser.addPositionalArgument("filename", "Source file name");
    parser.addOption(processFontOption);
    parser.addOption(refineFontOption);
    parser.addOption(scaleOption);
    parser.addOption(mapFileOption);
    parser.process(app);

    bool ok;
    c_Scale = parser.value(scaleOption).toInt(&ok);
    if (!ok) {
        parser.showHelp();
        Q_UNREACHABLE();
    }

    if (parser.isSet(refineFontOption)) {
        if (!parser.isSet(mapFileOption)) {
            parser.showHelp();
            Q_UNREACHABLE();
        }
    }

    const QStringList args = parser.positionalArguments();
    const QString scaleSuffix = c_Scale > 1 ? QString("_%1x").arg(c_Scale) : QString();
    for (const QString &sourceFileName : args) {
        QImage image(sourceFileName);

        QString destFileName = sourceFileName;
        if (sourceFileName.endsWith(".bmp")) {
            destFileName.replace(".bmp", scaleSuffix + QString(".png"));
            image = image.scaled(image.size() * c_Scale, Qt::IgnoreAspectRatio, Qt::FastTransformation);
        } else {
            destFileName.replace(".png", "_2.png");
        }

        QImage destImage(image.size(), QImage::Format_RGB32);

        ProcessingType processing = ProcessingType::NoProcessing;
        if (parser.isSet(processFontOption)) {
            processing = ProcessingType::FontScaling;
        }
        if (parser.isSet(refineFontOption)) {
            processing = ProcessingType::FontRefining;
        }

        processFont(image, &destImage, processing);
        destImage.save(destFileName);
    }

    return 0;
}
