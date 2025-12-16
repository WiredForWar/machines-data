#pragma once

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

QT_FORWARD_DECLARE_CLASS(QGraphicsScene)

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void browseFile();

    void openFile(const QString &filePath);

private:
    Ui::MainWindow *mUi{};
    QGraphicsScene *mScene{};
};
