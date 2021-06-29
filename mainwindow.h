#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void parseSmartctlOutput_Drive(QString data);
    void parseSmartctlOutput_List(QString data);
    QString runProcess(QString app, QStringList args);

public slots:
    void currentIndexChanged(int index);

private:
    void tableViewData_appendRow(QString cell1, QString cell2, QString cell3);
    Ui::MainWindow *ui;
    QStandardItemModel *model;
};
#endif // MAINWINDOW_H
