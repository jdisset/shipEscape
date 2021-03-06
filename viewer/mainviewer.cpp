
#include <QtCore/qmath.h>
#include <QtGui/QGuiApplication>
#include <QtGui/QMatrix4x4>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QScreen>
#include "../ship.hpp"
#include "shipwindow.hpp"

int main(int argc, char **argv) {
	ShipEscape::World w;
	QSurfaceFormat f;
	f.setSamples(8);
	QGuiApplication app(argc, argv);
	ShipWindow<decltype(w)> window(w, [&]() {
		if (w.collided || w.countdown <= 0) {
			std::cerr << "SCORE = " << w.ships.at(0).position.y << std::endl;
			exit(0);
		}
		w.update();
	});
	window.setFormat(f);
	window.resize(800, 800);
	window.show();

	window.setAnimating(true);

	return app.exec();
}
