#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QProcess>
#include <QStandardItemModel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // setup tableview
    model = new QStandardItemModel(this);
    ui->tableViewData->setModel(model);
    model->setColumnCount(3);
    // remove vertical headers
    ui->tableViewData->verticalHeader()->hide();
    // add proper names to horisontal headers
    model->setHeaderData(0, Qt::Horizontal, "Parameter", Qt::DisplayRole);
    model->setHeaderData(1, Qt::Horizontal, "Worst", Qt::DisplayRole);
    model->setHeaderData(2, Qt::Horizontal, "Value", Qt::DisplayRole);

    // conect signals
    connect(ui->comboBoxDriveSelect, SIGNAL(currentIndexChanged(int)), this, SLOT(currentIndexChanged(int)));

    // basic UI setup
    ui->labelDriveName->setText("Select drive first");
    ui->labelDriveSerial->setText("unknown");

    // request list of drives
    QStringList args;
    args << "--scan" << "-j";
    QString driveList= runProcess("smartctl", args);
    parseSmartctlOutput_List(driveList);
    currentIndexChanged(0);
}

// Gets data from process stdout
QString MainWindow::runProcess(QString app, QStringList args)
{
    QProcess process;
    process.start(app, args);

    // check if process started correctly
    if (!process.waitForStarted()){
        QMessageBox msgBox;
        msgBox.setText(QString("Cannot execute %1 %2").arg(app, args.join(" ")));
        msgBox.exec();
        return QString();
    }
    // wait for process to finish
    // TODO perform async request
    process.waitForFinished();
    QString output(process.readAllStandardOutput());
    process.close();
    return output;
}

// Parses smartctl output for drive info
void MainWindow::parseSmartctlOutput_List(QString data)
{
    // convert QString to JSON
    QJsonDocument d = QJsonDocument::fromJson(data.toUtf8());
    QJsonObject json = d.object();

    // loop through drive's list
    QJsonArray jsonArray = json["devices"].toArray();

    foreach (const QJsonValue & value, jsonArray) {
        QJsonObject obj = value.toObject();
        QString name = obj["name"].toString();
        QString info_name = obj["info_name"].toString();
        QString protocol = obj["protocol"].toString();

        ui->comboBoxDriveSelect->addItem(QString("%1 (%2)").arg(info_name, protocol), name);
    }
}


// Parses smartctl output for drive info
void MainWindow::parseSmartctlOutput_Drive(QString data)
{
    // convert QString to JSON
    QJsonDocument d = QJsonDocument::fromJson(data.toUtf8());
    QJsonObject json = d.object();

    // read hard drive modela and serial no
    ui->labelDriveName->setText(json["model_name"].toString());
    ui->labelDriveSerial->setText(json["serial_number"].toString());

    // loop through SMART attributes
    QJsonObject ata_smart_attributes = json["ata_smart_attributes"].toObject();
    QJsonArray jsonArray = ata_smart_attributes["table"].toArray();

    foreach (const QJsonValue & value, jsonArray) {
        QJsonObject obj = value.toObject();
        QString name = obj["name"].toString();
        QString worst = QString::number(obj["worst"].toInt());
        QJsonObject raw = obj["raw"].toObject();
        QString string = raw["string"].toString();
        // remove underscores from attribute name
        name = name.replace('_', ' ');

        tableViewData_appendRow(name, worst, string);
    }

    // read NVME SMART health information
    QJsonObject nvme = json["nvme_smart_health_information_log"].toObject();
    foreach (QString key, nvme.keys()) {
        tableViewData_appendRow(key.replace('_', ' '),
                                QString(),
                                QString::number(nvme.value(key).toInt())
                                );
    }

    // resize QTableView columns to clearly diaplay data
    ui->tableViewData->resizeColumnsToContents();

}

void MainWindow::currentIndexChanged(int index)
{
    // remove current printed SMART info
    model->clear();

    // get selected item from QCombobox
    QString dev = ui->comboBoxDriveSelect->itemData(index).toString();
    QStringList args;
    args << "-x" << "-j" << dev;
    // request SMART info for selected drive
    QString driveInfo = runProcess("smartctl", args);
    parseSmartctlOutput_Drive(driveInfo);
}

// adds data to tableview
void MainWindow::tableViewData_appendRow(QString cell1, QString cell2, QString cell3)
{
    // create QStandardItem row with 3 cells
    QList<QStandardItem*> items;
    QStandardItem * const item1 = new QStandardItem(cell1);
    items.append(item1);
    QStandardItem * const item2 = new QStandardItem(cell2);
    items.append(item2);
    QStandardItem * const item3 = new QStandardItem(cell3);
    items.append(item3);
    // append row to data model
    model->appendRow(items);
}

MainWindow::~MainWindow()
{
    delete ui;
}

