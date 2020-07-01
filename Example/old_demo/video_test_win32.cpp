//
// Created by 86135 on 2020/5/23.
//

#include<Windows.h>
#include <string>
#include <iostream>
#include <CommCtrl.h>
#include <ctime>
#include <cstdlib>
#include <logger.h>
#include <v4l2Tool.h>
#include <vector>
#include <CameraTool/VideoTool.h>

using namespace std;
using namespace clangTools;

#if !WindowWidth
#define WindowWidth        640 + 110 + 110
#define VideoWidth        640
#define VideoHeight       480
#define WindowHeight    (VideoHeight + 40 + 10)
#endif
#if !MAX_PATH
#define MAX_PATH		256
#endif

std::vector<VideoInfo> videoList;
v4l2Tool vTool;
VideoDevice *vDevice = nullptr;

static HWND hStatic = nullptr;
static HWND hListBox = nullptr;
static HWND hwndButton1 = nullptr;
static HWND hwndButton2 = nullptr;
static HWND hwndButton3 = nullptr;

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void video_cb(unsigned char *data, int len, LONG WIDTH, LONG HEIGHT, int bitsperpixel, VideoDevice *dev);

/**
 * 获取摄像头列表
 * @return
 */
std::vector<VideoInfo> GetVideoLists() {
    std::vector<VideoInfo> _videoList;
    vTool.Init();
    auto devLen = vTool.NumDevices();
    auto dev = vTool.GetDevices();
    for (int i = 0; i < devLen; i++) {
        VideoInfo vInfo;
        vInfo.name = dev[i].GetFriendlyName();
        vInfo.index = dev[i].GetId() - 1;
        vInfo.vid = dev[i].vid;
        vInfo.pid = dev[i].pid;
        vInfo.path = "";
        _videoList.push_back(vInfo);
    }
    return _videoList;
}

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow) {
    logger::instance()->init_default(logger::get_local_path() + logger::path_split + "log.log");

    videoList = GetVideoLists();


    HWND hwnd;
    MSG Msg;
    WNDCLASS wndclass;
    srand((unsigned int) time(nullptr));
    static TCHAR lpszClassName[] = ("窗口");//窗口类名
    static TCHAR lpszTitle[] = "摄像头测试";//窗口标题名
    //窗口类定义
    //窗口类定义了窗口的形式与功能 窗口类定义通过给窗口类数据结构WNDCLASS赋值完成
    //该数据结构中包含窗口类的各种属性
    wndclass.style = CS_HREDRAW | CS_VREDRAW; // 窗口类型为缺省类型
    wndclass.lpfnWndProc = WndProc; //定义窗口处理函数
    wndclass.cbClsExtra = 0; //窗口类无扩展
    wndclass.cbWndExtra = 0; //窗口实例无扩展
    wndclass.hInstance = hInstance; //当前实例句柄
    wndclass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);//窗口的最小化图标为缺省图标
    wndclass.hCursor = LoadCursor(nullptr, IDC_ARROW); // 窗口采用箭头光标
    wndclass.hbrBackground = (HBRUSH) (GetStockObject(WHITE_BRUSH)); //窗口背景为白色
    wndclass.lpszMenuName = nullptr; //窗口无菜单
    wndclass.lpszClassName = lpszClassName; //窗口类名为“窗口”

    if (!RegisterClass(&wndclass)) {
        MessageBox(nullptr, TEXT ("This program requires Windows XP!"),
                   lpszClassName, MB_ICONERROR);
        return 0;
    }//注册窗口类

    hwnd = CreateWindow(
            lpszClassName,     //{窗口类的名字}
            lpszTitle,    //{窗口标题}
            WS_OVERLAPPEDWINDOW,         //{窗口样式, 参加下表}
            CW_USEDEFAULT, CW_USEDEFAULT,          //{位置; 默认的X,Y可以指定为: Integer(CW_USEDEFAULT)}
            WindowWidth, WindowHeight,//{大小; 默认的宽度、高度可以指定为: Integer(CW_USEDEFAULT)}}
            nullptr,       //{父窗口句柄}
            nullptr,           //{主菜单句柄}
            hInstance,       //{模块实例句柄, 也就是当前 exe 的句柄}
            nullptr         //{附加参数, 创建多文档界面时才用到, 一般设为 nil}
    );                  //{返回所创建的窗口的句柄}
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    //消息循环
    while (GetMessage(&Msg, nullptr, 0,
                      0)) {
        TranslateMessage(
                &Msg);
        DispatchMessage(
                &Msg);
    }
    if (vDevice != nullptr) {
        vDevice->Stop();
    }

    return Msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    HINSTANCE hInstance;
    int d = 0;
    static POINT pt[2];
    switch (message) {
        case WM_CREATE:
            hInstance = ((LPCREATESTRUCT) lParam)->hInstance;
            hStatic = CreateWindow(
                    TEXT("static"), //静态文本框的类名
                    TEXT(""), //控件的文本
                    WS_CHILD /*子窗口*/ | WS_VISIBLE /*创建时显示*/ | WS_BORDER /*带边框*/,
                    5 /*X坐标*/, 5/*Y坐标*/, VideoWidth/*宽度*/, VideoHeight/*高度*/, hwnd/*父窗口句柄*/,
                    (HMENU) 1, //为控件指定一个唯一标识符
                    (HINSTANCE) GetWindowLong(hwnd, GWL_HINSTANCE), nullptr);
            hwndButton1 = CreateWindow("BUTTON", "live",
                                       WS_CHILD | WS_VISIBLE | WS_BORDER | BS_FLAT/*扁平样式*/,
                                       WindowWidth - 210, WindowHeight - 30 * 2 - 50, 190, 25, hwnd, nullptr,
                                       (HINSTANCE) GetWindowLong(hwnd, GWL_HINSTANCE), nullptr);
            hwndButton2 = CreateWindow("BUTTON", "eyes",
                                       WS_CHILD | WS_VISIBLE | WS_BORDER | BS_FLAT/*扁平样式*/,
                                       WindowWidth - 210, WindowHeight - 30 * 1 - 50, 190, 25, hwnd, nullptr,
                                       (HINSTANCE) GetWindowLong(hwnd, GWL_HINSTANCE), nullptr);
            hwndButton3 = CreateWindow("BUTTON", "copy",
                                       WS_CHILD | WS_VISIBLE | WS_BORDER | BS_FLAT/*扁平样式*/,
                                       WindowWidth - 210, WindowHeight - 30 * 3 - 50, 190, 25, hwnd, nullptr,
                                       (HINSTANCE) GetWindowLong(hwnd, GWL_HINSTANCE), nullptr);
            hListBox = CreateWindow(
                    TEXT("LISTBOX"), //静态文本框的类名
                    TEXT(""), //控件的文本
                    LBS_NOTIFY | LBS_SORT | WS_VSCROLL | WS_HSCROLL | WS_BORDER | WS_CHILD /*子窗口*/ |
                    WS_VISIBLE /*创建时显示*/ |
                    WS_BORDER /*带边框*/,
                    WindowWidth - 210, 5, 190, WindowHeight - 30 * 4 - 50, hwnd, nullptr,
                    (HINSTANCE) GetWindowLong(hwnd, GWL_HINSTANCE), nullptr);

            ShowWindow(hListBox, SW_SHOW);
            SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
            for (const auto &item : videoList)
                SendMessage(hListBox, LB_ADDSTRING, 0,
                            (LPARAM) ("[" + item.vid + ":" + item.pid + "] " + item.name).c_str());


            RECT rect;
            GetWindowRect(hwnd, &rect);
            InvalidateRect(hwnd, &rect, true);
            UpdateWindow(hwnd);
            return 0;
        case WM_COMMAND: //响应按钮消息
        {
            HWND now_hwndButton = (HWND) lParam;
            if (now_hwndButton == hwndButton1) {
                break;
            } else if (now_hwndButton == hwndButton2) {
                break;
            } else if (now_hwndButton == hwndButton3) {
                break;
            } else if (now_hwndButton == hListBox) {
                switch (HIWORD(wParam)) {
                    case LBN_DBLCLK:
                        int index = SendMessage(hListBox, LB_GETCURSEL, 0, 0);
                        if (index < vTool.NumDevices()) {
                            vDevice = &vTool.GetDevices()[index];
                            vDevice->SetCallback(video_cb);
                            vDevice->Start();
                        }
                        break;
                }
                break;
            }
            return 0;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

void video_cb(unsigned char *data, int len, LONG WIDTH, LONG HEIGHT, int bitsperpixel, VideoDevice *dev){

}


