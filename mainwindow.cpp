#include <QDir>
#include <QMap>
#include <QTimer>
#include <QScreen>
#include <QVector>
#include <QSettings>
#include <QClipboard>
#include <QMessageBox>
#include <QStyleFactory>
#include <QStandardPaths>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "selectablechar.h"

static auto application = QStringLiteral("QwikChar");

// need this exposed globally so the Windows API calls can interact with us
MainWindow* main_window{nullptr};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    main_window = this;

    setWindowTitle(tr("QwikChar by Bob Hood"));
    setWindowIcon(QIcon(":/images/QwikChar.png"));

    Qt::WindowFlags flags{Qt::WindowStaysOnTopHint|Qt::FramelessWindowHint};
    setWindowFlags(flags);

    qApp->setStyle(QStyleFactory::create("Fusion"));

    QPalette p = palette();

    p.setColor(QPalette::Window, QColor(53, 53, 53));
    p.setColor(QPalette::WindowText, Qt::white);
    p.setColor(QPalette::Base, QColor(25, 25, 25));
    p.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    p.setColor(QPalette::ToolTipBase, Qt::black);
    p.setColor(QPalette::ToolTipText, Qt::white);
    p.setColor(QPalette::Text, Qt::white);
    p.setColor(QPalette::Button, QColor(53, 53, 53));
    p.setColor(QPalette::ButtonText, Qt::white);
    p.setColor(QPalette::BrightText, Qt::red);
    p.setColor(QPalette::Link, QColor(42, 130, 218));
    p.setColor(QPalette::Highlight, QColor(42, 130, 218));
    p.setColor(QPalette::HighlightedText, Qt::black);
    qApp->setPalette(p);

    setGeometry(100, 100, 100, 100);

    ui->setupUi(this);

    connect(ui->label_Settings, &ClickableLabel::signal_clicked, this, &MainWindow::slot_settings);

    if(load_settings())
    {
        // Tray

        trayIcon = new QSystemTrayIcon(this);
        // connect(trayIcon, &QSystemTrayIcon::messageClicked, this, &MainWindow::slot_message_clicked);
        connect(trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::slot_icon_activated);

        trayIcon->setIcon(QIcon(":/images/QwikChar.png"));
        trayIcon->setToolTip(tr("QwikChar"));

        build_tray_menu();

        trayIcon->show();

        hook_global_keyboard_event();
    }
}

MainWindow::~MainWindow()
{
    delete ui;

    unhook_global_keyboard_event();
}

bool MainWindow::load_settings()
{
    // settings
    folders.append(QDir::toNativeSeparators(QString("%1/QwikChar")
                                .arg(QStandardPaths::standardLocations(
                                    QStandardPaths::GenericConfigLocation)[0])));
    // application
    folders.append(QDir::toNativeSeparators(QApplication::applicationDirPath()));

    QDir settings_dir(folders[0]);
    if(!settings_dir.exists())
    {
        if(!settings_dir.mkpath("."))
        {
            QMessageBox::critical(nullptr,
                                  tr("QwikChar: Error"),
                                  tr("Could not create application settings folder:\n%1").arg(folders[0]));
            QTimer::singleShot(100, qApp, &QApplication::quit);
            return false;
        }
    }

    QString settings_file = QString("%1/settings.ini").arg(folders[0]);
    QSettings app_settings(settings_file, QSettings::IniFormat);

    settings.load(app_settings);

    map_sequence_key();

    if(!load_glyphs())
    {
        QTimer::singleShot(100, qApp, &QApplication::quit);
        return false;
    }

    return true;
}

