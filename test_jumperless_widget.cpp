/*
 * Simple test to verify Jumperless configuration widget integration
 * 
 * This file can be used to manually test the widget functionality
 * during development.
 */

#include <QApplication>
#include <QMainWindow>
#include <QDebug>

#include "pv/widgets/jumperlessconfig.hpp"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Create a simple test window
    QMainWindow window;
    window.setWindowTitle("Jumperless Widget Test");
    window.resize(800, 100);
    
    // Create the Jumperless config widget
    pv::widgets::JumperlessConfig *config_widget = new pv::widgets::JumperlessConfig(&window);
    
    // Set it as the central widget for testing
    window.setCentralWidget(config_widget);
    
    // For testing, make it visible even without a device
    config_widget->setVisible(true);
    
    // Connect signals for testing
    QObject::connect(config_widget, &pv::widgets::JumperlessConfig::config_changed, [&]() {
        qDebug() << "Jumperless configuration changed!";
    });
    
    qDebug() << "Jumperless widget test started";
    qDebug() << "Widget should be visible with all controls";
    qDebug() << "Try changing the dropdown values and check console output";
    
    window.show();
    
    return app.exec();
}

/*
 * To compile and run this test:
 * 
 * 1. Make sure Qt development packages are installed
 * 2. Compile with: 
 *    g++ -fPIC -I. test_jumperless_widget.cpp pv/widgets/jumperlessconfig.cpp \
 *        $(pkg-config --cflags --libs Qt5Widgets) -o test_jumperless_widget
 * 3. Run: ./test_jumperless_widget
 * 
 * This will show a window with the Jumperless configuration controls
 * that you can interact with to verify the UI works correctly.
 */ 