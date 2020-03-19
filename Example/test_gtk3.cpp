//
// Created by caesar kekxv on 2020/3/18.
//

#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <logger.h>
#include <thread>
#include <atomic>

#if defined(LINUX) || defined(linux)
#include <X11/Xlib.h>
#endif


#define LOGGER_PRINT_FUNC_NAME()  logger::instance()->i(__FILENAME__, __LINE__, "%s", __FUNCTION__)

using namespace std;
using namespace cv;

int X_WIDTH       =   640;
int X_HEIGHT      =   480;

VideoCapture cap;
cv::Mat last_frame;
int fps = 30;

// 是否关闭
std::atomic<bool> stoped{false};
GMutex _mutex;
// 同步
std::mutex m_lock;
std::thread video_task;

GtkWidget *image = nullptr;
/* Surface to store current scribbles */
static cairo_surface_t *surface = nullptr;

GdkPixbuf *MatToGdkPixbuf(Mat inMat) {
    // IplImage -> GdkPixbuf
    GdkPixbuf *src = gdk_pixbuf_new_from_data(
            (const guchar *) inMat.data,
            GDK_COLORSPACE_RGB,
            false,
            8,
            inMat.size().width,
            inMat.size().height,
            inMat.step,
            nullptr,
            nullptr);
    return src;
}


static void
clear_surface() {
    LOGGER_PRINT_FUNC_NAME();
    if (surface == nullptr)return;
    cairo_t *cr;
    cr = cairo_create(surface);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);
    cairo_destroy(cr);
}

/* Create a new surface of the appropriate size to store our scribbles */
static gboolean
configure_event_cb(GtkWidget *widget,
                   GdkEventConfigure *event,
                   gpointer data) {
    LOGGER_PRINT_FUNC_NAME();
    if (surface)
        cairo_surface_destroy(surface);

    surface = gdk_window_create_similar_surface(gtk_widget_get_window(widget),
                                                CAIRO_CONTENT_COLOR,
                                                gtk_widget_get_allocated_width(widget),
                                                gtk_widget_get_allocated_height(widget));

    /* Initialize the surface to white */
    clear_surface();

    /* We've handled the configure event, no need for further processing. */
    return TRUE;
}

/* Redraw the screen from the surface. Note that the ::draw
 * signal receives a ready-to-be-used cairo_t that is already
 * clipped to only draw the exposed areas of the widget
 */
static gboolean
draw_cb(GtkWidget *widget,
        cairo_t *cr,
        gpointer data) {
    LOGGER_PRINT_FUNC_NAME();
    cairo_set_source_surface(cr, surface, 0, 0);
    cairo_paint(cr);

    return FALSE;
}


static void
close_window() {
    LOGGER_PRINT_FUNC_NAME();
    // std::unique_lock<std::mutex> _lock{m_lock};
    if (surface)
        cairo_surface_destroy(surface);
    surface = nullptr;
}


static gboolean draw_event(GtkWidget *widget, cairo_t *cr) {
    LOGGER_PRINT_FUNC_NAME();
    GdkWindow *win;
    win = gtk_widget_get_window(widget);

    // if (!_lock.try_lock())return FALSE;
    // std::unique_lock<std::mutex> _lock{m_lock};
    g_mutex_lock(&_mutex);
    if (!last_frame.empty()) {

        auto src = MatToGdkPixbuf(last_frame);
        g_mutex_unlock(&_mutex);
        // _lock.unlock();
        if (src == nullptr) {
            return FALSE;
        }
        gdk_cairo_set_source_pixbuf(cr, src, 0, 0);
        cairo_paint(cr);
        cairo_fill(cr);
        // cairo_destroy(cr);
        // cairo_region_destroy(c);

        // g_object_unref(src);
    } else
        g_mutex_unlock(&_mutex);
    // else
    //     _lock.unlock();
    return TRUE;
    // return FALSE;
}

void video_task_run(GtkWidget *_image) {
    while (!stoped.load() && cap.isOpened()) {
        Mat frame;
        cap >> frame;
        if (frame.empty())break;

        resize(frame, frame, Size(X_WIDTH, X_HEIGHT));
        // Mat(BGR) -> IplImage(RGB)
        cvtColor(frame, frame, COLOR_BGR2RGB);

        // imshow("frame",frame);

        if (surface == nullptr) {
            usleep(1000 / fps * 1000);
            continue;
        }

        // std::unique_lock<std::mutex> _lock{m_lock};
        g_mutex_lock(&_mutex);
        if (surface == nullptr) {
            g_mutex_unlock(&_mutex);
            break;
        }
        last_frame = frame;

        g_idle_add((GSourceFunc) gtk_widget_queue_draw, (void *) _image);
        // gdk_threads_add_idle((GSourceFunc) gtk_widget_queue_draw, (void *) _image);
        g_mutex_unlock(&_mutex);
        // _lock.unlock();
        // gtk_widget_queue_draw(_image);

        usleep(1000 / fps * 800);
        g_idle_remove_by_data((void *) _image);
        // waitKey(1000 / fps);
    }
    logger::instance()->d(__FILENAME__, __LINE__, "视频读取完毕");
}


static void
activate(GtkApplication *app,
         gpointer user_data) {
    LOGGER_PRINT_FUNC_NAME();
    GtkWidget *window;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW (window), "Window");
    gtk_window_set_default_size(GTK_WINDOW (window), X_WIDTH, X_HEIGHT);
    gtk_window_set_position( GTK_WINDOW(window), GTK_WIN_POS_CENTER_ALWAYS );

    g_signal_connect (window, "destroy", G_CALLBACK(close_window), NULL);

    image = gtk_drawing_area_new();

    gtk_widget_set_size_request(image, X_WIDTH, X_HEIGHT);
    gtk_container_add(GTK_CONTAINER (window), image);

    g_signal_connect (image, "draw",
                      G_CALLBACK(draw_event), NULL);
    /* Signals used to handle the backing surface */
    // g_signal_connect (image, "draw",
    //                   G_CALLBACK(draw_cb), NULL);

    g_signal_connect (image, "configure-event",
                      G_CALLBACK(configure_event_cb), NULL);

    video_task = thread(video_task_run, image);
    video_task.detach();


    gtk_widget_show_all(window);
}


int main(int argc, char *argv[]) {
    logger::instance()->init_default();

#if defined(LINUX) || defined(linux)
    setenv("DISPLAY", "localhost:10.0", 1);
    // setenv("DISPLAY", ":1", 1);
    logger::instance()->i(__FILENAME__, __LINE__, "XOpenDisplay : %s", XOpenDisplay(nullptr) ? "True" : "False");
#endif

    cap.open(1);
    // cap.open(optarg);
    if (cap.isOpened()) {
        fps = cap.get(CAP_PROP_FPS);
        X_WIDTH = cap.get(CAP_PROP_FRAME_WIDTH);
        X_HEIGHT = cap.get(CAP_PROP_FRAME_HEIGHT);
        cap.read(last_frame);
    }
    GtkApplication *app;
    int status;

    app = gtk_application_new("com.example", G_APPLICATION_FLAGS_NONE);
    g_signal_connect (app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION (app), 1, argv);
    g_object_unref(app);

    return status;
}

