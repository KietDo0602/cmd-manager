#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <QApplication>
#include <QSettings>
#include <QString>
#include <QFont>
#include <QWidget>

class SettingsManager : public QObject {
    Q_OBJECT

public:
    enum Theme {
        Dark,
        Light,
        Contrast
    };

    static SettingsManager* instance();
    
    // General settings
    QString getDefaultDirectory() const;
    void setDefaultDirectory(const QString& dir);
    
    bool getAutoSave() const;
    void setAutoSave(bool enabled);
    
    // Appearance settings
    Theme getTheme() const;
    void setTheme(Theme theme);
    
    int getFontSize() const;
    void setFontSize(int size);
    
    QString getFontFamily() const;
    void setFontFamily(const QString& family);
    
    // Keyboard shortcuts
    QString getNewCommandShortcut() const;
    void setNewCommandShortcut(const QString& shortcut);
    
    QString getSaveCommandShortcut() const;
    void setSaveCommandShortcut(const QString& shortcut);
    
    QString getOpenCommandsShortcut() const;
    void setOpenCommandsShortcut(const QString& shortcut);

    QString getStartExecuteShortcut() const;
    void setStartExecuteShortcut(const QString& shortcut);

    void applyThemeToAllWindows();
    QString getCurrentThemeStyleSheet() const;

    static QString getThemeStyleSheet(Theme theme, int fontSize, const QString& fontFamily);

signals:
    void themeChanged(Theme theme);
    void fontChanged();
    void shortcutsChanged();

private:
    explicit SettingsManager(QObject* parent = nullptr);
    static SettingsManager* m_instance;
    QSettings* m_settings;
};

#endif // SETTINGSMANAGER_H
