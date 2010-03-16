#include "ksane_viewer.h"

#include <KDebug>
#include <QApplication>

int main (int argc, char *argv[])
{
    QApplication app(argc, argv);

    if (argc != 2) {
        kDebug() << "An image filename is needed.";
        return 1;
    }
    QImage img(argv[1]);

    KSaneIface::KSaneViewer viewer;
    viewer.setQImage(&img);
    
    viewer.findSelections();

    viewer.show();

    viewer.zoom2Fit();
    
    return app.exec();
}
