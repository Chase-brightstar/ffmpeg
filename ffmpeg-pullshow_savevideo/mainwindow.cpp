//mainwindow.cpp
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QPainter>
#include <iostream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("客户端拉流");
    initView();
    initData();
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::initView()
{

}

void MainWindow::initData()
{
    m_pVideoPlayer = new CVideoPlayer();
    connect(m_pVideoPlayer, SIGNAL(signalDecodeError(int)), this, SLOT(slotDecodeError(int)));
    connect(m_pVideoPlayer, SIGNAL(signalGetOneFrame(QImage)), this, SLOT(slotGetOneFrame(QImage)));
    m_pVideoPlayer->start();
}

void MainWindow::slotDecodeError(int error)
{
    qDebug() << "slotDecodeError======error====" << error;
}

void MainWindow::slotGetOneFrame(QImage image)
{
    m_Image = image;
    update();//调用update将执行 paintEvent函数
}

void MainWindow::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setBrush(Qt::white);
    painter.drawRect(0, 0, this->width(), this->height());//先画成黑色
    if(m_Image.size().width() <= 0)
        return;
    //将图像按比例缩放成和窗口一样大小
    QImage img = m_Image.scaled(this->size(), Qt::KeepAspectRatio);
    int x = this->width() - img.width();
    int y = this->height() - img.height();
    x /=  2;
    y /= 2;
    painter.drawImage(QPoint(x, y), img); //画出图像
    painter.end();

}
