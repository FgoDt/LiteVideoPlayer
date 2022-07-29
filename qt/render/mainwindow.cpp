#include "mainwindow.h"
#include "ui_mainwindow.h"
//#include "lvp_qt_render.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //lvp_qt_render *r = new lvp_qt_render();
}

MainWindow::~MainWindow()
{
    delete ui;
}
