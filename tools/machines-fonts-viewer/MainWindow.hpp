#pragma once

#include <QMainWindow>

#include <QFont>

#include <memory>

QT_FORWARD_DECLARE_CLASS(QGraphicsScene)

namespace Ui
{
class MainWindow;
}

class BitmapFont;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    void setNumbersBase(int base);
private:
    void browseFile();

    void openFile(const QString &filePath);
    void reload();
    void redrawTable();
    void selectReferenceFont();

    void resizeEvent(QResizeEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    Ui::MainWindow* mUi{};
    QGraphicsScene* mScene{};
    std::unique_ptr<BitmapFont> mFont{};
    QFont mReferenceFont;
    QString mDialogPath;
    float mScale{};
    int mNumbersBase{};
};
