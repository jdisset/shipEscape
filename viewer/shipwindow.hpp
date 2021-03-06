#ifndef SHIPWINDOW_HPP
#define SHIPWINDOW_HPP
#include <QKeyEvent>
#include <QOpenGLFunctions>
#include <QOpenGLPaintDevice>
#include <QOpenGLTexture>
#include <QPainter>
#include <QVector2D>
#include <chrono>
#include <deque>
#include <functional>
#include <memory>
#include <random>
#include <sstream>
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

	QVector2D anchor, prevAnchor;

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
		particleRenderer.load(":/shaders/sprite.vert", ":/shaders/circle.frag");
		shipTex = std::unique_ptr<QOpenGLTexture>(
		    new QOpenGLTexture(QImage(":/images/ship.png").mirrored()));
		shipTex->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
		shipTex->setMagnificationFilter(QOpenGLTexture::Linear);
		t0 = std::chrono::high_resolution_clock::now();
		anchor = QVector2D(world.ships[0].position.x, world.ships[0].position.y);
		prevAnchor = anchor;
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
				const unsigned int nbP = 10;
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

	double prevL = 0;

	void render(QPainter *painter) {
		const qreal retinaScale = devicePixelRatio();
		GL->glViewport(0, 0, width() * retinaScale, height() * retinaScale);
		auto t1 = std::chrono::high_resolution_clock::now();
		auto dur = std::chrono::duration<double>(t1 - t0);
		t0 = std::chrono::high_resolution_clock::now();
		updateLambda();
		updateParticles(dur.count());
		processEvents();
		clear();
		// painter->begin();
		QMatrix4x4 model;
		QMatrix4x4 view;

		// we center the camera around the first ship
		QVector2D shipPosition(world.ships.at(0).position.x, world.ships.at(0).position.y);
		double pixelRatio = world.MAXH / (double)height();
		double scale = 1.0;

		// smooth camera
		const double K = 200;
		const double C = 0.01;
		auto anchorDir = shipPosition - anchor;
		auto v = (anchor - prevAnchor) / world.dt;
		auto L = anchorDir.length();
		auto speed = (L - prevL) / world.dt;
		prevL = L;
		anchor += (v + (anchorDir * K - (speed * C) * anchorDir) * world.dt) * world.dt;
		prevAnchor = anchor;

		view.ortho((anchor.x() - (width() * scale * 0.5 * pixelRatio)),
		           (anchor.x() + (width() * scale * 0.5 * pixelRatio)),
		           (anchor.y() - (world.MAXH * 0.5 * scale)),
		           (anchor.y() + (world.MAXH * 0.5 * scale)), 1, -1);
		int currentGridCell = world.getGridPosition(shipPosition.y());

		// ship lights
		QVector4D color1(0.1, 0.24, 0.48, 0.2);
		QVector4D color2(0.05, 0.12, 0.24, 0.0);
		for (auto &s : world.ships) {
			QVector2D sPosition(s.position.x, s.position.y);
			double teta = M_PI * 2.0;
			double maxDist = world.MAXH;
			auto dir = s.orientation;
			dir.rotate(-teta / 2.0);
			double N = 1500;
			for (int i = 0; i < N; ++i) {
				QMatrix4x4 targetModel;
				dir.rotate(teta / N);
				dir.normalize();
				QVector2D ori(dir.x, dir.y);
				double angle = atan2(-dir.y, dir.x);
				double dist = world.normalizedDistRay(dir, maxDist, s);
				targetModel.translate(sPosition + ori * dist * 0.5);
				targetModel.rotate((angle + M_PI * 0.5) * (180.0 / M_PI), QVector3D(0, 0, -1));
				targetModel.scale(0.25, 0.5 * dist);
				double v = 1.0 - dist / maxDist;
				plainRenderer.draw(targetModel, view, color1, color2);
			}
		}

		// particles
		double pIndex = 0.0;
		for (auto &p : particles) {
			QMatrix4x4 pModel;
			pModel.translate(p.position);
			pModel.translate(0, 0, -pIndex++ / (double)particles.size());
			pModel.scale(0.4, 0.4);
			auto v = chrono::duration_cast<chrono::milliseconds>(t0 - p.t);
			QColor cm =
			    QColor::fromHsvF(mix(0.55, 0.8, min((double)v.count() / 700.0, 1.0)), 1.0, 1);
			QVector4D color(cm.redF(), cm.greenF(), cm.blueF(), 1.0);
			QVector4D color2 = color;
			color2.setW(0.0);
			particleRenderer.draw(pModel, view, color, color2);
		}
		// we draw all the ships
		for (auto &s : world.ships) {
			model = QMatrix4x4();
			model.translate(s.position.x, s.position.y, -1);
			model.rotate(s.getAngle() * (180.0 / M_PI), 0, 0, 1);
			model.scale(s.dimensions.x, s.dimensions.y);
			spriteRenderer.draw(shipTex->textureId(), model, view, QVector3D(0.0, 0.0, 0));
		}

		// obstacles
		const int viewField = 10;
		for (int i = currentGridCell - viewField; i < currentGridCell + viewField; ++i) {
			if (world.obstacles.count(i)) {
				auto &om = world.obstacles[i];
				for (auto &o : om) {
					QMatrix4x4 cModel;
					cModel.translate(o.center.x, o.center.y);
					cModel.scale(o.radius, o.radius);
					QVector4D color1(1, 0.7, 0.5, 1.0);
					QVector4D color2(1, 0.9, 0.6, 1.0);
					circleRenderer.draw(cModel, view, color1, color2);
				}
			}
		}
		// walls
		const double wallWidth = 50;
		const QVector4D wColor1(.07, .3, .48, 1.0);
		const QVector4D wColor2(.12, .5, .34, 1.0);
		{
			QMatrix4x4 wModel;
			wModel.translate(-wallWidth, shipPosition.y());
			wModel.scale(wallWidth, world.MAXH * 2);
			plainRenderer.draw(wModel, view, wColor1, wColor2);
		}
		{
			QMatrix4x4 wModel;
			wModel.translate(world.W + wallWidth, shipPosition.y());
			wModel.scale(wallWidth, world.MAXH * 2);
			plainRenderer.draw(wModel, view, wColor2, wColor1);
		}

		// doors
		const double doorThickness = 0.3;
		const QVector4D dColor1(.97, .0, .32, 1.0);
		const QVector4D dColor2(.98, .8, .3, 1.0);
		{
			// closed one
			QMatrix4x4 wModel;
			wModel.translate(world.W * 0.5, world.prevReset);
			wModel.scale(world.W * 0.5, doorThickness * 2.0);
			plainRenderer.draw(wModel, view, dColor2, dColor2);
		}

		{
			// opened one
			double apertureSize = world.W * world.countdown / world.maxCountdown;
			double closedSize = (world.W - apertureSize) * 0.5;
			// left
			{
				QMatrix4x4 wModel;
				wModel.translate(closedSize * 0.5, world.nextReset);
				wModel.scale(closedSize * 0.5, doorThickness);
				plainRenderer.draw(wModel, view, dColor2, dColor1);
			}
			// right
			{
				QMatrix4x4 wModel;
				wModel.translate(world.W - closedSize * 0.5, world.nextReset);
				wModel.scale(closedSize * 0.5, doorThickness);
				plainRenderer.draw(wModel, view, dColor1, dColor2);
			}
		}

		QFont font("arial", 40);
		painter->setFont(font);
		painter->setPen(QColor::fromHsvF(0, 0, 1));
		QString score = QString::number((int)shipPosition.y());
		painter->drawText(QRect((width() * retinaScale * 0.5 - 80) - anchor.x() * retinaScale,
		                        height() * retinaScale * 0.9 - 80, 160, 160),
		                  Qt::AlignCenter, score);
		painter->end();
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
		GL->glClearColor(0.05, 0.12, 0.24, 1.0);
		GL->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		GL->glEnable(GL_DEPTH_TEST);
		GL->glEnable(GL_BLEND);
		GL->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
};
#endif
