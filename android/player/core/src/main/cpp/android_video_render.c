//
// Created by fgodt on 2022/7/20.
//

#include "../../../../../../src/core/lvp_module.h"
#include "../../../../../../src/core/lvp.h"
#include "../../../../../../src/core/lvp_mutex.h"
#include "../../../../../../src/core/lvp_thread.h"
#include "../../../../../../src/core/lvp_events.h"
#include <libavcodec/avcodec.h>
#include <GLES/gl.h>
#include <EGL/egl.h>
#include <android/native_window.h>
#include <GLES3/gl3.h>
#include <android/native_window_jni.h>


typedef struct LVPAndroidVideoRender{
    LVPLog  *log;
    LVPEventControl *ctl;
    lvp_mutex mutex;
    int egl_inited;
    int gles_inited;


    int media_width;
    int media_height;
    int media_format;

    //egl context
    EGLDisplay eglDisplay;
    EGLConfig eglConfig;
    EGLContext eglContext;
    EGLSurface eglSurface;
    ANativeWindow *nativeWindow;
    int eglSurfaceWidth;
    int eglSurfaceHeight;
    int need_reset_surface;

    //opengl context
    GLuint pg;
    GLuint vao;
    GLuint vbo;
    GLuint yTexture;
    GLuint uTexture;
    GLuint vTexture;
    GLint yTextureLoc;

    //jni
    JNIEnv *env;
}LVPAndroidVideoRender;

typedef struct Vertex{
    GLfloat pos[3];
    GLfloat tex_cord[2];
}Vertex;

