#include "utils.h"

namespace Utils {
    // Play Sound Function for multiple Operating Systems
    void playNotificationSound() {
        // Method 1: Qt beep (may not work on Linux)
        QApplication::beep();
        
        // Method 2: Try Linux sound commands
        #ifdef Q_OS_LINUX
            // Try paplay (PulseAudio)
            if (QProcess::execute("paplay", QStringList() << "/usr/share/sounds/freedesktop/stereo/complete.oga") == 0) {
                return;
            }
            
            // Try aplay (ALSA)
            if (QProcess::execute("aplay", QStringList() << "/usr/share/sounds/freedesktop/stereo/complete.oga") == 0) {
                return;
            }
            
            // Try canberra-gtk-play
            if (QProcess::execute("canberra-gtk-play", QStringList() << "-i" << "complete") == 0) {
                return;
            }
            
            // Fallback: terminal bell
            QProcess::execute("echo", QStringList() << "-e" << "\\a");
        #endif
        
        #ifdef Q_OS_WIN
            // Windows beep
            QProcess::execute("rundll32", QStringList() << "user32.dll,MessageBeep");
        #endif
        
        #ifdef Q_OS_MAC
            // macOS sound
            QProcess::execute("afplay", QStringList() << "/System/Library/Sounds/Glass.aiff");
        #endif
    }
}

