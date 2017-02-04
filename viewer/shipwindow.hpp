#ifndef SHIPWINDOW_HPP
#define SHIPWINDOW_HPP
#include <QKeyEvent>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QVector2D>
#include <chrono>
#include <deque>
#include <functional>
#include <memory>
#include <random>
#include <unordered_set>
#include "extern.h"
#include "openglwindow.h"
#include "renderquad.hpp"

struct Particle {
	Particle(QVector2D p, QVector2D v,
	         std::chrono::time_point<std::chrono::high_resolution_clock> T)
	    : position(p), velocity(v), t(T){};
	QVector2D position;
	QVector2D velocity;
	std::chrono::time_point<std::chrono::high_resolution_clock> t;
};

template <typename World> class ShipWindow : public OpenGLWindow {
	int screenCoef = 1.0;

	// Visual elements
	std::unique_ptr<QOpenGLTexture> shipTex;
	std::unique_ptr<QOpenGLTexture> bordTex;
	RenderQuad spriteRenderer, circleRenderer, plainRenderer, particleRenderer;
	std::deque<Particle> particles;

	// Stats
	std::chrono::time_point<std::chrono::high_resolution_clock> t0;
	std::default_random_engine generator;
	int frame = 0;
	World &world;
	std::function<void()> updateLambda;
	std::unordered_set<int> keyMap;

 public:
	bool keyboardEnabled = true;
	ShipWindow(World &w, std::function<void()> upl) : world(w), updateLambda(upl) {}
	~ShipWindow() {}
	void initialize() {
		GL = QOpenGLContext::currentContext()->functions();
		GL->initializeOpenGLFunctions();
		generator.seed(t0.time_since_epoch().count());
		spriteRenderer.load(":/shaders/sprite.vert", ":/shaders/sprite.frag");
		circleRenderer.load(":/shaders/sprite.vert", ":/shaders/circle.frag");
		plainRenderer.load(":/shaders/sprite.vert", ":/shaders/plain.frag");
		particleRenderer.load(":/shaders/sprite.vert", ":/shaders/thrust.frag");
		shipTex = std::unique_ptr<QOpenGLTexture>(
		    new QOpenGLTexture(QImage(":/images/ship.png").mirrored()));
		shipTex->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
		shipTex->setMagnificationFilter(QOpenGLTexture::Linear);
		t0 = std::chrono::high_resolution_clock::now();
	}

	void processEvents() {
		if (keyboardEnabled) {
			if (keyMap.count(Qt::Key_Space)) world.ships.at(0).thrust(world.dt);
			if (keyMap.count(Qt::Key_H)) world.ships.at(0).rotate(1.0, world.dt * 8.0);
			if (keyMap.count(Qt::Key_L)) world.ships.at(0).rotate(-1.0, world.dt * 8.0);
		}
		for (auto &s : world.ships) {
			if (s.thrusting) {
				s.thrusting = false;
				const unsigned int nbP = 5;
				const double delta = 0.1;
				const double baseSpeed = 40.0;
				const double speedVariation = 0.5;
				uniform_real_distribution<double> distD(-delta, delta);
				uniform_real_distribution<double> dist(-1.0, 1.0);
				for (unsigned int i = 0; i < nbP; ++i) {
					QVector2D shipPosition(world.ships.at(0).position.x,
					                       world.ships.at(0).position.y);
					auto direction = world.ships.at(0).orientation;
					direction.rotate(distD(generator));
					QVector2D dir(-direction.x, -direction.y);
					particles.push_back(Particle(
					    shipPosition + dir * (3.0 + dist(generator) * 1.0),
					    dir * (baseSpeed + dist(generator) * speedVariation * baseSpeed), t0));
				}
			}
		}
	}

	void handleKey(QKeyEvent *event) {
		if (event->type() == QEvent::KeyPress)
			keyMap.insert(event->key());
		else if (event->type() == QEvent::KeyRelease)
			keyMap.erase(event->key());
	}

	void render() {
		auto t1 = std::chrono::high_resolution_clock::now();
		auto dur = std::chrono::duration<double>(t1 - t0);
		t0 = std::chrono::high_resolution_clock::now();
		updateLambda();
		updateParticles(dur.count());
		processEvents();
		clear();
		QMatrix4x4 model;
		QMatrix4x4 view;

		// we center the camera around the first ship
		QVector2D shipPosition(world.ships.at(0).position.x, world.ships.at(0).position.y);
		double pixelRatio = world.MAXH / (double)height();
		view.ortho(shipPosition.x() - (width() * 0.5 * pixelRatio),
		           shipPosition.x() + (width() * 0.5 * pixelRatio),
		           shipPosition.y() - (world.MAXH * 0.5),
		           shipPosition.y() + (world.MAXH * 0.5), 1, -1);
		int currentGridCell = world.getGridPosition(shipPosition.y());
		// particles
		double pIndex = 0.0;
		for (auto &p : particles) {
			QMatrix4x4 pModel;
			pModel.translate(p.position);
			pModel.translate(0, 0, -pIndex++ / (double)particles.size());
			pModel.scale(3, 3);
			auto v = chrono::duration_cast<chrono::milliseconds>(t0 - p.t);
			QColor cm =
			    QColor::fromHsvF(mix(0.55, 0.8, min((double)v.count() / 700.0, 1.0)), 0.9, 0.8);
			QVector3D color(cm.redF(), cm.greenF(), cm.blueF());
			particleRenderer.draw(pModel, view, color);
		}

		// we draw all the ships
		for (auto &s : world.ships) {
			model = QMatrix4x4();
			model.translate(s.position.x, s.position.y, -1);
			model.rotate(s.getAngle() * (180.0 / M_PI), 0, 0, 1);
			model.scale(s.dimensions.x, s.dimensions.y);
			spriteRenderer.draw(shipTex->textureId(), model, view, QVector3D(0.0, 0.0, 0));
		}

		// laser dots
		for (auto &s : world.ships) {
			QVector2D sPosition(s.position.x, s.position.y);
			double teta = M_PI;
			double maxDist = world.MAXH;
			auto dir = s.orientation;
			dir.rotate(-teta / 2.0);
			double N = 100;
			for (int i = 0; i < N; ++i) {
				QMatrix4x4 targetModel;
				dir.rotate(teta / N);
				dir.normalize();
				QVector2D ori(dir.x, dir.y);
				double dist = world.normalizedDistRay(dir, maxDist, s);
				targetModel.translate(sPosition + ori * dist);
				targetModel.translate(0, 0, -(float)i / (N + 1.0));
				targetModel.scale(1, 1);
				double v = pow(dist / maxDist, 2);
				QColor cm = QColor::fromHsvF(mix(0.05, 0.95, v), 0.85, 1);
				QVector3D color(cm.redF(), cm.greenF(), cm.blueF());
				circleRenderer.draw(targetModel, view, color);
			}
		}
		++frame;
	}

	void updateParticles(double dt) {
		const double destProba = 0.05;
		std::uniform_real_distribution<double> dist(0.0, 1.0);
		auto new_end =
		    std::remove_if(particles.begin(), particles.end(),
		                   [&](const Particle &) { return dist(generator) < destProba; });
		particles.erase(new_end, particles.end());
		for (auto &p : particles) {
			p.position += p.velocity * dt;
		}
	}
	double mix(double a, double b, double c) { return a * c + b * (1.0 - c); }

	void clear() {
		GL->glDepthMask(true);
		GL->glClearColor(0.0, 0.0, 0.0, 1.0);
		GL->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		GL->glEnable(GL_DEPTH_TEST);
		GL->glEnable(GL_BLEND);
		GL->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
};
#endif