const Vertex square_vertex[6]  = {
        {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
        {{-1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
        {{1.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
        {{-1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
        {{1.0f, 1.0f, 0.0f},{1.0f, 1.0f}},
        {{1.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
};


/*** EGL operation ***/

static int egl_reset_surface(LVPAndroidVideoRender *r){
    r->eglSurface = eglCreateWindowSurface(r->eglDisplay, r->eglConfig, r->nativeWindow, 0);
    if(r->eglSurface == EGL_NO_SURFACE){
        EGLint code = eglGetError();
        lvp_error(r->log, "EGL NO SURFACE code: %08x", code);
        return  LVP_E_FATAL_ERROR;
    }
    if(EGL_FALSE == eglQuerySurface(r->eglDisplay, r->eglSurface, EGL_WIDTH, &r->eglSurfaceWidth)){
        EGLint code = eglGetError();
        lvp_error(r->log, "eglQuerySurface error code: %08x", code);
        return LVP_E_FATAL_ERROR;
    }
    eglQuerySurface(r->eglDisplay, r->eglSurface, EGL_HEIGHT, &r->eglSurfaceHeight);
    return LVP_OK;
}

static int egl_init(LVPAndroidVideoRender *r){
    r->eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    if(r->eglDisplay == EGL_NO_DISPLAY){
        lvp_error(r->log, "eglGetDisplay error", NULL);
        return LVP_E_FATAL_ERROR;
    }

    if(EGL_TRUE != eglInitialize(r->eglDisplay, 0, 0)){
        lvp_error(r->log, "eglInitialize error", NULL);
        return LVP_E_FATAL_ERROR;
    }

    EGLint configNum;
    EGLint configSpec[] = {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_NONE
    };

    if(EGL_TRUE != eglChooseConfig(r->eglDisplay, configSpec, &r->eglConfig, 1, &configNum)){
        lvp_error(r->log, "eglChooseConfig error", NULL);
        return LVP_E_FATAL_ERROR;
    }
    return  LVP_OK;
}

static int egl_init_context(LVPAndroidVideoRender *r){
    const EGLint attribList[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE
    };

    if(r->eglContext == NULL) {
        r->eglContext = eglCreateContext(r->eglDisplay, r->eglConfig, NULL, attribList);
        if (EGL_NO_CONTEXT == r->eglContext) {
            EGLint code = eglGetError();
            lvp_error(r->log, "eglCreateContext error code: %d", code);
            return LVP_E_FATAL_ERROR;
        }
    }
    if(EGL_FALSE == eglMakeCurrent(r->eglDisplay, r->eglSurface, r->eglSurface, r->eglContext)){
        EGLint code = eglGetError();
        lvp_error(r->log, "eglMakeCurrent code: %d", code);
        return LVP_E_FATAL_ERROR;
    }

    lvp_debug(r->log, "egl init context done", NULL);
    return  LVP_OK;
}

static int egl_swap(LVPAndroidVideoRender *r){
    EGLBoolean ret = eglSwapBuffers(r->eglDisplay, r->eglSurface);
    if(!ret){
        EGLint code = eglGetError();
        lvp_error(r->log, "swap buffers error code: %08x", code);
        return LVP_E_FATAL_ERROR;
    }
    return LVP_OK;
}

/*** OPENGL operation ***/

const char *vssrc = "attribute vec4 pos;\n"
                    "attribute vec2 texCoord;\n"
                    "varying vec2 vTexCoord;\n"
                    "void main() {\n"
                    "    gl_Position = pos;\n"
                    "    vTexCoord = texCoord;\n"
                    "}\n";

const char *fssrc = "precision mediump float;\n"
                    "varying vec2 vTexCoord;\n"
                    "uniform sampler2D ytex;\n"
                    "uniform sampler2D utex;\n"
                    "uniform sampler2D vtex;\n"
                    "void main() {\n"
                    "   vec4 color  = texture2D(ytex, vec2(vTexCoord.x, 1.0 - vTexCoord.y)) ;\n"
                    //"    vec3 color = vec3(vTexCoord, 1.0f);\n"
                    //"    gl_FragColor = vec4(color.rgb, 1.0f);\n"
                    "    gl_FragColor = color;\n"
                    "}\n";

static GLuint gl_create_shader(LVPLog  *log,GLenum shader_type, const char* src){
    GLuint shader = glCreateShader(shader_type);
    if (!shader) {
        GLenum code = glGetError();
        lvp_error(log, "lvp create fragment shader error code: %08x", code);
        return LVP_E_FATAL_ERROR;
        return 0;
    }
    glShaderSource(shader, 1, &src, NULL);

    GLint compiled = GL_FALSE;
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLint infoLogLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLen);
        if (infoLogLen > 0) {
            GLchar* infoLog = (GLchar*)malloc(infoLogLen);
            if (infoLog) {
                glGetShaderInfoLog(shader, infoLogLen, NULL, infoLog);
                lvp_error(log, "Could not compile %s shader:\n%s\n",
                      shader_type == GL_VERTEX_SHADER ? "vertex" : "fragment",
                      infoLog);
                free(infoLog);
            }
        }
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

static int gl_create_program(LVPAndroidVideoRender *r){
    int ret = LVP_OK;
    GLuint fs, vs = 0;
    fs = gl_create_shader(r->log, GL_FRAGMENT_SHADER, fssrc);
    if(fs == 0){
        ret = LVP_E_FATAL_ERROR;
        goto exit;
    }

    vs = gl_create_shader(r->log, GL_VERTEX_SHADER, vssrc);
    if(vs == 0){
        ret = LVP_E_FATAL_ERROR;
        goto exit;
    }

    GLuint program = glCreateProgram();
    if (!program) {
        GLenum  code = glGetError();
        lvp_error(r->log, "glCreateProgram error code:%08x", code);
        ret = LVP_E_FATAL_ERROR;
        goto exit;
    }
    glAttachShader(program, vs);
    glAttachShader(program, fs);

    GLint linked;
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
        GLenum  code = glGetError();
        lvp_error(r->log, "glLinkProgram error code:%08x", code);
        GLint infoLogLen = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLen);
        if (infoLogLen) {
            GLchar* infoLog = (GLchar*)malloc(infoLogLen);
            if (infoLog) {
                glGetProgramInfoLog(program, infoLogLen, NULL, infoLog);
                lvp_error(r->log, "glLinkProgram error \n%s\n", code);
                free(infoLog);
            }
        }
        glDeleteProgram(program);
        program = 0;
    }

    r->pg = program;

exit:
    if(vs)
        glDeleteShader(vs);
    if(fs)
        glDeleteShader(fs);
    return ret;
}

static void gl_create_texture(GLuint *tex){
    glGenTextures(1, tex);
    glBindTexture(GL_TEXTURE_2D,*tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

static void checkGL(LVPLog *log){
    GLenum code = glGetError();
    if(code != 0){
        lvp_debug(log, "gl get error code:%08x", code);
    }
}

static void gl_set_view_port(LVPAndroidVideoRender *r){
    glViewport(0, 0, r->eglSurfaceWidth, r->eglSurfaceHeight);
}



static int gl_init(LVPAndroidVideoRender *r){
    glGenVertexArrays(1, &r->vao);
    glGenBuffers(1, &r->vbo);

    glBindVertexArray(r->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(square_vertex), square_vertex, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    int ret = gl_create_program(r);
    if(ret != LVP_OK){
        lvp_error(r->log, "gl create program error", NULL);
        return ret;
    }

    checkGL(r->log);


    gl_set_view_port(r);

    gl_create_texture(&r->yTexture);
    gl_create_texture(&r->uTexture);
    gl_create_texture(&r->vTexture);

    r->yTextureLoc = glGetUniformLocation(r->pg, "ytex");
    lvp_debug(r->log, "y texture loc: %d", r->yTextureLoc);

    lvp_debug(r->log, "init opengl done", NULL);

    return  LVP_OK;
}

static int init_graphics(LVPAndroidVideoRender *r){
    int ret = egl_init(r);
    if(ret != LVP_OK){
        lvp_error(r->log, "egl init surface error", NULL);
        return LVP_E_FATAL_ERROR;
    }


    ret = egl_init_context(r);
    if(ret != LVP_OK){
        lvp_error(r->log, "egl init context error", NULL);
        return LVP_E_FATAL_ERROR;
    }

    ret = gl_init(r);
    if(ret != LVP_OK){
        lvp_error(r->log, "gl init error", NULL);
        return LVP_E_FATAL_ERROR;
    }
    return  ret;
}


static int handle_update_video(LVPEvent *e,void*data){
    LVPAndroidVideoRender  *r = (LVPAndroidVideoRender*)data;

    //surface may changed or destroyed
    if(!r->nativeWindow){
        return LVP_OK;
    }

    if(!r->egl_inited){
        r->egl_inited = 1;
        int ret = egl_init(r);
        if(ret != LVP_OK){
            lvp_error(r->log, "egl init surface error", NULL);
            return LVP_E_FATAL_ERROR;
        }
    }

    if(r->need_reset_surface){
        r->need_reset_surface = 0;
        int ret = egl_reset_surface(r);
        if(ret != LVP_OK){
            lvp_error(r->log, "egl reset surface error", NULL);
            return LVP_E_FATAL_ERROR;
        }
        ret = egl_init_context(r);
        if(ret != LVP_OK){
            lvp_error(r->log, "egl init context error", NULL);
            return LVP_E_FATAL_ERROR;
        }
    }

    if(!r->gles_inited){
        r->gles_inited = 1;
        int ret = gl_init(r);
        if(ret != LVP_OK){
            lvp_error(r->log, "gl init error", NULL);
            return LVP_E_FATAL_ERROR;
        }
    }

    EGLContext context = eglGetCurrentContext();
    if(context != r->eglContext){
        lvp_waring(r->log, "current context not render egl context", NULL);
    }

    AVFrame  *frame = (AVFrame*)e->data;



    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 1.0f,1.0f);

    glUseProgram(r->pg);
    glBindVertexArray(r->vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, r->yTexture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, frame->width, frame->height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, frame->data[0]);
    glUniform1i(r->yTextureLoc, 0);


    glDrawArrays(GL_TRIANGLES, 0, 6);
    checkGL(r->log);
    egl_swap(r);
    return LVP_OK;
}


static int handle_surface_created(LVPEvent* e, void *data){
    LVPAndroidVideoRender *r = data;
    if(!r->env){
        lvp_error(r->log,"JNIEnv not setting", NULL);
        return LVP_E_USE_NULL;
    }
    ANativeWindow* nativeWindow = ANativeWindow_fromSurface(r->env, e->data);
    r->nativeWindow = nativeWindow;
    r->need_reset_surface = 1;
    return LVP_OK;
}

static int handle_surface_destroyed(LVPEvent* e, void *data){
    LVPAndroidVideoRender *r = data;
    r->nativeWindow = NULL;
    return LVP_OK;
}

static int module_init(struct lvp_module *module,
        LVPMap *options, LVPEventControl *ctl, LVPLog *log){
    assert(module);
    assert(ctl);
    assert(log);

    LVPAndroidVideoRender *r = (LVPAndroidVideoRender*)module->private_data;
    r->ctl = ctl;
    r->log = lvp_log_alloc(module->name);
    r->log->log_call = log->log_call;
    r->log->usr_data = log->usr_data;

    int ret = lvp_event_control_add_listener(r->ctl, LVP_EVENT_UPDATE_VIDEO, handle_update_video, r);
    if(ret != LVP_OK){
        lvp_error(r->log, "add update video event error", NULL);
        return LVP_E_FATAL_ERROR;
    }

    ret = lvp_event_control_add_listener(r->ctl, LVP_EVENT_AN_SURFACE_CREATED,
                                         handle_surface_created, r);
    if(ret!=LVP_OK){
        lvp_error(r->log, "handle surface changed error", NULL);
        return LVP_E_FATAL_ERROR;
    }

    ret = lvp_event_control_add_listener(r->ctl, LVP_EVENT_AN_SURFACE_DESTROYED,
                                         handle_surface_destroyed, r);
    if(ret!=LVP_OK){
        lvp_error(r->log, "handle surface destroyed error", NULL);
        return LVP_E_FATAL_ERROR;
    }

    r->env = lvp_map_get(options, LVP_OPTIONS_JNI_ENV);

    lvp_mutex_create(&r->mutex);

    return LVP_OK;
}

static void module_close(struct lvp_module *module){

}

LVPModule lvp_android_video_render = {
        .version = lvp_version,
        .name = "LVP_ANDROID_VIDEO_RENDER",
        .type = LVP_MODULE_CORE|LVP_MODULE_RENDER,
        .private_data_size = sizeof(LVPAndroidVideoRender),
        .private_data = NULL,
        .module_init = module_init,
        .module_close = module_close,
};

