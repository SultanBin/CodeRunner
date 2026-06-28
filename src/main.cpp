#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application properties
    app.setApplicationName("CodeRunner");
    app.setApplicationVersion("1.0.0");
    app.setApplicationDisplayName("CodeRunner - C/C++ IDE");
    
    // Create and show main window
    MainWindow window;
    window.show();
    
    return app.exec();
}
