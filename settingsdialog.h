#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QComboBox>
#include <QSpinBox>
#include <QFontComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QKeySequenceEdit>
#include "settingsmanager.h"

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);

private slots:
    void onThemeChanged();
    void onFontSizeChanged();
    void onFontFamilyChanged();
    void onApplyClicked();
    void onOkClicked();
    void onCancelClicked();
    void onRestoreDefaultsClicked();

private:
    void setupGeneralTab();
    void setupAppearanceTab();
    void setupKeyboardTab();
    void loadSettings();
    void applySettings();
    void applyTheme(SettingsManager::Theme theme);
    
    QTabWidget* m_tabWidget;
    
    // General tab
    QWidget* m_generalTab;
    QLineEdit* m_defaultDirEdit;
    QPushButton* m_browseDirButton;
    QCheckBox* m_autoSaveCheck;
    
    // Appearance tab
    QWidget* m_appearanceTab;
    QComboBox* m_themeCombo;
    QSpinBox* m_fontSizeSpin;
    QFontComboBox* m_fontFamilyCombo;
    QLabel* m_previewLabel;
    
    // Keyboard tab
    QWidget* m_keyboardTab;
    QKeySequenceEdit* m_newCommandEdit;
    QKeySequenceEdit* m_saveCommandEdit;
    QKeySequenceEdit* m_openCommandsEdit;
    
    // Buttons
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;
    QPushButton* m_applyButton;
    QPushButton* m_restoreDefaultsButton;
    
    SettingsManager* m_settings;
    QKeySequenceEdit* m_startExecuteEdit;
};

#endif // SETTINGSDIALOG_H
