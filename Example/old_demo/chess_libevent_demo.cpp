//
// Created by caesar kekxv on 2020/6/5.
//
#include <kHttpd.h>
#include <logger.h>
#include <kHttpdClient.h>
#include <opencv2/opencv.hpp>
#include <json11/json11.hpp>
#include <nlohmann/json.hpp>
#include <Base64/base64.h>
#include <cstdio>

#ifdef _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <getopt.h>
#include <io.h>
#include <fcntl.h>

#ifndef S_ISDIR
#define S_ISDIR(x) (((x) & S_IFMT) == S_IFDIR)
#endif
#else /* !_WIN32 */

#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

#endif /* _WIN32 */

#include <event.h>
#include <evhttp.h>

using namespace std;
using namespace cv;
string web_root;

int init_win_socket() {
#ifdef WIN32
    WORD wVersionRequested;
    WSADATA wsaData;
    wVersionRequested = MAKEWORD(2, 2);
    WSAStartup(wVersionRequested, &wsaData);
#endif
    return 0;
}

vector<Vec2f> FindChess(const Mat &srcImage, Mat &image_copy) {
    Mat midImage, dstImage;//临时变量和目标图的定义

    // Rect rect(250, 0, 1205, 1080);   //创建一个Rect框，属于cv中的类，四个参数代表x,y,width,height
    Mat image_cut = srcImage.clone();      //从img中按照rect进行切割，此时修改image_cut时image中对应部分也会修改，因此需要copy
    image_copy = image_cut.clone();   //clone函数创建新的图片
    Mat midImage2;
    Mat image_copy2;
    cv::transpose(image_copy, image_copy2);
    cv::flip(image_copy2, image_copy2, 1);
    //flip(image_copy2, image_copy,-1);
    //【3】转为灰度图，进行图像平滑
    cvtColor(image_copy, midImage, COLOR_BGR2GRAY);//灰度化
    //二值化
    threshold(midImage, midImage, 128, 255, THRESH_BINARY | THRESH_OTSU);
    //使用3*3内核来降噪
    blur(midImage, midImage, Size(3, 3));//进行模糊
    //GaussianBlur(midImage, midImage, Size(9, 9), 2, 2);
    cv::transpose(midImage, midImage2);
    cv::flip(midImage2, midImage2, 1);
    //【4】进行霍夫圆变换
    vector<Vec3f> circles;
    //  HoughCircles(midImage, circles, CV_HOUGH_GRADIENT, 1.5, 10, 200, 100, 0, 0);
    //      第五个参数   圆的圆心之间的最小距离
    HoughCircles(midImage2, circles, HOUGH_GRADIENT, 1.5, 70, 100, 25, 36, 43);

    vector<Vec2f> xys;
    //【5】依次在图中绘制出圆
    for (auto &i : circles) {
        Point center(cvRound(i[0]), cvRound(i[1]));
        int radius = cvRound(i[2]);
        int x = round((center.x - 64.4) / 122.5);
        int y = round((center.y - 94.16) / 114.4);
        //int newx=x*rotMat
        // cout << "矩阵坐标  " << x+4 << "," << y+4 << ";\n" << endl;
        xys.emplace_back(x + 4, y + 4);
        //绘制圆心
        circle(midImage, center, 3, Scalar(0, 255, 0), -1, 8, 0);
        //绘制圆轮廓
        circle(midImage, center, radius, Scalar(155, 50, 255), 3, 8, 0);
        //绘制圆心
        circle(image_copy2, center, 3, Scalar(0, 255, 0), -1, 8, 0);
        //绘制圆轮廓
        circle(image_copy2, center, radius, Scalar(155, 50, 255), 3, 8, 0);
    }
    return xys;
}


static const struct table_entry {
    const char *extension;
    const char *content_type;
} content_type_table[] = {
        {"txt",  "text/plain"},
        {"c",    "text/plain"},
        {"h",    "text/plain"},
        {"html", "text/html"},
        {"htm",  "text/htm"},
        {"css",  "text/css"},
        {"gif",  "image/gif"},
        {"jpg",  "image/jpeg"},
        {"jpeg", "image/jpeg"},
        {"png",  "image/png"},
        {"pdf",  "application/pdf"},
        {"ps",   "application/postscript"},
        {NULL, NULL},
};

/* Try to guess a good content-type for 'path' */
static const char *
guess_content_type(const char *path) {
    const char *last_period, *extension;
    const struct table_entry *ent;
    last_period = strrchr(path, '.');
    if (!last_period || strchr(last_period, '/'))
        goto not_found; /* no exension */
    extension = last_period + 1;
    for (ent = &content_type_table[0]; ent->extension; ++ent) {
        if (!evutil_ascii_strcasecmp(ent->extension, extension))
            return ent->content_type;
    }

    not_found:
    return "application/misc";
}


