#include "qeglfshooks.h"
//#include <EGL/fbdev_window.h>
#include <EGL/egl.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>


// Convert This file to using
//https://www.khronos.org/registry/egl/extensions/EXT/EGL_EXT_platform_x11.txt

#include <private/qcore_unix_p.h>
#include </home/odroid/Documents/qt5/qtbase/src/gui/kernel/qplatformwindow.h>
#include <QDebug>

QT_BEGIN_NAMESPACE

class QEglFSOdroidHooks : public QEglFSHooks
{
private:
    void initDisplay();
    EGLNativeDisplayType   mDisplay;
    EGLDisplay                            mEGL;
    EGLConfig                              mEGLConfig;
    Colormap                               mColormap;
    XVisualInfo *                        mX11Info;
    EGLNativeWindowType mWindow;
    EGLSurface                            mEGLSurface;
public:
    void platformInit() Q_DECL_OVERRIDE;
    EGLNativeWindowType createNativeWindow(QPlatformWindow *window, const QSize &size, const QSurfaceFormat &format) Q_DECL_OVERRIDE;
    void destroyNativeWindow(EGLNativeWindowType window) Q_DECL_OVERRIDE;
};

void QEglFSOdroidHooks::initDisplay() // do display grab here
{
  /*  int fd = qt_safe_open("/dev/fb0", O_RDWR, 0);
    if (fd == -1)
        qWarning("Failed to open fb to detect screen resolution!");

    struct fb_var_screeninfo vinfo;
    memset(&vinfo, 0, sizeof(vinfo));
    if (ioctl(fd, FBIOGET_VSCREENINFO, &vinfo) == -1)
        qWarning("Could not get variable screen info");*/
    mDisplay = 0;
    mDisplay = XOpenDisplay(NULL);
    if(!mDisplay)
    {
	qWarning("Error getting XOpenDislay");
        abort();
    }
    else
    {
	qWarning("Got X11 Display hook");
    }
    mEGL = EGL_NO_DISPLAY;
    mEGL = eglGetDisplay(mDisplay);
    if(mEGL == EGL_NO_DISPLAY)
    {
        qWarning("Failed to get EGL Display");
        abort();
    }
    EGLint major, minor;
    if (!eglInitialize(mEGL, &major, &minor)) {
            abort();
        }

     EGLint egl_config_attribs[] = {
            EGL_BUFFER_SIZE,        32,
            EGL_RED_SIZE,                8,
            EGL_GREEN_SIZE,          8,
            EGL_BLUE_SIZE,              8,
            EGL_ALPHA_SIZE,          8,

            EGL_DEPTH_SIZE,         EGL_DONT_CARE,
            EGL_STENCIL_SIZE,       EGL_DONT_CARE,

            EGL_RENDERABLE_TYPE,    EGL_OPENGL_ES2_BIT,
            EGL_SURFACE_TYPE,       EGL_WINDOW_BIT | EGL_PIXMAP_BIT,
            EGL_NONE,
        };

    EGLint num_configs;
    eglChooseConfig(mEGL,
                             egl_config_attribs,
                             &mEGLConfig, 1,
                             &num_configs);
    if (num_configs == 0) {
            abort();
    }
    XVisualInfo x11_visual_info_template;
        if (!eglGetConfigAttrib(mEGL,
                                mEGLConfig,
                                EGL_NATIVE_VISUAL_ID,
                                (EGLint*) &x11_visual_info_template.visualid)) {
            abort();
        }

        int num_visuals;
        mX11Info = XGetVisualInfo(mDisplay,
                                    VisualIDMask,
                                    &x11_visual_info_template,
                                    &num_visuals);
        if (!mX11Info) {
            abort();
        }
        mColormap = XCreateColormap(mDisplay,
                                          RootWindow(mDisplay, 0),
                                          mX11Info->visual, AllocNone);
        if (mColormap) {
            abort();
        }
    return;
}

void QEglFSOdroidHooks::platformInit()
{
    QEglFSHooks::platformInit();
    initDisplay();
}

EGLNativeWindowType QEglFSOdroidHooks::createNativeWindow(QPlatformWindow *window, const QSize &size, const QSurfaceFormat &format)
{
    XSetWindowAttributes attr;
    unsigned long mask;
    attr.colormap = mColormap;
    mask = CWColormap;
    int x_pos(0);
    int y_pos(0);
    if(window)
    {
        x_pos = window->geometry().x();
        y_pos = window->geometry().y();
    }
    mWindow = XCreateWindow(mDisplay, DefaultRootWindow(mDisplay),x_pos,y_pos,size.width(),size.height(),
                                                                      0,mX11Info->depth,InputOutput,mX11Info->visual,mask,&attr);
    if(mWindow)
    {
        qWarning("Failed to create X Window");        
        abort();
    }
    mEGLSurface = eglCreateWindowSurface (mEGL, mEGLConfig, mWindow,NULL);
    if(mEGLSurface == EGL_NO_SURFACE)
    {
        qWarning("Failed to create EGL Window surface");        
        abort();
    }
    
    return mWindow;
}

void QEglFSOdroidHooks::destroyNativeWindow(EGLNativeWindowType window)
{
    XDestroyWindow(mDisplay, window);
}

QEglFSOdroidHooks eglFSOdroidHooks;
QEglFSHooks *platformHooks = &eglFSOdroidHooks;

QT_END_NAMESPACE