#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QMessageBox>
#include <QFileDialog>  // For file dialog
#include <QFile>       // For file operations
#include <QTextStream> // For reading from the file

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    serialPort = new QSerialPort(this);

    // Populate serial port combo box
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        ui->portComboBox->addItem(info.portName());
    }

    // Populate baud rate combo box
    ui->baudRateComboBox->addItem("9600");  // Add common baud rates
    ui->baudRateComboBox->addItem("115200");
    ui->baudRateComboBox->setCurrentText("115200"); // Set default

    connect(serialPort, &QSerialPort::readyRead, this, &MainWindow::readData);
}

MainWindow::~MainWindow()
{
    delete ui;
    if (serialPort->isOpen()) {
        serialPort->close();
    }
    delete serialPort;
}

void MainWindow::on_connectButton_clicked()
{
    if (serialPort->isOpen()) {
        // Disconnect
        serialPort->close();
        ui->connectButton->setText("Connect");
        ui->outputTextBox->append("Disconnected");
    } else {
        // Connect
        serialPort->setPortName(ui->portComboBox->currentText());
        serialPort->setBaudRate(ui->baudRateComboBox->currentText().toInt());
        serialPort->setDataBits(QSerialPort::Data8);
        serialPort->setParity(QSerialPort::NoParity);
        serialPort->setStopBits(QSerialPort::OneStop);
        serialPort->setFlowControl(QSerialPort::NoFlowControl);

        if (serialPort->open(QIODevice::ReadWrite)) {
            ui->connectButton->setText("Disconnect");
            ui->outputTextBox->append("Connected to " + ui->portComboBox->currentText());
        } else {
            QMessageBox::critical(this, "Error", "Failed to open serial port: " + serialPort->errorString());
            ui->outputTextBox->append("Failed to connect");
        }
    }
}


void MainWindow::on_sendButton_clicked()
{
    if (serialPort->isOpen()) {
        QString gCode = ui->gCodePlainTextEdit->toPlainText(); // Get entire text
        QStringList lines = gCode.split("\n"); // Split into individual lines

        foreach (const QString &line, lines) {
            QString trimmedLine = line.trimmed(); // Remove whitespace
            if (!trimmedLine.isEmpty()) {          // Skip empty lines
                QString lineWithNewline = trimmedLine + "\n"; // Add newline to each line
                serialPort->write(lineWithNewline.toUtf8()); // Send the G-code line
                ui->outputTextBox->append("Sent: " + lineWithNewline);
            }
        }
    } else {
        QMessageBox::warning(this, "Warning", "Not connected to serial port.");
    }
}

void MainWindow::readData()
{
    QByteArray data = serialPort->readAll();
    ui->outputTextBox->append("Received: " + QString(data));
}


void MainWindow::on_loadFileButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Open G-code File",
                                                    QDir::homePath(),
                                                    "G-code Files (*.gcode *.nc);;All Files (*)");

    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString fileContent = in.readAll();
            ui->gCodePlainTextEdit->setPlainText(fileContent);

            // Extract and set the file name in the line edit
            QFileInfo fileInfo(fileName);
            ui->fileNameLineEdit->setText(fileInfo.fileName());

            file.close();
        } else {
            QMessageBox::critical(this, "Error", "Could not open file: " + file.errorString());
        }
    }
}


void MainWindow::on_clearButton_clicked()
{
     ui->gCodePlainTextEdit->clear();
}

