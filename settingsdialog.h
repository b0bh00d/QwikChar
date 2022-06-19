#pragma once

#include <QDialog>
#include <QSettings>

namespace Ui {
class SettingsDialog;
}

struct Settings
{
    QKeySequence sequence;
#ifdef QT_WIN
    bool use_windows_key{false};
#endif

    void save(QSettings& app_settings)
    {
        app_settings.beginGroup("Settings");
        app_settings.setValue("activation", sequence.toString());
        app_settings.setValue("use_windows_key", use_windows_key);
        app_settings.endGroup();
    }

    void load(QSettings& app_settings)
    {
        app_settings.beginGroup("Settings");
        sequence = QKeySequence::fromString(app_settings.value("activation", "Z").toString());
        use_windows_key = app_settings.value("use_windows_key", true).toBool();
        app_settings.endGroup();
    }
};

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(const Settings& settings = Settings(), QWidget *parent = nullptr);
    ~SettingsDialog();

    void set_sequence(const QKeySequence& seq) { settings.sequence = seq; }
#ifdef QT_WIN
    void set_windows_key(bool key) { settings.use_windows_key = key; }
#endif

    void get_settings(Settings& settings_) const { settings_ = settings; }

protected:
    void showEvent(QShowEvent* event);

private slots:
    void slot_accepted();
    void slot_check_sequence(const QKeySequence& sequence);

private:
    Ui::SettingsDialog *ui;

    Settings settings;
};