bool MainWindow::load_glyphs()
{
    QDomDocument glyph_database(application);
    QDomElement root = glyph_database.createElement(application);
    root.setAttribute("version", "1");

    glyph_database.appendChild(root);

    QDomNode node(glyph_database.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf8\""));
    glyph_database.insertBefore(node, glyph_database.firstChild());

    // first, look in the same folder as our settings in case
    // the user has customized their glyphs

    QString glyph_file = QString("%1/glyphs.xml").arg(folders[0]);

    QFile xml;
    xml.setFileName(glyph_file);
    if(!xml.open(QFile::ReadOnly | QFile::Text))
    {
        // fall back to the default shipping list
        glyph_file = QString("%1/glyphs.xml").arg(folders[1]);
        xml.setFileName(glyph_file);
        if(!xml.open(QFile::ReadOnly | QFile::Text))
        {
            // we have no glyph file!
            QMessageBox::critical(nullptr,
                                  tr("QwikChar: Error"),
                                  tr("No glyph file cound be found!"));
            return false;
        }
    }

    VowelMap vowel_map;

    QString errorStr, versionStr = "1";
    int errorLine;
    int errorColumn;

    if(glyph_database.setContent(&xml, true, &errorStr, &errorLine, &errorColumn))
    {
        root = glyph_database.documentElement();
        if(!root.tagName().compare(application))
        {
            if(root.hasAttribute("version"))
                versionStr = root.attribute("version");

            auto version = versionStr.toInt();
            Q_UNUSED(version)

            auto children = root.childNodes();
            for(int i = 0;i < children.length();i++)
            {
                auto node = children.at(i);
                if(!node.nodeName().compare("key"))
                {
                    auto element = node.toElement();
                    QChar key = element.attribute("value")[0];

                    WritingSystems systems;

                    auto children = node.childNodes();
                    for(int j = 0;j < children.length();j++)
                    {
                        auto node = children.at(j);
                        if(!node.nodeName().compare("system"))
                        {
                            auto element = node.toElement();
                            auto system = element.attribute("name");

                            CharSets sets;

                            auto children = node.childNodes();
                            for(int k = 0;k < children.length();k++)
                            {
                                auto node = children.at(k);
                                auto name = node.nodeName();
                                if(!node.nodeName().compare("glyph"))
                                {
                                    QString glyph;
                                    auto element = node.toElement();
                                    auto value = element.attribute("value");
                                    if(value.startsWith("0x"))
                                    {
                                        bool ok;
                                        auto range = value.split('-');
                                        auto start = range[0].toUInt(&ok, 16);
                                        if(ok)
                                        {
                                            auto end = start;
                                            if(range.length() > 1)
                                                end = range[1].toUInt(&ok, 16);

                                            if(ok)
                                            {
                                                auto i = start;
                                                for(; i < end;++i)
                                                    sets.push_back(QChar(i));
                                                glyph = QChar(i);
                                            }
                                        }
                                    }
                                    else
                                        glyph = value;

                                    sets.push_back(glyph);
                                }
                            }

                            systems.push_back(qMakePair(system, sets));
                        }
                    }

                    vowel_map[key] = systems;
                }
            }
        }
    }

    auto row = 0;

    for(QChar key : vowel_map.keys())
    {
        if(!sections.contains(key))
            sections[key] = qMakePair(0, QVector<QWidget*>());

        for(auto pair : vowel_map[key])
        {
            auto label = new QLabel(pair.first);
            auto font = label->font();
            font.setPointSize(14);
            label->setFont(font);

            sections[key].second.append(label);

            ui->gridLayout->addWidget(label, row++, 0, 1, 10);

            auto col = 0;
            for(auto iter = pair.second.begin();iter != pair.second.end();++iter)
            {
                if(iter->length() <= 10)
                {
                    auto sc = new SelectableChar(*iter, this);
                    auto col_span = static_cast<int>(sc->width() / sc->height());

                    connect(sc, &SelectableChar::signal_selected, this, &MainWindow::slot_char_selected);

                    // if there's not enoguh room left on this row, start on
                    // the next
                    if((col + col_span) >= 10)
                    {
                        ++row;
                        col = 0;
                    }

                    ui->gridLayout->addWidget(sc, row, col, 1, col_span, Qt::AlignLeft);
                    col += col_span;
                }
            }

            ++row;
        }
    }

    return true;
}

void MainWindow::save_settings()
{
    QString settings_file = QString("%1/settings.ini").arg(folders[0]);
    QSettings app_settings(settings_file, QSettings::IniFormat);
    app_settings.clear();

    settings.save(app_settings);
}

bool MainWindow::set_modifier(unsigned long win_key)
{
#ifdef QT_WIN
    if(win_key == VK_LWIN || win_key == VK_RWIN)
        active_modifiers[Modifier_Windows] = true;
    else if(win_key == VK_LCONTROL || win_key == VK_RCONTROL)
        active_modifiers[Modifier_Ctrl] = true;
    else if(win_key == VK_LMENU || win_key == VK_RMENU)
        active_modifiers[Modifier_Alt] = true;
    else if(win_key == VK_LSHIFT || win_key == VK_RSHIFT)
        active_modifiers[Modifier_Shift] = true;
#endif

    return have_modifiers();
}

