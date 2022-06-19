#include "settingsdialog.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(const Settings& settings, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog),
    settings(settings)
{
    Qt::WindowFlags flags{Qt::WindowStaysOnTopHint|Qt::FramelessWindowHint};
    setWindowFlags(flags);

    ui->setupUi(this);

    connect(this, &SettingsDialog::accepted, this, &SettingsDialog::slot_accepted);
    connect(ui->sequence_Activation, &QKeySequenceEdit::keySequenceChanged, this, &SettingsDialog::slot_check_sequence);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::showEvent(QShowEvent* event)
{
    if(!settings.sequence.isEmpty())
        ui->sequence_Activation->setKeySequence(settings.sequence);
#ifdef QT_WIN
    ui->check_Windows->setChecked(settings.use_windows_key);
#endif

    QDialog::showEvent(event);
}

void SettingsDialog::slot_accepted()
{
    settings.sequence = ui->sequence_Activation->keySequence();
#ifdef QT_WIN
    settings.use_windows_key = ui->check_Windows->isChecked();
#endif
}

void SettingsDialog::slot_check_sequence(const QKeySequence& sequence)
{
    if(ui->check_Windows->isChecked())
        ui->check_Windows->setChecked(!sequence.toString().contains("+"));
}
