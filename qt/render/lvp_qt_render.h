#pragma once

#include <qopenglwidget.h>
#include <qopenglfunctions.h>
#include <QtGui/qwindow.h>
#include <QtGui/qopenglfunctions.h>

QT_BEGIN_NAMESPACE
class QPainter;
class QOpenGLContext;
class QOpenGLPaintDevice;
QT_END_NAMESPACE

#include <QtGui/qopenglshaderprogram.h>
#include <qmutex.h>

extern "C" {
#include "../../src/core/lvp.h"
#include "../../src/core/lvp_core.h"
}


class lvp_qt_render:public QOpenGLWidget
{
	Q_OBJECT
public:
	lvp_qt_render();
	~lvp_qt_render();
static void lvp_qt_init();
static void lvp_qt_test();

protected:
	void initializeGL();
	void paintGL();
	void resizeGL(int w, int h);
};