void MainWindow::reset_modifier(unsigned long win_key)
{
#ifdef QT_WIN
    if(win_key == VK_LWIN || win_key == VK_RWIN)
        active_modifiers[Modifier_Windows] = false;
    else if(win_key == VK_LCONTROL || win_key == VK_RCONTROL)
        active_modifiers[Modifier_Ctrl] = false;
    else if(win_key == VK_LMENU || win_key == VK_RMENU)
        active_modifiers[Modifier_Alt] = false;
    else if(win_key == VK_LSHIFT || win_key == VK_RSHIFT)
        active_modifiers[Modifier_Shift] = false;
#endif
}

bool MainWindow::have_modifiers() const
{
    bool have_all = true;
    for(int mod : needed_modifiers)
    {
        if(!active_modifiers[mod])
            have_all = false;
    }

    return have_all;
}

bool MainWindow::should_activate(unsigned long win_key)
{
    return win_key == activation_key && have_modifiers();
}

void MainWindow::position_window(bool is_event)
{
    if(is_event || last_position.isNull())
    {
        // center the window on the mouse cursor, and adjust
        // to make sure it doesn't exceed the screen boundaries

        auto g = geometry();
        auto x_offset = static_cast<int>(g.width() / 2);
        auto y_offset = static_cast<int>(g.height() / 2);

        QPoint p{g.x() + (g.width() / 2), g.y() + (g.height() / 2)};

        QPoint globalCursorPos = QCursor::pos();
        auto mouseScreen = this->screen();
        QRect mouseScreenGeometry = mouseScreen->geometry();
        QPoint localCursorPos = globalCursorPos - mouseScreenGeometry.topLeft();

        auto x = localCursorPos.x() - x_offset;
        if(x < mouseScreenGeometry.left())
            x = mouseScreenGeometry.left();
        if((x + g.width()) > mouseScreenGeometry.right())
            x = mouseScreenGeometry.right() - g.width();

        auto y = localCursorPos.y() - y_offset;
        if(y < mouseScreenGeometry.top())
            y = mouseScreenGeometry.top();
        if((y + g.height()) > mouseScreenGeometry.bottom())
            y = mouseScreenGeometry.bottom() - g.height();

        last_position.setX(x);
        last_position.setY(y);
    }

    move(last_position);
}

void MainWindow::cycle_writing_system(const QChar& vowel)
{
    if(last_cycle_vowel != vowel)
        sections[vowel].first = 0;

    if(last_highlighted_label)
        last_highlighted_label->setStyleSheet("");

    last_highlighted_label = reinterpret_cast<QLabel*>(sections[vowel].second[sections[vowel].first++]);
    assert(last_highlighted_label);
    last_highlighted_label->setStyleSheet("color: #ffff00");

    ui->scrollArea->ensureWidgetVisible(last_highlighted_label);
    if(sections[vowel].first >= sections[vowel].second.length())
        sections[vowel].first = 0;

    last_cycle_vowel = vowel;
}

void MainWindow::rebuild_MRU()
{
    // clear the list
    QLayoutItem *child;
    while((child = ui->mruLayout->takeAt(0)) != nullptr)
    {
        delete child->widget();
        delete child;
    }

    ui->line->setVisible(!mru.isEmpty());

    for(int ndx = 0, col = 0;col < 11 && ndx < mru.length();)
    {
        auto sc = new SelectableChar(mru[ndx], this);
        auto col_span = static_cast<int>(sc->width() / sc->height());

        if((col + col_span) >= 11)
        {
            sc->deleteLater();

            // strip off all remaining MRU entries
            while(mru.length() < ndx)
                mru.removeAt(ndx);
            // TODO: save this change?
            break;
        }

        connect(sc, &SelectableChar::signal_selected, this, &MainWindow::slot_mru_selected);

        ui->mruLayout->addWidget(sc);
        col += col_span;

        ++ndx;
    }

    auto horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    ui->mruLayout->addItem(horizontalSpacer);
}

void MainWindow::showEvent(QShowEvent* /*event*/)
{
    position_window(double_clicked_to_open ? false : true);

    rebuild_MRU();

    double_clicked_to_open = false;
    active_modifiers.fill(false);

    show();
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    auto key = event->key();
    if(key == Qt::Key_Escape)
    {
        hide();
        if(settings_dlg)
            slot_settings_rejected();
        if(last_highlighted_label)
            last_highlighted_label->setStyleSheet("");
    }
    else if(key >= Qt::Key_A && key <= Qt::Key_Z)
    {
        auto offset = 'a' - Qt::Key_A;
        char index = static_cast<char>(key + offset);
        if(sections.contains(index))
            cycle_writing_system(index);
    }
}

