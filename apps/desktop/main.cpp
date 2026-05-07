#include <QApplication>
#include <QStringList>

#include "AppBootstrap.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("web-IDE");
    app.setOrganizationName("web-IDE");

    QStringList arguments;
    for (int index = 0; index < argc; ++index) {
        arguments.append(QString::fromLocal8Bit(argv[index]));
    }

    webide::AppBootstrap bootstrap;
    return bootstrap.run(app, arguments);
}
