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
        Dark,           // Default: Dark Theme
        Light,
        Contrast
    };

    enum TerminalColorScheme {
        NeonGreen,      // Default: green on black
        Classic,        // White on black
        LightMode,      // Black on white
        Matrix,         // Dark blue background, cyan text
        Dracula,        // Purple/pink theme
        Monokai,        // Dark with orange/yellow
        Nord,           // Cool blues and grays
        SolarizedDark,  // Muted yellows on dark
        GruvboxDark,    // Warm retro colors
        OneDark         // Atom-inspired dark theme
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

    bool getShowCommandLabel() const;
    void setShowCommandLabel(bool show);

    bool getInstantRunFromMenu() const;
    void setInstantRunFromMenu(bool instant);

    TerminalColorScheme getTerminalColorScheme() const;
    void setTerminalColorScheme(TerminalColorScheme scheme);
    
    QString getTerminalFontFamily() const;
    void setTerminalFontFamily(const QString& family);
    
    int getTerminalFontSize() const;
    void setTerminalFontSize(int size);
    
    static QPair<QColor, QColor> getTerminalColors(TerminalColorScheme scheme);

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
