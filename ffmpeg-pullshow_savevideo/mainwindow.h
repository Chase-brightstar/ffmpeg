//mainwindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#pragma execution_character_set("utf-8")

#include <QMainWindow>
#include "CVideoPlayer.h"
#include <QImage>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void initView();
    void initData();

public slots:
    void slotDecodeError(int error);
    void slotGetOneFrame(QImage image);

protected:
    void paintEvent(QPaintEvent *event);

private:
    Ui::MainWindow *ui;
    CVideoPlayer *m_pVideoPlayer;
    QImage m_Image;
};
#endif // MAINWINDOW_H