//指定generic callback
void generic_handler(evhttp_request *req, void *) {
#ifdef WIN32
    return ;
#endif
    struct evbuffer *buf = evbuffer_new();
    if (!buf) {
        puts("failed to create response buffer \n");
        return;
    }
    const char *uri = evhttp_request_get_uri(req);
    struct evhttp_uri *decoded = NULL;
    const char *path = NULL;
    char *decoded_path = NULL;
    char *whole_path = NULL;
    struct stat st;
    /* Decode the URI */
    decoded = evhttp_uri_parse(uri);
    if (!decoded) {
        printf("It's not a good URI. Sending BADREQUEST\n");
        evhttp_send_error(req, HTTP_BADREQUEST, 0);
        return;
    }
    /* Let's see what path the user asked for. */
    path = evhttp_uri_get_path(decoded);
    if (!path) path = "/";

    /* We need to decode it, to see what path the user really wanted. */
    decoded_path = evhttp_uridecode(path, 0, NULL);
    if (decoded_path == NULL)
        goto err;

    {
        int len = strlen(decoded_path) + web_root.size() + 2;
        if (!(whole_path = (char *) malloc(len))) {
            perror("malloc");
            goto err;
        }
        evutil_snprintf(whole_path, len, "%s/%s", web_root.c_str(), decoded_path);

        if (stat(whole_path, &st) < 0) {
            goto err;
        }
        /* Otherwise it's a file; add it to the buffer to get
		 * sent via sendfile */
        const char *type = guess_content_type(decoded_path);
        int fd;
        if ((fd = open(whole_path, O_RDONLY | O_BINARY)) < 0) {
            perror("open");
            goto err;
        }

        if (fstat(fd, &st) < 0) {
            /* Make sure the length still matches, now that we
             * opened the file :/ */
            perror("fstat");
            goto err;
        }
        evhttp_add_header(evhttp_request_get_output_headers(req),
                          "Content-Type", type);

        printf("%s:%d %d\n", __FILE__, __LINE__,st.st_size);
        evbuffer_add_file(buf, fd, 0, st.st_size);
        printf("%s:%d\n", __FILE__, __LINE__);
    }
    goto done;
    err:
    evbuffer_free(buf);
    buf = evbuffer_new();
    evbuffer_add_printf(buf, "%s", "{\"error\":-1}");
    done:
    evhttp_send_reply(req, HTTP_OK, "OK", buf);
    if (decoded)
        evhttp_uri_free(decoded);
    if (decoded_path)
        free(decoded_path);
    if (whole_path)
        free(whole_path);
    if (buf)
        evbuffer_free(buf);
}

void findChess(evhttp_request *req, void *) {
    struct evbuffer *buf = evbuffer_new();
    if (!buf) {
        puts("failed to create response buffer \n");
        return;
    }
    string data = (char *) EVBUFFER_DATA(req->input_buffer);

    json11::Json json;
    std::string err;
    json = json11::Json::parse((const char *) data.data(), err);
    if (!err.empty() || !json["image"].is_string()) {
        string _ = "{\"error\":-1}";
        evbuffer_add_printf(buf, "%s", _.c_str());
        evhttp_send_reply(req, HTTP_OK, "OK", buf);
        evbuffer_free(buf);
        return;
    }

    string image = json["image"].string_value();
    auto _index = image.find(',');
    if (_index != string::npos) {
        image = image.substr(_index + 1);
    }
    auto img_bin = base64_decode(image);
    Mat img = imdecode(img_bin, 1);
    if (img.empty()) {
        string _ = "{\"error\":-2}";
        evbuffer_add_printf(buf, "%s", _.c_str());
        evhttp_send_reply(req, HTTP_OK, "OK", buf);
        evbuffer_free(buf);
        return;
    }
    nlohmann::json Json;
    Json["error"] = 0;

    Mat outImg;
    auto out = FindChess(img, outImg);
    nlohmann::json outJson;
    for (auto &i : out) {
        outJson.push_back({
                                  {"x", i[0]},
                                  {"y", i[1]},
                          });
    }
    Json["data"] = outJson;
    vector<unsigned char> photo;
    vector<int> compression_params;
    compression_params.push_back(IMWRITE_JPEG_QUALITY);
    compression_params.push_back(85);
    imencode(".jpg", outImg, photo, compression_params);
    Json["image"] = "data:image/jpg;base64," + base64_encode(photo.data(), photo.size());

    string _json = Json.dump();
    evbuffer_add_printf(buf, "%s", _json.c_str());
    evhttp_send_reply(req, HTTP_OK, "OK", buf);
    evbuffer_free(buf);
}

evhttp *httpd = nullptr;

int main(int argc, char *argv[]) {
    init_win_socket();
    logger::instance()->init_default();
    web_root = logger::get_local_path() + logger::path_split + "WebRoot";
    if (!logger::exists(web_root)) {
        web_root =
                logger::get_local_path() + logger::path_split + ".." + logger::path_split + "Example" +
                logger::path_split +
                "WebRoot";
    }
    //初始化event API
    event_init();

    struct event_base *base = event_base_new();
    struct evhttp *http_server = evhttp_new(base);
    if (!http_server) {
        return -1;
    }
    int ret = evhttp_bind_socket(http_server, "0.0.0.0", 8080);
    if (ret != 0) {
        return -1;
    }

    evhttp_set_gencb(http_server, generic_handler, nullptr);
    evhttp_set_cb(http_server, "/FindChess", findChess, nullptr);
    printf("http server start OK! \n");
    printf("http://0.0.0.0:8080/ \n");
    event_base_dispatch(base);
    evhttp_free(http_server);
/*
    int thread_num = 20;

    kChess.set_cb("POST", "/FindChess",
                  [](void *kClient, const std::vector<unsigned char> &data, const std::string &url_path,
                     const std::string &method, int type, void *arg) -> int {
                      if (-1 != type)return -1;
                      auto *kHttpdClient = (::kHttpdClient *) kClient;
                  });
*/
    event_dispatch();
#ifdef WIN32
    WSACleanup();
#endif
    return 0;
}