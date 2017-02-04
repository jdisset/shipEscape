#include <QKeyEvent>
#include <QtGui/QOpenGLFunctions>
#include <QtGui/QWindow>

class QPainter;
class QOpenGLContext;
class QOpenGLPaintDevice;

class OpenGLWindow : public QWindow, protected QOpenGLFunctions {
	Q_OBJECT
 public:
	explicit OpenGLWindow(QWindow *parent = 0);
	~OpenGLWindow();

	virtual void render(QPainter *painter);
	virtual void render();

	virtual void initialize();

	void setAnimating(bool animating);

 public slots:
	void renderLater();
	void renderNow();

 protected:
	bool event(QEvent *event) override;

	void exposeEvent(QExposeEvent *event) override;

	virtual void handleKey(QKeyEvent *);

 private:
	bool m_animating;

	QOpenGLContext *m_context;
	QOpenGLPaintDevice *m_device;
};
