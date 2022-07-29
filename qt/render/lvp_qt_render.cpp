#include "lvp_qt_render.h"
#include <qdebug.h>

#pragma comment(lib,"lvp.lib")
#pragma comment(lib,"avutil")
#pragma comment(lib,"avcodec")
#pragma comment(lib,"avformat")
#pragma comment(lib,"SDL2")
#pragma comment(lib,"SDL2main")
#pragma comment(lib,"swresample")
//#pragma comment(lib,"swscalse")
#pragma comment(lib,"SoundTouchD_x64")


typedef struct {
	LVPEventControl *ctl;
	LVPLog *log;
	lvp_qt_render *render;
}LVPQTRender;

static int update_video(LVPEvent *ev, void *usrdata) {
	qDebug() << "UPDATE VIDEO";
	lvp_qt_render *r = (lvp_qt_render*)usrdata;
	
	return LVP_OK;
}


int module_init(LVPModule *m,LVPMap *options, LVPEventControl *ctl, LVPLog *log) {
	LVPQTRender *r = (LVPQTRender*)m->private_data;
	r->ctl = ctl;
	r->log = lvp_log_alloc(m->name);
	lvp_event_control_add_listener(ctl, LVP_EVENT_UPDATE_VIDEO, update_video, r);
	return LVP_OK;
}

static void module_close(LVPModule *m) {

}

static void custom_log(const char *log, void *usrdata) {
	qDebug() << log;
}

static const char *vs =
"attribute lowp vec4 posAttr;\n"
"attribute lowp vec2 colAttr;\n"
"varying lowp vec2 col;\n"
"void main(){\n"
"	col = colAttr;\n"
"	gl_Position = posAttr;\n"
"}\n"
;
static const char *fs =
"varying lowp vec2 col;\n"
"uniform sampler2D tex_y;\n"
"uniform sampler2D tex_u;\n"
"uniform sampler2D tex_v;\n"
"void main(){\n"
"	lowp vec3 yuv;\n"
"	lowp vec3 rgb;\n"
"	yuv.x = texture2D(tex_y,col).r;\n"
"	yuv.y = texture2D(tex_u,col).r-0.5;\n"
"	yuv.z = texture2D(tex_v,col).r-0.5;\n"
"	rgb = mat3(1,          1,              1,\n"
"           0,          -0.39465,       2.03211,\n"
"           1.13983,    -0.58060,       0) * yuv;\n"
"	gl_FragColor = vec4(rgb, 1);"
"}\n";

lvp_qt_render::lvp_qt_render()
{
	LVPCore *core = lvp_core_alloc();
	lvp_core_set_custom_log(core, custom_log, NULL);
	LVPQTRender *crender = new LVPQTRender();
	crender->render = this;
	lvp_core_register_dynamic_module(core,module_init, module_close, "LVP_QT_VIDEO_RENDER", LVP_MODULE_CORE | LVP_MODULE_RENDER, crender);
	lvp_core_set_option(core, "-video_render LVP_QT_VIDEO_RENDER -lvp_vcodec h264_cuvid");
	lvp_core_set_url(core, "c:/Ñ¸À×ÏÂÔØ/oupt/oupt.mkv");
	lvp_core_play(core);
}


lvp_qt_render::~lvp_qt_render()
{
}

void lvp_qt_render::lvp_qt_init()
{
}

void lvp_qt_render::lvp_qt_test()
{

}

void lvp_qt_render::initializeGL()
{
//	initialize();
}

void lvp_qt_render::paintGL()
{
//	render();
}

void lvp_qt_render::resizeGL(int w, int h)
{
//	glViewport(0, 0, width(), height());
}

