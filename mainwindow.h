#pragma once

#ifdef QT_WIN
#define WIN32_MEAN_AND_LEAN // necessary to avoid compiler errors
#include <Windows.h>
#endif

#include <QMainWindow>
#include <QKeyEvent>
#include <QShowEvent>
#include <QBitArray>
#include <QSystemTrayIcon>

#include "settingsdialog.h"

class QLabel;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

using CharSets = QVector<QString>;
using WritingSystem = QPair<QString, CharSets>;
using WritingSystems = QVector<WritingSystem>;
using VowelMap = QMap<QChar, WritingSystems>;

using WidgetVector = QVector<QWidget*>;
using WidgetPair = QPair<qint32, WidgetVector>;
using WidgetMap = QMap<QChar, WidgetPair>;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    bool is_showing() const { return isVisible(); }

    void process_show();

    bool set_modifier(unsigned long win_key);
    void reset_modifier(unsigned long win_key);
    bool have_modifiers() const;
    bool should_activate(unsigned long win_key);

protected:
    void showEvent(QShowEvent* event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;

private slots:
    void slot_char_selected(const QString);
    void slot_mru_selected(const QString);
    void slot_icon_activated(QSystemTrayIcon::ActivationReason reason);
    void slot_menu_action(QAction* action);
    void slot_quit();

    void slot_settings_accepted();
    void slot_settings_rejected();
    void slot_settings();

private: // typedefs and enums
    enum
    {
        Modifier_Ctrl = 0,
        Modifier_Alt,
        Modifier_Shift,
        Modifier_Windows,
        Modifier_Max
    };

private: // methods
    bool load_settings();
    bool load_glyphs();
    void save_settings();

    void map_sequence_key();
    void activate_sequence(const QString seq);

    void build_tray_menu();

    void cycle_writing_system(const QChar& vowel);
    void rebuild_MRU();
    void position_window(bool is_event = true);

    void hook_global_keyboard_event();
    void unhook_global_keyboard_event();

private: // data members
    Ui::MainWindow *ui{nullptr};

    QSystemTrayIcon* trayIcon;
    QMenu* trayIconMenu{nullptr};

    QAction* quit_action{nullptr};

#ifdef QT_WIN
    HHOOK keyboard_hook_handle{nullptr};
#endif

    QChar last_cycle_vowel;

    WidgetMap sections;

    bool double_clicked_to_open{false};
    QLabel* last_highlighted_label{nullptr};
    QPoint last_position;

    SettingsDialog* settings_dlg{nullptr};
    Settings settings;

    unsigned long activation_key{'Z'};
    QVector<int> needed_modifiers;
    QBitArray active_modifiers{Modifier_Max};

    QStringList folders;
    QStringList mru;
};

extern MainWindow* main_window;
