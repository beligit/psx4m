#include <X11/Xlib.h>
#include <GLES/egl.h>
#include <GL/gl.h>
#include <stdio.h>

#define GL_COMPILE                        0x1300
#define GL_COMPILE_AND_EXECUTE            0x1301

EGLDisplay display;
EGLSurface surface;
EGLContext context;
EGLConfig config;

Window root_window;
Window xwin;
Display* xdisplay;

typedef struct tglesMesaContext* glesMesaContext;
extern glesMesaContext glesMesaCreateContext(EGLDisplay display, EGLConfig config);
extern void glesMesaMakeCurrent(glesMesaContext ctx, EGLContext context, EGLSurface read, EGLSurface draw);
extern void glesMesaSwapBuffers(void);

typedef double GLdouble;
extern void glOrtho (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
extern void glVertex3f (GLfloat x, GLfloat y, GLfloat z);
extern void glColor3f (GLfloat x, GLfloat y, GLfloat z);
extern void glColor4f (GLfloat x, GLfloat y, GLfloat z, GLfloat w);

glesGLXContext glxContext;
glesMesaContext glesMesaCtx;

void createX11window() {
    xdisplay = XOpenDisplay(NULL);
    if (xdisplay == NULL) {
        fprintf(stderr, "Cannot connect to X server %s\n", ":0");
    }

    int screen_num;

    /* these variables will store the size of the screen, in pixels.    */
    int screen_width;
    int screen_height;

    /* this variable will be used to store the ID of the root window of our */
    /* screen. Each screen always has a root window that covers the whole   */
    /* screen, and always exists.                                           */

    /* these variables will be used to store the IDs of the black and white */
    /* colors of the given screen. More on this will be explained later.    */
    unsigned long white_pixel;
    unsigned long black_pixel;

    /* check the number of the default screen for our X server. */
    screen_num = DefaultScreen(xdisplay);

    /* find the width of the default screen of our X server, in pixels. */
    screen_width = DisplayWidth(xdisplay, screen_num);

    /* find the height of the default screen of our X server, in pixels. */
    screen_height = DisplayHeight(xdisplay, screen_num);

    /* find the ID of the root window of the screen. */
    root_window = RootWindow(xdisplay, screen_num);

    /* find the value of a white pixel on this screen. */
    white_pixel = WhitePixel(xdisplay, screen_num);

    /* find the value of a black pixel on this screen. */
    black_pixel = BlackPixel(xdisplay, screen_num);

    xwin = XCreateSimpleWindow(
            xdisplay,
            RootWindow(xdisplay, screen_num),
            0, 0,
            800, 480,
            0, BlackPixel(xdisplay, screen_num),
            WhitePixel(xdisplay, screen_num)
        );
    XMapWindow(xdisplay, xwin);
}

void initEGL()
{
    EGLint attrs[] = {
        EGL_BUFFER_SIZE, 16,
        EGL_NONE
    };
    EGLint configs;

    display = eglGetDisplay(xdisplay);
    eglInitialize(display, NULL, NULL);
    eglChooseConfig(display, attrs, &config, 1, &configs);
    printf("Config ID: %p\n", config);
    surface = eglCreateWindowSurface(display, config, xwin, NULL);
    context = eglCreateContext(display, config, EGL_NO_CONTEXT, NULL);
    eglMakeCurrent(display, surface, surface, context);
}

void initGLESMesa()
{
   glxContext = glXCreateContext(display, config);
   glesMesaCtx = glesMesaCreateContext(glxContext);
   glesMesaMakeCurrent(glesMesaCtx, context, surface, surface);
}

int main(int argc, char** argv)
{
    const GLfloat vertices[] = {
         0, -1, -3,
        -1,  1, -3,
         1,  1, -3,
    };

    createX11window();
    initEGL();
    initGLESMesa();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, 800, 480);
    glOrtho(-2.f, 2.f, 2.f, -2.f, 1.f, 100.f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glShadeModel(GL_SMOOTH);

    glNewList(1, GL_COMPILE);
    glBegin(GL_TRIANGLES);
    glColor4f(1, .7, 1, 1);
    glVertex3f( 0, -1, -3);
    glColor4f(.7, 1, 1, 1);
    glVertex3f(-1,  1,  -3);
    glColor4f(1, 1, .7, 1);
    glVertex3f( 1,  1,  -3);
    glEnd();
    glEndList();

    while (1)
    {
        glClearColor(0, .3, .3, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glRotatef(1, 0, 0, 1);

#if 0
        glBegin(GL_TRIANGLES);
        glColor4f(0, .7, 0, 1);
        glVertex3f( 0, -1, -3);
        glColor4f(.7, 0, 0, 1);
        glVertex3f(-1,  1,  -3);
        glColor4f(0, 0, .7, 1);
        glVertex3f( 1,  1,  -3);
        /*
        glVertex3f(1, 1, 3);
        glVertex3f(2, 1, 3);
        glVertex3f(1, 2, 3);
        glVertex3f(1, 1, 3);
        glVertex3f(2, 1, 3);
        glVertex3f(1, 2, 3);
        glVertex3f(1, 1, 3);
        glVertex3f(2, 1, 3);
        glVertex3f(1, 2, 3);
        glVertex3f(1, 1, 3);
        glVertex3f(2, 1, 3);
        glVertex3f(1, 2, 3);
        glVertex3f(1, 1, 3);
        glVertex3f(2, 1, 3);
        glVertex3f(1, 2, 3);
        */
        glEnd();
#elif 0
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, vertices);
        glDrawArrays(GL_TRIANGLES, 0, 3);
#else
        glCallList(1);
#endif

        glesMesaSwapBuffers();
        eglSwapBuffers(display, surface);
    }

    return 0;
}