void MainWindow::slot_icon_activated(QSystemTrayIcon::ActivationReason reason)
{
    switch(reason)
    {
        case QSystemTrayIcon::DoubleClick:
            double_clicked_to_open = true;
            show();
            break;

        case QSystemTrayIcon::Trigger:
        case QSystemTrayIcon::MiddleClick:
        default:
            break;
    }
}

void MainWindow::slot_char_selected(const QString seq)
{
    activate_sequence(seq);

    if(!mru.contains(seq))
        mru.prepend(seq);  // TODO: save this change?

    if(last_highlighted_label)
        last_highlighted_label->setStyleSheet("");
}

void MainWindow::slot_mru_selected(const QString seq)
{
    activate_sequence(seq);
}

void MainWindow::slot_menu_action(QAction* action)
{
    if(action == quit_action)
        QTimer::singleShot(50, this, &MainWindow::slot_quit);
}

void MainWindow::slot_quit()
{
    hide();
    if(settings_dlg)
        slot_settings_rejected();
    trayIcon->hide();
    qApp->quit();
}

void MainWindow::activate_sequence(const QString seq)
{
    auto clipboard = QGuiApplication::clipboard();
    clipboard->setText(seq);

    hide();
}

void MainWindow::map_sequence_key()
{
    active_modifiers.fill(false);
    needed_modifiers.clear();

#ifdef QT_WIN
    if(settings.use_windows_key)
        needed_modifiers.push_back(Modifier_Windows);
#endif

    auto keys = settings.sequence.toString().split("+");
    for(QString key : keys)
    {
        if(key.compare(QStringLiteral("Alt"), Qt::CaseInsensitive) == 0 && !settings.use_windows_key)
            needed_modifiers.push_back(Modifier_Alt);
        else if(key.compare(QStringLiteral("Ctrl"), Qt::CaseInsensitive) == 0 && !settings.use_windows_key)
            needed_modifiers.push_back(Modifier_Ctrl);
        else if(key.compare(QStringLiteral("Shift"), Qt::CaseInsensitive) == 0 && !settings.use_windows_key)
            needed_modifiers.push_back(Modifier_Shift);
        else
        {
            // this obviously doesn't encompass all possible key values

            QRegularExpression re1("^\\w$", QRegularExpression::CaseInsensitiveOption);
            auto m1 = re1.match(key);
            if(m1.hasMatch())
                activation_key = static_cast<unsigned long>(key.toUpper()[0].toLatin1());
            else
            {
                QRegularExpression re2("^F[1-9][1-2]?$");
                auto m2 = re2.match(key);
                if(m2.hasMatch())
                {
                    auto index = key.right(key.length() - 1).toInt();
#ifdef QT_WIN
                    activation_key = static_cast<unsigned long>(VK_F1 + (index - 1));
#endif
                }
            }
        }
    }
}

void MainWindow::slot_settings_accepted()
{
    settings_dlg->get_settings(settings);

    settings_dlg->hide();
    settings_dlg->deleteLater();
    settings_dlg = nullptr;

    save_settings();
}

void MainWindow::slot_settings_rejected()
{
    settings_dlg->hide();
    settings_dlg->deleteLater();
    settings_dlg = nullptr;
}

void MainWindow::slot_settings()
{
    QPoint globalCursorPos = QCursor::pos();
    auto mouseScreen = this->screen();
    QRect mouseScreenGeometry = mouseScreen->geometry();
    QPoint localCursorPos = globalCursorPos - mouseScreenGeometry.topLeft();

    settings_dlg = new SettingsDialog(settings);
    settings_dlg->move(localCursorPos);
    connect(settings_dlg, &QDialog::accepted, this, &MainWindow::slot_settings_accepted);
    connect(settings_dlg, &QDialog::rejected, this, &MainWindow::slot_settings_rejected);
    settings_dlg->show();
}

void MainWindow::process_show()
{
    show();
}

void MainWindow::build_tray_menu()
{
    if(trayIconMenu)
    {
        disconnect(trayIconMenu, &QMenu::triggered, this, &MainWindow::slot_menu_action);
        delete trayIconMenu;
    }

    quit_action = new QAction(tr("&Quit"), this);

    trayIconMenu = new QMenu(this);

    trayIconMenu->addAction(quit_action);

    trayIcon->setContextMenu(trayIconMenu);
    connect(trayIconMenu, &QMenu::triggered, this, &MainWindow::slot_menu_action);
}
