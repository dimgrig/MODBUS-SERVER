#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QTableView>


#include <QApplication>
#include <QCloseEvent>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QStatusBar>
#include <QTableView>
#include <QTimer>

#include <iostream>

#include "aqp.h"
#include "icons.h"
#include "Logger.h"
#include "macroses.h"
#include "templates.h"
#include "TestRunner.h"

#include <QTranslator>
#include <QLibraryInfo>

#include "Profiler.h"

#include "ModbusServer.h"
#include "ModbusClientSettings.h"

//using namespace std;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

#define MODBUS_MS_SLEEP 10

class MainWindow : public QMainWindow {
Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void init_auth_users();
signals:
    void return_pressed();
protected:
    void changeEvent(QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *target, QEvent *event) override;
public slots:

private slots:
    void setDirty() { setWindowModified(true); }

    void auth_ok();
    void on_timer();
    void on_random_timer();
    void modbus_read(QString reg_name, QModbusDataUnit mdu);

    void DiscreteInputsChanged(bool value);
    void CoilsChanged(bool value);
    void InputRegistersChanged(double value);
    void HoldingRegistersChanged(double value);
    void StringInputRegistersChanged(const QString &value);
    void StringHoldingRegistersChanged(const QString &value);
private:
    void connect_signal_slot();
    void read_config();
    void write_config();
    void menu_bar();
    void tool_bar();
    void mvc();
    void docks();
    void auth_users();
    void _saveGeometry();
    void restore_geometry();
    void modbus_ui();

    void test();

    bool okToClearData();

    Ui::MainWindow *ui;

    QTimer *timer = new QTimer(this);
    QTimer *random_timer = new QTimer(this);

    ModbusServer *modbus_server = new ModbusServer(this);
    ModbusClientSettings *modbus_client_settings = new ModbusClientSettings(this);

    QTranslator qtLanguageTranslator;

    std::vector<QCheckBox*> di_v;
    std::vector<QCheckBox*> coil_v;
    std::vector<QDoubleSpinBox*> ir_v;
    std::vector<QDoubleSpinBox*> hr_v;
    std::vector<QLineEdit*> ir_le_v;
    std::vector<QLineEdit*> hr_le_v;

    std::map<QString, QModbusDataUnit> _mdum;

    bool _request = false;
};
#endif // MAINWINDOW_H
