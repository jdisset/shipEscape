
#include <QtCore/qmath.h>
#include <QtGui/QGuiApplication>
#include <QtGui/QMatrix4x4>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QScreen>
#include "../ship.hpp"
#include "shipwindow.hpp"

int main(int argc, char **argv) {
	QSurfaceFormat f;
	f.setProfile(QSurfaceFormat::CoreProfile);
	f.setVersion(4, 1);
	f.setSamples(8);

	ShipEscape::World w;
	QGuiApplication app(argc, argv);
	ShipWindow<decltype(w)> window(w, [&]() { w.update(); });
	window.setFormat(f);
	window.resize(500, 800);
	window.show();

	window.setAnimating(true);

	return app.exec();
}
