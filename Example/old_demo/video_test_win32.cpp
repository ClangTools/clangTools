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
 * ��ȡ����ͷ�б�
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
    static TCHAR lpszClassName[] = ("����");//��������
    static TCHAR lpszTitle[] = "����ͷ����";//���ڱ�����
    //�����ඨ��
    //�����ඨ���˴��ڵ���ʽ�빦�� �����ඨ��ͨ�������������ݽṹWNDCLASS��ֵ���
    //�����ݽṹ�а���������ĸ�������
    wndclass.style = CS_HREDRAW | CS_VREDRAW; // ��������Ϊȱʡ����
    wndclass.lpfnWndProc = WndProc; //���崰�ڴ�����
    wndclass.cbClsExtra = 0; //����������չ
    wndclass.cbWndExtra = 0; //����ʵ������չ
    wndclass.hInstance = hInstance; //��ǰʵ�����
    wndclass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);//���ڵ���С��ͼ��Ϊȱʡͼ��
    wndclass.hCursor = LoadCursor(nullptr, IDC_ARROW); // ���ڲ��ü�ͷ���
    wndclass.hbrBackground = (HBRUSH) (GetStockObject(WHITE_BRUSH)); //���ڱ���Ϊ��ɫ
    wndclass.lpszMenuName = nullptr; //�����޲˵�
    wndclass.lpszClassName = lpszClassName; //��������Ϊ�����ڡ�

    if (!RegisterClass(&wndclass)) {
        MessageBox(nullptr, TEXT ("This program requires Windows XP!"),
                   lpszClassName, MB_ICONERROR);
        return 0;
    }//ע�ᴰ����

    hwnd = CreateWindow(
            lpszClassName,     //{�����������}
            lpszTitle,    //{���ڱ���}
            WS_OVERLAPPEDWINDOW,         //{������ʽ, �μ��±�}
            CW_USEDEFAULT, CW_USEDEFAULT,          //{λ��; Ĭ�ϵ�X,Y����ָ��Ϊ: Integer(CW_USEDEFAULT)}
            WindowWidth, WindowHeight,//{��С; Ĭ�ϵĿ�ȡ��߶ȿ���ָ��Ϊ: Integer(CW_USEDEFAULT)}}
            nullptr,       //{�����ھ��}
            nullptr,           //{���˵����}
            hInstance,       //{ģ��ʵ�����, Ҳ���ǵ�ǰ exe �ľ��}
            nullptr         //{���Ӳ���, �������ĵ�����ʱ���õ�, һ����Ϊ nil}
    );                  //{�����������Ĵ��ڵľ��}
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    //��Ϣѭ��
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
                    TEXT("static"), //��̬�ı��������
                    TEXT(""), //�ؼ����ı�
                    WS_CHILD /*�Ӵ���*/ | WS_VISIBLE /*����ʱ��ʾ*/ | WS_BORDER /*���߿�*/,
                    5 /*X����*/, 5/*Y����*/, VideoWidth/*���*/, VideoHeight/*�߶�*/, hwnd/*�����ھ��*/,
                    (HMENU) 1, //Ϊ�ؼ�ָ��һ��Ψһ��ʶ��
                    (HINSTANCE) GetWindowLong(hwnd, GWL_HINSTANCE), nullptr);
            hwndButton1 = CreateWindow("BUTTON", "live",
                                       WS_CHILD | WS_VISIBLE | WS_BORDER | BS_FLAT/*��ƽ��ʽ*/,
                                       WindowWidth - 210, WindowHeight - 30 * 2 - 50, 190, 25, hwnd, nullptr,
                                       (HINSTANCE) GetWindowLong(hwnd, GWL_HINSTANCE), nullptr);
            hwndButton2 = CreateWindow("BUTTON", "eyes",
                                       WS_CHILD | WS_VISIBLE | WS_BORDER | BS_FLAT/*��ƽ��ʽ*/,
                                       WindowWidth - 210, WindowHeight - 30 * 1 - 50, 190, 25, hwnd, nullptr,
                                       (HINSTANCE) GetWindowLong(hwnd, GWL_HINSTANCE), nullptr);
            hwndButton3 = CreateWindow("BUTTON", "copy",
                                       WS_CHILD | WS_VISIBLE | WS_BORDER | BS_FLAT/*��ƽ��ʽ*/,
                                       WindowWidth - 210, WindowHeight - 30 * 3 - 50, 190, 25, hwnd, nullptr,
                                       (HINSTANCE) GetWindowLong(hwnd, GWL_HINSTANCE), nullptr);
            hListBox = CreateWindow(
                    TEXT("LISTBOX"), //��̬�ı��������
                    TEXT(""), //�ؼ����ı�
                    LBS_NOTIFY | LBS_SORT | WS_VSCROLL | WS_HSCROLL | WS_BORDER | WS_CHILD /*�Ӵ���*/ |
                    WS_VISIBLE /*����ʱ��ʾ*/ |
                    WS_BORDER /*���߿�*/,
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
        case WM_COMMAND: //��Ӧ��ť��Ϣ
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


