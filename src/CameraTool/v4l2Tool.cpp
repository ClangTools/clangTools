#ifndef __APPLE__

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <assert.h>
#include <string>

#ifdef WIN32
#else

#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <dirent.h>
#include <iostream>
#include <fstream>

#endif

#include "v4l2Tool.h"
#include <logger.h>
#include "BmpTool.h"

#ifdef WIN32

#include <comdef.h>
#include <string>
#include <dshow.h>
// 头文件
#include <regex>

extern const GUID GUID_NULL = { 0x0000, 0x0000, 0x0000, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} };
#endif // WIN32

using namespace std;
// #define pthread_mutex_lock(TAG) pthread_mutex_lock(TAG);logger::instance()->d("pthread_mutex_lock","pthread_mutex_lock")
// #define pthread_mutex_unlock(TAG) pthread_mutex_unlock(TAG);logger::instance()->d("pthread_mutex_lock","pthread_mutex_unlock")
namespace clangTools {


#ifdef __FILENAME__
const char *v4l2Tool::TAG = __FILENAME__;
#else
const char * v4l2Tool::TAG = "v4l2Tool";
#endif

#ifdef WIN32

    //VIDEODEVICE
    //public
    VideoDevice::VideoDevice()
    {
        memset(vid, 0x00, 20);
        memset(pid, 0x00, 20);
        friendlyname = (char *)calloc(1, MAX_DEVICE_NAME * sizeof(char));
        filtername = (WCHAR *)calloc(1, MAX_DEVICE_NAME * sizeof(WCHAR));

        id = -1;
        sourcefilter = 0;
        samplegrabberfilter = 0;
        nullrenderer = 0;
        callbackhandler = new CallbackHandler(this);
    }

    VideoDevice::~VideoDevice()
    {
        Stop();
        if (samplegrabberfilter != NULL)
            samplegrabberfilter->Release();
        if (nullrenderer != NULL)
            nullrenderer->Release();

        if (sourcefilter != NULL) {
            sourcefilter->Release();
            sourcefilter = NULL;
            //if (capture_filter != NULL) {
            //	capture_filter->Release();
            //	capture_filter = NULL;
            //}
            //if (samplegrabber != NULL) {
            //	samplegrabber->Release();
            //	samplegrabber = NULL;
            //}
            //if (graph != NULL) {
            //	graph->Release();
            //	graph = NULL;
            //}
        }
        /*if (callbackhandler != NULL)
        {
            delete[] callbackhandler;
            callbackhandler = NULL;
        }*/
        free(friendlyname);
        free(filtername);

    }
    /**
     * @brief 获取ID
     *
     * @return int
     */
    int VideoDevice::GetId()
    {
        return id;
    }
    /**
     * @brief 回去设备名
     *
     * @return const char*
     */
    const char *VideoDevice::GetFriendlyName()
    {
        return friendlyname;
    }
    /**
     * @brief 设置回调函数
     *
     * @param cb 回调函数
     */
    void VideoDevice::SetCallback(VideoCaptureCallback cb)
    {
        callbackhandler->SetCallback(cb);
    }
    /**
     * @brief 开始获取视频流
     *
     */
    void VideoDevice::Start()
    {
        HRESULT hr;

        hr = nullrenderer->Run(0);
        //if (hr < 0) throw hr;
        if (hr < 0)
            return;

        hr = samplegrabberfilter->Run(0);
        //if (hr < 0) throw hr;
        if (hr < 0)
            return;

        hr = sourcefilter->Run(0);
        //if (hr < 0) throw hr;
        if (hr < 0)
            return;
    }
    /**
     * @brief 获取亮度值
     *
     * @param val 当前亮度值
     * @param Min 最小值
     * @param Max 最大值
     * @return true 成功
     * @return false 失败
     */
    bool VideoDevice::GetBrightness(long *val, long *Min, long *Max) {
        if (capture_filter == NULL)return false;
        IAMVideoProcAmp *pProcAmp = 0;
        HRESULT hr = capture_filter->QueryInterface(IID_IAMVideoProcAmp, (void**)&pProcAmp);
        if (SUCCEEDED(hr))
        {
            long Step, Default, Flags;
            hr = pProcAmp->GetRange(VideoProcAmp_Brightness, Min, Max, &Step, &Default, &Flags);
            if (SUCCEEDED(hr))
            {
                hr = pProcAmp->Get(VideoProcAmp_Brightness, val, &Flags);
                return SUCCEEDED(hr);
            }
        }
        return false;
    }
    /**
     * @brief 设置亮度
     *
     * @param val 需要设置的值
     * @return true 设置成功
     * @return false 设置失败
     */
    bool VideoDevice::SetBrightness(long *val) {
        if (capture_filter == NULL)return false;
        IAMVideoProcAmp *pProcAmp = 0;
        HRESULT hr = capture_filter->QueryInterface(IID_IAMVideoProcAmp, (void**)&pProcAmp);
        if (SUCCEEDED(hr))
        {
            long Min, Max, Step, Default, Flags;
            hr = pProcAmp->GetRange(VideoProcAmp_Brightness, &Min, &Max, &Step, &Default, &Flags);
            if (SUCCEEDED(hr))
            {
                hr = pProcAmp->Set(VideoProcAmp_Brightness, *val, Flags);
                return SUCCEEDED(hr);
            }
        }
        return false;
    }
    /**
     * @brief 获取对比度
     *
     * @param val 当前对比度值
     * @param Min 最小对比度的值
     * @param Max 最大对比度值
     * @return true 获取成功
     * @return false 失败
     */
    bool VideoDevice::GetContrast(long *val, long *Min, long *Max) {
        if (capture_filter == NULL)return false;
        IAMVideoProcAmp *pProcAmp = 0;
        HRESULT hr = capture_filter->QueryInterface(IID_IAMVideoProcAmp, (void**)&pProcAmp);
        if (SUCCEEDED(hr))
        {
            long Step, Default, Flags;
            hr = pProcAmp->GetRange(VideoProcAmp_Contrast, Min, Max, &Step, &Default, &Flags);
            if (SUCCEEDED(hr))
            {
                hr = pProcAmp->Get(VideoProcAmp_Contrast, val, &Flags);
                return SUCCEEDED(hr);
            }
        }
        return false;
    }
    /**
     * @brief 对比度
     *
     * @param val 需要设置的对比度值
     * @return true 设置成功
     * @return false 设置失败
     */
    bool VideoDevice::SetContrast(long *val) {
        if (capture_filter == NULL)return false;
        IAMVideoProcAmp *pProcAmp = 0;
        HRESULT hr = capture_filter->QueryInterface(IID_IAMVideoProcAmp, (void**)&pProcAmp);
        if (SUCCEEDED(hr))
        {
            long Min, Max, Step, Default, Flags;
            HRESULT hr = pProcAmp->GetRange(VideoProcAmp_Contrast, &Min, &Max, &Step, &Default, &Flags);
            if (SUCCEEDED(hr))
            {
                hr = pProcAmp->Set(VideoProcAmp_Contrast, *val, Flags);
                return SUCCEEDED(hr);
            }
        }
        return false;
    }
    /**
     * @brief 获取饱和度值以及范围
     *
     * @param val 当前饱和度的值
     * @param Min 最小值
     * @param Max 最大值
     * @return true 获取成功
     * @return false 失败
     */
    bool VideoDevice::GetSaturation(long *val, long *Min, long *Max) {
        if (capture_filter == NULL)return false;
        IAMVideoProcAmp *pProcAmp = 0;
        HRESULT hr = capture_filter->QueryInterface(IID_IAMVideoProcAmp, (void**)&pProcAmp);
        if (SUCCEEDED(hr))
        {
            long Step, Default, Flags;
            hr = pProcAmp->GetRange(VideoProcAmp_Saturation, Min, Max, &Step, &Default, &Flags);
            if (SUCCEEDED(hr))
            {
                hr = pProcAmp->Get(VideoProcAmp_Saturation, val, &Flags);
                return SUCCEEDED(hr);
            }
        }
        return false;
    }
    /**
     * @brief 设置饱和度
     *
     * @param val 需要设置的饱和度的值
     * @return true 成功
     * @return false 失败
     */
    bool VideoDevice::SetSaturation(long *val) {
        if (capture_filter == NULL)return false;
        IAMVideoProcAmp *pProcAmp = 0;
        HRESULT hr = capture_filter->QueryInterface(IID_IAMVideoProcAmp, (void**)&pProcAmp);
        if (SUCCEEDED(hr))
        {
            long Min, Max, Step, Default, Flags;
            hr = pProcAmp->GetRange(VideoProcAmp_Saturation, &Min, &Max, &Step, &Default, &Flags);
            if (SUCCEEDED(hr))
            {
                hr = pProcAmp->Set(VideoProcAmp_Saturation, *val, Flags);
                return SUCCEEDED(hr);
            }
        }
        return false;
    }

    /**
     * @brief 停止预览
     *
     */
    void VideoDevice::Stop()
    {
        HRESULT hr;
        if (sourcefilter != NULL)
        {
            hr = sourcefilter->Stop();
            //if (hr < 0) throw hr;
            if (hr < 0)
                return;
        }

        if (samplegrabberfilter != NULL)
        {
            hr = samplegrabberfilter->Stop();
            //if (hr < 0) throw hr;
            if (hr < 0)
                return;
        }

        if (nullrenderer != NULL)
        {
            hr = nullrenderer->Stop();
            //if (hr < 0) throw hr;
            if (hr < 0)
                return;
        }
    }

    //VIDEODEVICE::CALLBACKHANDLER
    //public
    VideoDevice::CallbackHandler::CallbackHandler(VideoDevice *vd)
    {
        callback = 0;
        parent = vd;
    }

    VideoDevice::CallbackHandler::~CallbackHandler()
    {
        delete[] parent;
    }

    void VideoDevice::CallbackHandler::SetCallback(VideoCaptureCallback cb)
    {
        callback = cb;
    }

    HRESULT VideoDevice::CallbackHandler::SampleCB(double time, IMediaSample *sample)
    {
        HRESULT hr;
        unsigned char *buffer;

        hr = sample->GetPointer((BYTE **)&buffer);
        if (hr != S_OK)
            return S_OK;
        DWORD len = sample->GetActualDataLength();

        AM_MEDIA_TYPE *pMtype;
        hr = sample->GetMediaType(&pMtype);
        if (hr == S_OK)
        {
            if (pMtype->formattype == FORMAT_VideoInfo)
            {
                VIDEOINFOHEADER *vInfo1 = (VIDEOINFOHEADER *)pMtype->pbFormat;
                char fourCC[5] = { 0 };
                memcpy(fourCC, &vInfo1->bmiHeader.biCompression, 4);
                printf("pixformat: %s fps: %lld  width: %d height: %d planes: %d",
                    fourCC,
                    10000000LL / vInfo1->AvgTimePerFrame,
                    vInfo1->bmiHeader.biWidth,
                    vInfo1->bmiHeader.biHeight,
                    vInfo1->bmiHeader.biPlanes);
            }
            else if (pMtype->formattype == FORMAT_VideoInfo2)
            {
                VIDEOINFOHEADER2 *vInfo1 = (VIDEOINFOHEADER2 *)pMtype->pbFormat;
                char fourCC[5] = { 0 };
                memcpy(fourCC, &vInfo1->bmiHeader.biCompression, 4);
                printf("pix format: %s fps: %lld width: %d height: %d planers: %d",
                    fourCC, 10000000LL / vInfo1->AvgTimePerFrame,
                    vInfo1->bmiHeader.biWidth,
                    vInfo1->bmiHeader.biHeight,
                    vInfo1->bmiHeader.biPlanes);
            }
            else if (pMtype->formattype == FORMAT_DvInfo)
            {
                printf("unknown type...");
            }
            else if (pMtype->formattype == FORMAT_MPEG2Video)
            {
                printf("unknown type...");
            }
            else if (pMtype->formattype == FORMAT_MPEGStreams)
            {
                printf("unknown type...");
            }
            else if (pMtype->formattype == FORMAT_MPEGVideo)
            {
                printf("unknown type...");
            }
            else if (pMtype->formattype == FORMAT_None)
            {
                printf("unknown type...");
            }
            else if (pMtype->formattype == FORMAT_WaveFormatEx)
            {
                printf("unknown type...");
            }
            else if (pMtype->formattype == GUID_NULL)
            {
                printf("unknown type...");
            }
        }

        if (callback)
            callback(buffer, len, parent->Width, parent->Height, BITS_PER_PIXEL, parent);
        return S_OK;
    }

    HRESULT VideoDevice::CallbackHandler::BufferCB(double time, BYTE *buffer, long len)
    {
        return S_OK;
    }

    HRESULT VideoDevice::CallbackHandler::QueryInterface(const IID &iid, LPVOID *ppv)
    {
        if (iid == IID_ISampleGrabberCB || iid == IID_IUnknown)
        {
            *ppv = (void *)static_cast<ISampleGrabberCB *>(this);
            return S_OK;
        }
        return E_NOINTERFACE;
    }

    ULONG VideoDevice::CallbackHandler::AddRef()
    {
        return 1;
    }

    ULONG VideoDevice::CallbackHandler::Release()
    {
        return 2;
    }

    //VIDEOCAPTURE
    //public
    v4l2Tool::v4l2Tool()
    {
    }
    void v4l2Tool::Init()
    {
        CoInitialize(NULL);

        playing = false;
        current = 0;
        devices = new VideoDevice[MAX_DEVICES];

        InitializeGraph();
        InitializeVideo();

        // we have to use this construct, because other
        // filters may have been added to the graph
        control->Run();
        for (int i = 0; i < num_devices; i++)
        {
            devices[i].Stop();
        }
    }

    v4l2Tool::~v4l2Tool()
    {
        if (graph != NULL)
            graph->Release();
        if (capture != NULL)
            capture->Release();
        if (control != NULL)
            control->Release();
        if (devices != NULL)
        {
            delete[] devices;
            devices = NULL;
        }
    }

    bool clangTools::VideoDevice::Check(char vid[], char pid[])
    {
        if (strlen(vid) != strlen(this->vid))
        {
            return false;
        }
        if (strlen(pid) != strlen(this->pid))
        {
            return false;
        }
        for (size_t i = 0; i < strlen(pid); i++)
        {
            if (this->pid[i] >= 'A' && this->pid[i] <= 'Z')
            {
                this->pid[i] = this->pid[i] - 'A' + 'a'; //+32转换为小写
                                                         //s[i]=s[i]-'A'+'a';
            }
            if (pid[i] >= 'A' && pid[i] <= 'Z')
            {
                pid[i] = pid[i] - 'A' + 'a'; //+32转换为小写
                                             //s[i]=s[i]-'A'+'a';
            }
            if (pid[i] != this->pid[i])
            {
                return false;
            }
        }
        for (size_t i = 0; i < strlen(vid); i++)
        {
            if (this->vid[i] >= 'A' && this->vid[i] <= 'Z')
            {
                this->vid[i] = this->vid[i] - 'A' + 'a'; //+32转换为小写
                                                         //s[i]=s[i]-'A'+'a';
            }
            if (vid[i] >= 'A' && vid[i] <= 'Z')
            {
                vid[i] = vid[i] - 'A' + 'a'; //+32转换为小写
                                             //s[i]=s[i]-'A'+'a';
            }
            if (vid[i] != this->vid[i])
            {
                return false;
            }
        }
        return true;
    }
    VideoDevice *v4l2Tool::GetDevices()
    {
        return devices;
    }

    int v4l2Tool::NumDevices()
    {
        return num_devices;
    }

    //protected
    void v4l2Tool::InitializeGraph()
    {
        HRESULT hr;

        //create the FilterGraph
        hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IFilterGraph2, (void **)&graph);
        //if (hr < 0) throw hr;
        if (hr < 0)
            return;
        //create the CaptureGraphBuilder
        hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void **)&capture);
        //if (hr < 0) throw hr;
        if (hr < 0)
            return;

        //get the controller for the graph
        hr = graph->QueryInterface(IID_IMediaControl, (void **)&control);
        //if (hr < 0) throw hr;
        if (hr < 0)
            return;

        capture->SetFiltergraph(graph);
    }
    /**
     * @brief 设置分辨率
     *
     * @param width
     * @param height
     */
    void v4l2Tool::SetWH(LONG width, LONG height) {
        wWidth = width;
        wHeight = height;
    }
    void v4l2Tool::InitializeVideo()
    {
        HRESULT hr;
        VARIANT name;
        WCHAR filtername[MAX_DEVICE_NAME + 2];

        LONGLONG start = 0, stop = MAXLONGLONG;

        ICreateDevEnum *dev_enum;
        IEnumMoniker *enum_moniker;
        IMoniker *moniker;
        IPropertyBag *pbag;

        //create an enumerator for video input devices
        hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void **)&dev_enum);
        //if (hr < 0) throw hr;
        if (hr < 0)
            return;

        hr = dev_enum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &enum_moniker, (DWORD)NULL);
        //if (hr < 0) throw hr;
        if (hr < 0)
            return;
        if (hr == S_FALSE)
            return; //no devices found

        //get devices (max 8)
        num_devices = 0;
        ULONG got;
        enum_moniker->Reset();
        while (enum_moniker->Next(1, &moniker, &got) == S_OK)
        {
            //get properties
            hr = moniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pbag);
            if (hr >= 0)
            {
                VariantInit(&name);

                //get the description
                hr = pbag->Read(L"Description", &name, 0);
                if (hr < 0)
                    hr = pbag->Read(L"FriendlyName", &name, 0);
                if (hr >= 0)
                {
                    //Initialize the VideoDevice struct
                    VideoDevice *dev = devices + num_devices++;
                    BSTR ptr = name.bstrVal;

                    hr = moniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&dev->capture_filter);
                    if (!SUCCEEDED(hr)) {
                        dev->capture_filter = NULL;
                    }

                    for (int c = 0; *ptr; c++, ptr++)
                    {
                        //bit hacky, but i don't like to include ATL
                        dev->filtername[c] = *ptr;
                        dev->friendlyname[c] = *ptr & 0xFF;
                    }

                    //获取 vid 和 pid
                    BSTR devicePath = NULL;
                    hr = moniker->GetDisplayName(NULL, NULL, &devicePath);
                    if (SUCCEEDED(hr))
                    {
                        wchar_t *lpszDevicePath = _bstr_t(devicePath);

                        std::wstring ws = std::wstring(lpszDevicePath);

                        char *str = new char[ws.length()];
                        memset(str, 0, ws.length());
                        std::copy(ws.begin(), ws.end(), str);

                        sscanf(str, "%*[^#]#%*[^_]_%[^&]", dev->vid);
                        sscanf(str, "%*[^&]&%*[^_]_%[^&]", dev->pid);
                    }

                    //add a filter for the device
                    hr = graph->AddSourceFilterForMoniker(moniker, 0, dev->filtername, &dev->sourcefilter);
                    //if (hr != S_OK) throw hr;
                    if (hr != S_OK)
                        return;

                    //create a samplegrabber filter for the device
                    hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void **)&dev->samplegrabberfilter);
                    //if (hr < 0) throw hr;
                    if (hr < 0)
                        return;

                    //set mediatype on the samplegrabber
                    hr = dev->samplegrabberfilter->QueryInterface(IID_ISampleGrabber, (void **)&dev->samplegrabber);
                    //if (hr != S_OK) throw hr;
                    if (hr != S_OK)
                        return;

                    IAMVideoProcAmp *pProcAmp = 0;
                    long Min, Max, val;
                    hr = dev->samplegrabberfilter->QueryInterface(IID_IAMVideoProcAmp, (void**)&pProcAmp);
                    if (SUCCEEDED(hr))
                    {
                        long Step, Default, Flags;
                        hr = pProcAmp->GetRange(VideoProcAmp_Brightness, &Min, &Max, &Step, &Default, &Flags);
                        if (SUCCEEDED(hr))
                        {
                            hr = pProcAmp->Get(VideoProcAmp_Brightness, &val, &Flags);

                        }
                    }

                    wcscpy(filtername, L"SG ");
                    wcscpy(filtername + 3, dev->filtername);
                    graph->AddFilter(dev->samplegrabberfilter, filtername);

                    //set the media type
                    AM_MEDIA_TYPE mt;
                    memset(&mt, 0, sizeof(AM_MEDIA_TYPE));

                    mt.majortype = MEDIATYPE_Video;
                    mt.subtype = MEDIASUBTYPE_RGB24;
                    // setting the above to 32 bits fails consecutive Select for some reason
                    // and only sends one single callback (flush from previous one ???)
                    // must be deeper problem. 24 bpp seems to work fine for now.

                    hr = dev->samplegrabber->SetMediaType(&mt);
                    //if (hr != S_OK) throw hr;
                    if (hr != S_OK)
                        return;

                    //add the callback to the samplegrabber
                    hr = dev->samplegrabber->SetCallback(dev->callbackhandler, 0);
                    //if (hr != S_OK) throw hr;
                    if (hr != S_OK)
                        return;

                    //set the null renderer
                    hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void **)&dev->nullrenderer);
                    //if (hr < 0) throw hr;
                    if (hr < 0)
                        return;

                    wcscpy(filtername, L"NR ");
                    wcscpy(filtername + 3, dev->filtername);
                    graph->AddFilter(dev->nullrenderer, filtername);

                    //set the render path
#ifdef SHOW_DEBUG_RENDERER
                    hr = capture->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, dev->sourcefilter, dev->samplegrabberfilter, NULL);
#else
                    hr = capture->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, dev->sourcefilter, dev->samplegrabberfilter, dev->nullrenderer);
#endif
                    //if (hr < 0) throw hr;
                    if (hr < 0)
                        return;

                    //if the stream is started, start capturing immediatly
                    hr = capture->ControlStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, dev->sourcefilter, &start, &stop, 1, 2);
                    //if (hr < 0) throw hr;
                    if (hr < 0)
                        return;

                    //reference the graph
                    dev->graph = graph;

                    dev->id = num_devices;

                    IAMStreamConfig *pSC = NULL;
                    VIDEOINFOHEADER *vinfo = 0;
                    hr = capture->FindInterface(&PIN_CATEGORY_CAPTURE,
                        &MEDIATYPE_Video, dev->sourcefilter, IID_IAMStreamConfig, (void **)&pSC);
                    if (!FAILED(hr))
                    {
                        AM_MEDIA_TYPE *mmt = NULL;
                        pSC->GetFormat(&mmt); //取得默认参数
                        VIDEOINFOHEADER *pvih = (VIDEOINFOHEADER *)mmt->pbFormat;

                        dev->Height = pvih->bmiHeader.biHeight;
                        dev->Width = pvih->bmiHeader.biWidth;

                        pvih->bmiHeader.biHeight = wHeight; //修改采集视频的高为240
                        pvih->bmiHeader.biWidth = wWidth;  //修改采集视频的宽为320

                        mmt->pbFormat = (unsigned char *)pvih;
                        hr = pSC->SetFormat(mmt); //重新设置参数
                        if (!FAILED(hr))
                        {
                            dev->Height = pvih->bmiHeader.biHeight;
                            dev->Width = pvih->bmiHeader.biWidth;
                        }
                        else
                        {
                            pvih->bmiHeader.biHeight = dev->Height; //修改采集视频的高为240
                            pvih->bmiHeader.biWidth = dev->Width;   //修改采集视频的宽为320

                            mmt->pbFormat = (unsigned char *)pvih;
                            hr = pSC->SetFormat(mmt); //重新设置参数
                        }
                    }
                    if (pSC != NULL)
                        pSC->Release();
                }
                VariantClear(&name);
                if (pbag != NULL)
                    pbag->Release();
            }
            if (moniker != NULL)
                moniker->Release();
        }
        if (dev_enum != NULL)
            dev_enum->Release();
        if (enum_moniker != NULL)
            enum_moniker->Release();
    }
#else
    // pthread_mutex_t v4l2Tool::testlock = PTHREAD_MUTEX_INITIALIZER;
    // pthread_t v4l2Tool::test_thread = 0;
    // v4l2device *v4l2Tool::dev = NULL;
    // unsigned int v4l2Tool::running = 0;
    // unsigned char *v4l2Tool::frame_buffer = NULL;
    // char v4l2Tool::VideoName[100];


    // 摄像头路径
    static string video2linux = "/sys/class/video4linux";

    std::string &trim(std::string &);

    /**
     * @brief 清除尾部空格
     *
     * @param s
     * @return std::string&
     */
    std::string &trim(std::string &s) {
        if (s.empty()) {
            return s;
        }

        s.erase(0, s.find_first_not_of(" "));
        s.erase(s.find_last_not_of(" ") + 1);
        return s;
    }
    /**
     * @brief 获取摄像头列表
     *
     * @param vd 出參 需要事先分配
     * @param vdLen 入参 vd的分配大小
     * @return int 实际找到的摄像头数
     */
    int v4l2Tool::GetDevices(VideoDevice vd[], int vdLen) {
        int len = 0;
        DIR *dir;
        struct dirent *ptr;
        if ((dir = opendir(video2linux.c_str())) == NULL) {
            logger::instance()->e(TAG, __LINE__, "摄像头信息目录打开出错");
            return -1;
        }
        while (len < vdLen && ((ptr = readdir(dir)) != NULL)) {
            if (strcmp(ptr->d_name, ".") == 0 ||
                strcmp(ptr->d_name, "..") == 0) ///current dir OR parrent dir
                continue;
            else if (ptr->d_type == 8) ///file
            {
                logger::instance()->i(TAG, __LINE__, "文件%s", ptr->d_name);
                continue;
            } else if (ptr->d_type == 10) ///link file
            {
                logger::instance()->i(TAG, __LINE__, "链接 %s", ptr->d_name);
            } else if (ptr->d_type == 4) ///dir
            {
                logger::instance()->i(TAG, __LINE__, "目录 %s", ptr->d_name);
            } else {
                continue;
            }
            string sub_dir = video2linux + "/" + ptr->d_name;

            string videoPath = string("/dev/") + ptr->d_name;

            memset(vd[len].path, 0x00, 256);
            memcpy(vd[len].path, videoPath.c_str(), videoPath.size());

            logger::instance()->i(TAG, __LINE__, "摄像头路径 %s", (videoPath).c_str());

            string s;
            string name;
            ifstream filestr;
            filestr.open((sub_dir + "/name").c_str(), ios::binary);
            while (filestr >> s) {
                name += s + " ";
            }
            trim(s);

            filestr.close();
            memset(vd[len].name, 0x00, 50);
            memcpy(vd[len].name, name.c_str(), name.size());
            logger::instance()->i(TAG, __LINE__, "摄像头名 %s", name.c_str());

            DIR *dir1;
            struct dirent *ptr1;
            if ((dir1 = opendir((sub_dir + "/device/input").c_str())) == nullptr) {
                logger::instance()->e(TAG, __LINE__, "摄像头信息目录打开出错 %s ", (sub_dir + "/device/input").c_str());
                continue;
            }
            while ((ptr1 = readdir(dir1)) != nullptr) {
                if (strcmp(ptr1->d_name, ".") == 0 ||
                    strcmp(ptr1->d_name, "..") == 0) ///current dir OR parrent dir
                    continue;
                else if (ptr1->d_type == 10) ///link file
                {
                    logger::instance()->i(TAG, __LINE__, "链接 %s", ptr1->d_name);
                } else if (ptr1->d_type == 4) ///dir
                {
                    logger::instance()->i(TAG, __LINE__, "目录 %s", ptr1->d_name);
                } else {
                    continue;
                }
                string vidPath = sub_dir + "/device/input/" + ptr1->d_name + "/id/vendor";
                string pidPath = sub_dir + "/device/input/" + ptr1->d_name + "/id/product";

                string s1;
                string vid, pid;
                ifstream filestr1;
                filestr1.open((vidPath).c_str(), ios::binary);
                while (filestr1 >> s1) {
                    vid += s1 + " ";
                }
                filestr1.close();
                filestr1.open((pidPath).c_str(), ios::binary);
                while (filestr1 >> s1) {
                    pid += s1 + " ";
                }
                filestr1.close();
                trim(vid);
                trim(pid);

                memset(vd[len].vid, 0x00, 10);
                memcpy(vd[len].vid, vid.c_str(), vid.size());
                memset(vd[len].pid, 0x00, 10);
                memcpy(vd[len].pid, pid.c_str(), pid.size());
                logger::instance()->i(TAG, __LINE__, "vid : %s ;pid : %s", vid.c_str(), pid.c_str());
            }
            closedir(dir1);
            len++;
        }
        closedir(dir);

        return len;
    }

    /**
     * @brief 设置摄像头 函数
     *
     * @param fd
     * @param request
     * @param arg
     * @return int
     */
    int v4l2Tool::xioctl(int fd, int request, void *arg) {
        int r;
        do
            r = ioctl(fd, request, arg);
        while (-1 == r && EINTR == errno);
        return r;
    }

    /**
     * @brief 获取摄像头数据
     *
     * @return int
     */
    int v4l2Tool::v4l2_capabilities() {
        /*
        * print capabilites of camera
        */
        struct v4l2_capability caps;
        if (xioctl(dev->fd, VIDIOC_QUERYCAP, &caps) == -1) {
            logger::instance()->e(TAG, __LINE__, "Querying Capabilities");
            return 1;
        }
        logger::instance()->d(TAG, __LINE__, "Driver Caps:"
                                                      "  Driver: \"%s\""
                                                      "  Card: \"%s\""
                                                      "  Bus: \"%s\""
                                                      "  Version: %d.%d"
                                                      "  Capabilities: %08x",
                              caps.driver,
                              caps.card,
                              caps.bus_info,
                              (caps.version >> 16) & 0xff,
                              (caps.version >> 24) & 0xff,
                              caps.capabilities);

        // if (strcmp((char *)caps.card, VideoName) != 0)
        // {
        // 	return 1;
        // }

        struct v4l2_fmtdesc fmtdesc;
        fmtdesc.index = 0;
        fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        char c, e;
        char fourcc[5];
        logger::instance()->d(TAG, __LINE__, "  FMT : CE Desc");
        while (0 == xioctl(dev->fd, VIDIOC_ENUM_FMT, &fmtdesc)) {
            strncpy(fourcc, (char *) &fmtdesc.pixelformat, 4);
            c = fmtdesc.flags & 1 ? 'C' : ' ';
            e = fmtdesc.flags & 2 ? 'E' : ' ';
//            logger::instance()->d(__FILENAME__,__LINE__, "  %s: %c%c %s", fourcc, c, e, fmtdesc.description);
            fmtdesc.index++;
        }
        return 0;
    }

    /**
     * @brief 设置摄像头输入
     *
     * @return int
     */
    int v4l2Tool::v4l2_set_input() {
        /*
        * define video input
        * vfe_v4l2 the driver is forced to input = -1
        * set as the input = 0, works fine.
        */
        struct v4l2_input input;
        int count = 0;
        CLEAR(input);
        input.index = count;
        while (!xioctl(dev->fd, VIDIOC_ENUMINPUT, &input)) {
            input.index = ++count;
        }
        count -= 1;

        assert(count > -1);

        if (xioctl(dev->fd, VIDIOC_S_INPUT, &count) == -1) {
            logger::instance()->d(TAG, __LINE__, "Error selecting input %d", count);
            return 1;
        }
        return 0;
    }

    /**
     * @brief 设置格式 分辨率之类
     *
     * @return int
     */
    int v4l2Tool::v4l2_set_pixfmt() {
        /*
        * define pixel format
        * in gc2035 driver, tested with:
        * -- 422P/YU12/YV12/NV16/NV12/NV61/NV21/UYVY
        * others will be ignored
        *
        */
        struct v4l2_format fmt;
        CLEAR(fmt);

        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width = dev->width;   // ignored by gc2035 driver
        fmt.fmt.pix.height = dev->height; // ignored by gc2035 driver
        fmt.fmt.pix.pixelformat = dev->fmt;
        fmt.fmt.pix.field = V4L2_FIELD_ANY;
        if (-1 == xioctl(dev->fd, VIDIOC_S_FMT, &fmt)) {
            logger::instance()->e(TAG, __LINE__, "Setting Pixel Format");
            return 1;
        }
        logger::instance()->d(TAG, __LINE__, "Selected Camera Mode:"
                                                      "  Width: %d"
                                                      "  Height: %d"
                                                      "  PixFmt: %s",
                              fmt.fmt.pix.width,
                              fmt.fmt.pix.height,
                              (char *) &fmt.fmt.pix.pixelformat);

        dev->width = fmt.fmt.pix.width;
        dev->height = fmt.fmt.pix.height;
        logger::instance()->d(TAG, __LINE__, "Selected Camera Mode:"
                                                      "  Width: %d"
                                                      "  Height: %d",
                              dev->width,
                              dev->height);

        if (dev->fmt != fmt.fmt.pix.pixelformat) {
            logger::instance()->e(TAG, __LINE__, "Pix format not accepted");
            return 1;
        }
        logger::instance()->i(TAG, __LINE__, "Pix format is accepted");
        return 0;
    }

    /**
     * @brief 设置相机帧速率
     *
     * @return int
     */
    int v4l2Tool::v4l2_set_fps() {
        /*
        * set camera frame rate
        */
        struct v4l2_streamparm setfps;
        CLEAR(setfps);
        setfps.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        setfps.parm.capture.timeperframe.numerator = 1;
        setfps.parm.capture.timeperframe.denominator = dev->fps;
        if (xioctl(dev->fd, VIDIOC_S_PARM, &setfps) == -1) {
            logger::instance()->e(TAG, __LINE__, "Error setting frame rate");
            return 1;
        }
        dev->fps = setfps.parm.capture.timeperframe.denominator;
        return 0;
    }

    /**
     * @brief 设置内存映射模式缓冲器
     *
     * @return int
     */
    int v4l2Tool::v4l2_init_mmap() {
        /*
        * setup memory map mode buffers
        */
        struct v4l2_requestbuffers req;
        CLEAR(req);
        req.count = dev->n_buffers;
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_MMAP;
        if (-1 == xioctl(dev->fd, VIDIOC_REQBUFS, &req)) {
            logger::instance()->e(TAG, __LINE__, "Requesting Buffer");
            return 1;
        }
        dev->buffer = (buffer_t *) malloc(req.count * sizeof(buffer_t));
        if (!dev->buffer) {
            logger::instance()->e(TAG, __LINE__, "Out of memory");
            dev->buffer = NULL;
            return 1;
        }
        unsigned int i;
        for (i = 0; i < req.count; i++) {
            struct v4l2_buffer buf;
            CLEAR(buf);
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;
            buf.index = i;
            if (xioctl(dev->fd, VIDIOC_QUERYBUF, &buf) == -1) {
                logger::instance()->e(TAG, __LINE__, "VIDIOC_QUERYBUF");
                return 1;
            }
            dev->buffer[i].length = buf.length;
            dev->buffer[i].data = (unsigned char *) mmap(nullptr, buf.length, PROT_READ | PROT_WRITE,
                                                         MAP_SHARED, dev->fd, buf.m.offset);
            if (dev->buffer[i].data == MAP_FAILED) {
                for (i--; i >= 0; i--) {
                    munmap(dev->buffer[i].data, dev->buffer[i].length);
                    dev->buffer[i].data = nullptr;
                }
                free(dev->buffer);
                dev->buffer = nullptr;
                logger::instance()->e(TAG, __LINE__, "Error mapping buffer");
                return 1;
            }
        }
        return 0;
    }

    /**
     * @brief 开始预览
     *
     * @return int
     */
    int v4l2Tool::prepare_cap() {
        /*
        * preparing to capture frames
        */
        unsigned int i;
        for (i = 0; i < dev->n_buffers; ++i) {
            struct v4l2_buffer buf;
            CLEAR(buf);
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;
            buf.index = i;
            if (xioctl(dev->fd, VIDIOC_QBUF, &buf) < 0) {
                logger::instance()->e(TAG, __LINE__, "VIDIOC_QBUF");
                return 1;
            }
        }
        enum v4l2_buf_type type;
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (xioctl(dev->fd, VIDIOC_STREAMON, &type) < 0) {
            logger::instance()->e(TAG, __LINE__, "VIDIOC_STREAMON");
            return 1;
        }
        return 0;
    }

    /**
     * @brief 停止预览
     *
     * @return int
     */
    int v4l2Tool::stop_capturing() {
        if (dev == NULL)
            return 0;
        /*
                         * stopping capture and free nmap buffers
                         */
        enum v4l2_buf_type type;
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (xioctl(dev->fd, VIDIOC_STREAMOFF, &type) < 0) {
            logger::instance()->e(TAG, __LINE__, "VIDIOC_STREAMOFF");
            return 1;
        }
        unsigned int i;
        for (i = 0; i < dev->n_buffers && dev->buffer != nullptr; ++i) {
            if (munmap(dev->buffer[i].data, dev->buffer[i].length) < 0) {
                logger::instance()->e(TAG, __LINE__, "munmap");
                return 1;
            }
        }
        return 0;
    }

    /**
     * @brief 拍照，保存到路径
     *
     * @param BmpPath 需要保存的路径，进支持bmp格式
     * @return int < 0 保存失败
     */
    int v4l2Tool::grabFrame(const char *BmpPath) {
        logger::instance()->d(TAG, __LINE__, "be capturing..");
        int ret = pthread_mutex_lock(&testlock);
        if (ret != 0)logger::instance()->d(TAG, __LINE__, "获取锁返回值 ：%d", ret);
        logger::instance()->d(TAG, __LINE__, "capturing..");
        if (frame_buffer == NULL || dev == NULL) {
            pthread_mutex_unlock(&testlock);
            return -998;
        }
        logger::instance()->d(TAG, __LINE__, "capturing..");

        BMP bmp;
        bmp.SetData(dev->width, dev->height, 24, frame_buffer);
        if (isMirror) {
            bmp.SetMirror(1);
        }
        bool flag = bmp.WriteBmp(BmpPath);
        pthread_mutex_unlock(&testlock);
        logger::instance()->d(TAG, __LINE__, "capture ok");
        return flag ? 0 : -1001;
    }

    /**
     * @brief 获取相机数据
     *
     * @param colors 像素数据
     * @param BitPerPixel 位数
     * @return int
     */
    int v4l2Tool::grabFrame(unsigned char *colors, int BitPerPixel) {
        // logger::instance()->d(TAG, __LINE__, "be capturing..");
        int ret = pthread_mutex_lock(&testlock);
        if (ret != 0)logger::instance()->d(TAG, __LINE__, "获取锁返回值 ：%d", ret);
        if (frame_buffer == NULL || running == 0) {
            pthread_mutex_unlock(&testlock);
            return -998;
        }
        // logger::instance()->d(TAG, __LINE__, "capturing..");

#ifndef ANDROID_SO
        BMP bmp;
        bmp.SetData(dev->width, dev->height, 24, frame_buffer);
        if (isMirror) {
            bmp.SetMirror(1);
        }
        bmp.SetMirror(2);
        // memcpy(colors, bmp.GetData(), bmp.GetDataSize());
        ret = bmp.WriteBmp(colors);
        pthread_mutex_unlock(&testlock);
        return ret;
#else
        unsigned char *lrgb = NULL;
        lrgb = &frame_buffer[0];
        int width = dev->width;
        int height = dev->height;
#ifdef ANDROID_SO
        isMirror = false;
#endif
        if (isMirror) {
            int __i = 3;
            int __j = (BitPerPixel == 32) ? 4 : 3;
            for (int i = 0; i < width * height; i++) {
                int _h = i / width;
                int _w = i % width;

                int index = width * _h * __j + _w * __j;
                int index1 = width * (_h + 1) * __i - (_w - 1) * __i;

#ifndef ANDROID_SO
                for (int _i = 0; _i < 3; _i++)
                    colors[index + _i] = lrgb[index1 + _i];
#else
                index = width * (_h + 1) * __i - (_w - 1) * __i;
                colors[index1 + 2] = lrgb[index - 3];
                colors[index1 + 1] = lrgb[index - 2];
                colors[index1 + 0] = lrgb[index - 1];
#endif
                if (__j == 4)
                    colors[index + 3] = 255;
            }
        } else {
            int __i = 3;
            int __j = (BitPerPixel == 32) ? 4 : 3;

            for (int i = 0; i < width * height; i++) {
                int _h = i / width;
                int _w = i % width;

                int index = width * _h * __i + _w * __i;
                int index1 = width * _h * __j + _w * __j;
#ifndef ANDROID_SO
                for (int _i = 0; _i < 3; _i++)
                    colors[index1 + _i] = lrgb[index + _i];
#else
                index = width * (_h + 1) * __i - (_w - 1) * __i;
                colors[index1 + 2] = lrgb[index - 3];
                colors[index1 + 1] = lrgb[index - 2];
                colors[index1 + 0] = lrgb[index - 1];
#endif
                if (__j == 4) {
                    colors[index1 + 3] = 255;
                }
            }
        }
#endif

        pthread_mutex_unlock(&testlock);
        return 0;
    }

    /**
     * @brief 配置DST视频设备
     *
     * @return int
     */
    int v4l2Tool::v4l2loopbackDevice() {
        /*
        * configure dst video device
        */
        // dev->fdout_devname = open(dev->out_devname, O_RDWR | O_NONBLOCK, 0);
        dev->fdout_devname = open(dev->out_devname, O_RDWR, 0);
        if (dev->fdout_devname == -1) {
            logger::instance()->e(TAG, __LINE__, "v4l2loopback: Opening virtual video device");
            return 1;
        }
        struct v4l2_capability vid_caps;
        struct v4l2_format vid_format;
        CLEAR(vid_format);

        if (xioctl(dev->fdout_devname, VIDIOC_QUERYCAP, &vid_caps) < 0) {
            logger::instance()->e(TAG, __LINE__, "v4l2loopback: VIDIOC_QUERYCAP");
            return 1;
        }
        vid_format.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
        vid_format.fmt.pix.width = dev->width;
        vid_format.fmt.pix.height = dev->height;
        vid_format.fmt.pix.pixelformat = dev->fmt;
        vid_format.fmt.pix.field = V4L2_FIELD_ANY;
        if (xioctl(dev->fdout_devname, VIDIOC_S_FMT, &vid_format) < 0) {
            logger::instance()->e(TAG, __LINE__, "v4l2loopback: VIDIOC_S_FMT");
            return 1;
        }

        struct v4l2_streamparm setfps;
        CLEAR(setfps);
        setfps.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        setfps.parm.capture.timeperframe.numerator = 1;
        setfps.parm.capture.timeperframe.denominator = dev->fps;
        if (xioctl(dev->fdout_devname, VIDIOC_S_PARM, &setfps) == -1) {
            logger::instance()->e(TAG, __LINE__, "v4l2loopback: Error setting frame rate");
            return 1;
        }
        return 0;
    }

    /**
     * @brief Construct a new v4l2 Tool::v4l2 Tool object
     *
     */
    v4l2Tool::v4l2Tool() {
        testlock = PTHREAD_MUTEX_INITIALIZER;

        // test_thread = 0;
        // dev = NULL;
        // running = 0;
        // frame_buffer = NULL;
        // VIDIOC_DQBUF_Fail_Flag = false;

        width = 640;
        height = 480;
        fps = 10;
        test_thread = new pthread_t;
        pthread_mutex_init(&testlock, NULL);
    }

    /**
     * @brief 获取白平衡模式
     *
     * @param fd 摄像头设备
     * @return int 摄像头值
     */
    int v4l2Tool::GetAutoWhiteBalance() {
        int fd = dev->fd;
        if (fd <= 0)return -999;
        struct v4l2_control ctrl = {0};
        memset(&ctrl, 0, sizeof(ctrl));
        ctrl.id = V4L2_CID_AUTO_WHITE_BALANCE;
        if (-1 == xioctl(fd, VIDIOC_G_CTRL, &ctrl)) {
            logger::instance()->e(TAG, __LINE__, "V4L2_CID_AUTO_WHITE_BALANCE Fail");
        }
        return ctrl.value;
    }

    /**
     * @brief 设置摄像头白平衡
     *
     * @param fd 摄像头设备
     * @param enable 设置摄像头模式
     * @return int 是否设置成功
     */
    int v4l2Tool::SetAutoWhiteBalance(int enable) {
        int fd = dev->fd;
        if (fd <= 0)return -999;
        struct v4l2_control ctrl = {0};
        memset(&ctrl, 0, sizeof(ctrl));
        ctrl.id = V4L2_CID_AUTO_WHITE_BALANCE;
        ctrl.value = enable;
        if (-1 == xioctl(fd, VIDIOC_S_CTRL, &ctrl)) {
            logger::instance()->e(TAG, __LINE__, "V4L2_CID_AUTO_WHITE_BALANCE Fail");
        }
        return GetAutoWhiteBalance() == enable;
    }

    /**
     * @brief 获取亮度
     *
     * @param fd 摄像头设备
     * @return int 亮度值
     */
    int v4l2Tool::GetBrightness() {
        int fd = dev->fd;
        if (fd <= 0)return -999;
        struct v4l2_control ctrl = {0};
        memset(&ctrl, 0, sizeof(ctrl));
        ctrl.id = V4L2_CID_BRIGHTNESS;
        if (-1 == xioctl(fd, VIDIOC_G_CTRL, &ctrl)) {
            logger::instance()->e(TAG, __LINE__, "V4L2_CID_BRIGHTNESS : 失败 ");
            return -999;
        }
        logger::instance()->i(TAG, __LINE__, "V4L2_CID_BRIGHTNESS = 0x%x", ctrl.value);
        return ctrl.value;
    }

    /**
     * @brief 设置摄像头亮度
     *
     * @param fd 摄像头设备
     * @param enable 设置亮度值
     *      struct v4l2_queryctrl queryctrl
     *      queryctrl.default_value;
     *
     * @return int 是否设置成功
     */
    int v4l2Tool::SetBrightness(int enable) {
        int fd = dev->fd;
        if (fd <= 0)return -999;
        struct v4l2_control ctrl = {0};
        memset(&ctrl, 0, sizeof(ctrl));
        ctrl.id = V4L2_CID_BRIGHTNESS;
        ctrl.value = enable; // queryctrl.default_value;
        if (-1 == xioctl(fd, VIDIOC_S_CTRL, &ctrl)) {
            logger::instance()->e(TAG, __LINE__, "V4L2_CID_BRIGHTNESS Fail");
            return -999;
        }
        return GetBrightness() == enable;
    }

    /**
     * @brief 获取对比度
     *
     * @param fd 摄像头设备
     * @return int 对比度
     */
    int v4l2Tool::GetContrast() {
        int fd = dev->fd;
        if (fd <= 0)return -999;
        struct v4l2_control ctrl = {0};
        memset(&ctrl, 0, sizeof(ctrl));
        ctrl.id = V4L2_CID_CONTRAST;
        if (-1 == xioctl(fd, VIDIOC_G_CTRL, &ctrl)) {
            logger::instance()->e(TAG, __LINE__, "V4L2_CID_CONTRAST : 失败 ");
            return -999;
        }
        logger::instance()->i(TAG, __LINE__, "V4L2_CID_CONTRAST = 0x%x", ctrl.value);
        return ctrl.value;
    }

    /**
     * @brief 设置摄像头对比度
     *
     * @param fd 摄像头设备
     * @param enable 设置对比度
     *
     * @return int 是否设置成功
     */
    int v4l2Tool::SetContrast(int enable) {
        int fd = dev->fd;
        if (fd <= 0)return -999;
        struct v4l2_control ctrl = {0};
        memset(&ctrl, 0, sizeof(ctrl));
        ctrl.id = V4L2_CID_CONTRAST;
        ctrl.value = enable; // queryctrl.default_value;
        if (-1 == xioctl(fd, VIDIOC_S_CTRL, &ctrl)) {
            logger::instance()->e(TAG, __LINE__, "V4L2_CID_CONTRAST Fail");
            return -999;
        }
        return GetBrightness() == enable;
    }

    /**
     * @brief 开启摄像头
     *
     * @param devname 摄像头路径
     * @return int < 0 失败
     */
    int v4l2Tool::Open(const char *devname) {

        Close();
        logger::instance()->d(TAG, __LINE__, "OpenVideo : %s", devname);
        running = 0;
        dev = (v4l2device *) malloc(sizeof(v4l2device));
        CLEAR(*dev);
        unsigned int retcode = 0;


        // default values
        dev->fd = -1;
        dev->fdout_devname = -1;
        dev->width = width;   //1024; //2592;
        dev->height = height; //768; // 1944;
        dev->fps = fps;
        dev->n_buffers = 4;
        dev->timeout = 5;
        dev->in_devname = (char *) devname;

        dev->fmt = V4L2_PIX_FMT_YUYV;


        // print args
        logger::instance()->d(TAG, __LINE__, "Required width: %d", dev->width);
        logger::instance()->d(TAG, __LINE__, "Required height: %d", dev->height);
        logger::instance()->d(TAG, __LINE__, "Required FPS: %d", dev->fps);
        logger::instance()->d(TAG, __LINE__, "input device: %s", dev->in_devname);
        logger::instance()->d(TAG, __LINE__, "output device: %s", dev->out_devname);

        // dev->fd = open(dev->in_devname, O_RDWR | O_NONBLOCK, 0);
        dev->fd = open(dev->in_devname, O_RDWR, 0);
        logger::instance()->i(TAG, __LINE__, "video fd : %d ", dev->fd);
        // if (dev->fd <=0)
        // {
        //     logger::instance()->e(__FILENAME__,__LINE__, "Opening video device");
        //     close(dev->fd);
        //     dev->fd = open(dev->in_devname, O_RDWR | O_NONBLOCK, 0);

        if (dev->fd <= 0) {
            logger::instance()->e(TAG, __LINE__, "Opening video device");
            Close();
            return -1;
        }

        // }

        // 	check device capabilites
        retcode = v4l2_capabilities();
        if (retcode) {
            logger::instance()->e(TAG, __LINE__, "%d", retcode);
            Close();
            return -1;
        }

        // 	set input
        retcode = v4l2_set_input();
        if (retcode) {
            logger::instance()->e(TAG, __LINE__, "%d", retcode);
            Close();
            return -1;
        }

        // 	set pixformat
        retcode = v4l2_set_pixfmt();
        if (retcode) {
            logger::instance()->e(TAG, __LINE__, "%d", retcode);
            Close();
            return -1;
        }
        // 	set framerate
        retcode = v4l2_set_fps();
        if (retcode) {
            logger::instance()->e(TAG, __LINE__, "%d", retcode);
            Close();
            return -1;
        }

        // 	allocate buffers
        retcode = v4l2_init_mmap();
        if (retcode) {
            logger::instance()->e(TAG, __LINE__, "%d", retcode);
            Close();
            return -1;
        }

        // 	prepare to grab frames
        retcode = prepare_cap();
        if (retcode) {
            logger::instance()->e(TAG, __LINE__, "%d", retcode);
            Close();
            return -1;
        }

        logger::instance()->i(TAG, __LINE__, "摄像头开启成功");
        running = 1;
        isClose = false;
        // pthread_attr_t attr; //attributes of the thread   //from base code

        // /* Setup threads to find prime numbers */
        // pthread_attr_init(&attr);  //from base code

        try {
            pthread_create(test_thread, NULL, &ReadVideo, (void *) this);
        }
        catch (...) {
            logger::instance()->e(TAG, __LINE__, "出现异常");
            return -1;
        }
        usleep(100 * 1000);
        logger::instance()->i(TAG, __LINE__, "摄像头开启成功End");
        return 0;
    }

    /**
     * @brief YUYV 数据转 RGB 数据格式
     *
     * @param pointer
     * @param frame_buffer
     * @param width_2
     * @param i
     * @param j
     */
    void v4l2Tool::yuyv_to_rgb_G(unsigned char *pointer, unsigned char *frame_buffer,
                                 int width_2, int i, int j) {
        unsigned char y1, y2, u, v;
        int r1, g1, b1, r2, g2, b2;
        y1 = *(pointer + (i * (dev->width / 2) + j) * 4);
        u = *(pointer + (i * (dev->width / 2) + j) * 4 + 1);
        y2 = *(pointer + (i * (dev->width / 2) + j) * 4 + 2);
        v = *(pointer + (i * (dev->width / 2) + j) * 4 + 3);

        r1 = static_cast<int>(y1 + 1.042 * (v - 128));
        g1 = static_cast<int>(y1 - 0.34414 * (u - 128) - 0.71414 * (v - 128));
        b1 = static_cast<int>(y1 + 1.772 * (u - 128));

        r2 = static_cast<int>(y2 + 1.042 * (v - 128));
        g2 = static_cast<int>(y2 - 0.34414 * (u - 128) - 0.71414 * (v - 128));
        b2 = static_cast<int>(y2 + 1.772 * (u - 128));

        if (r1 > 255)
            r1 = 255;
        else if (r1 < 0)
            r1 = 0;
        if (b1 > 255)
            b1 = 255;
        else if (b1 < 0)
            b1 = 0;
        if (g1 > 255)
            g1 = 255;
        else if (g1 < 0)
            g1 = 0;
        if (r2 > 255)
            r2 = 255;
        else if (r2 < 0)
            r2 = 0;
        if (b2 > 255)
            b2 = 255;
        else if (b2 < 0)
            b2 = 0;
        if (g2 > 255)
            g2 = 255;
        else if (g2 < 0)
            g2 = 0;

        *(frame_buffer + ((dev->height - 1 - i) * (dev->width / 2) + width_2 - j) * 6 +
          0) = (unsigned char) b2;
        *(frame_buffer + ((dev->height - 1 - i) * (dev->width / 2) + width_2 - j) * 6 +
          1) = (unsigned char) g2;
        *(frame_buffer + ((dev->height - 1 - i) * (dev->width / 2) + width_2 - j) * 6 +
          2) = (unsigned char) r2;
        *(frame_buffer + ((dev->height - 1 - i) * (dev->width / 2) + width_2 - j) * 6 +
          3) = (unsigned char) b1;
        *(frame_buffer + ((dev->height - 1 - i) * (dev->width / 2) + width_2 - j) * 6 +
          4) = (unsigned char) g1;
        *(frame_buffer + ((dev->height - 1 - i) * (dev->width / 2) + width_2 - j) * 6 +
          5) = (unsigned char) r1;
    }

    /**
     * @brief  YUYV 数据转 RGB 数据格式
     *
     * @param pointer YUYV 数据
     * @param frame_buffer 转换后的RGB格式
     */
    void v4l2Tool::yuyv_to_rgb(unsigned char *pointer, unsigned char *frame_buffer) {
        unsigned int i;
        int j;
        for (i = 0; i < dev->height; i++) {
            int width_2 = dev->width / 2 - 1;

            // if (isMirror)
            //     for (j = (dev->width / 2)-1; j >=0; j--)
            //         yuyv_to_rgb_G(pointer,frame_buffer,width_2,i,j);
            // else
            for (j = 0; j < (dev->width / 2); j++)
                yuyv_to_rgb_G(pointer, frame_buffer, width_2, i, j);
        }

    }

    /**
     * @brief 获取摄像头数据线程
     *
     * @param arg 当前对象指针
     * @return void*
     */
    void *v4l2Tool::ReadVideo(void *arg) {
        v4l2Tool *that = (v4l2Tool *) arg;
        if (that->frame_buffer != NULL) {
            free(that->frame_buffer);
            that->frame_buffer = NULL;
        }
        that->frame_buffer = (unsigned char *) malloc(
                (that->dev->width * that->dev->height * 3) * sizeof(unsigned char));
        while (!that->isClose) {
            // logger::instance()->i(__FILENAME__,__LINE__, "VIDIOC_QBUF run");
            for (int i = 0; i < 100 * 5; i++) {
                usleep(200);
                if (that->isClose) {
                    break;
                }
            }
            if (that->isClose) {
                break;
            }
            try {
                int ret = pthread_mutex_lock(&that->testlock);
                if (ret != 0)logger::instance()->d(TAG, __LINE__, "获取锁返回值 ：%d", ret);
            }
            catch (...) {
                continue;
            }
            /*
                            * read and processing image and write into output
                            */
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(that->dev->fd, &fds);
            struct timeval tv;
            CLEAR(tv);
            tv.tv_sec = that->dev->timeout;
            tv.tv_usec = 0;
            int r = select(that->dev->fd + 1, &fds, NULL, NULL, &tv);
            if (-1 == r) {
                logger::instance()->e(TAG, __LINE__, "Waiting for Frame");
                pthread_mutex_unlock(&that->testlock);
                continue;
            }
            struct v4l2_buffer buf;
            CLEAR(buf);
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;
            // dequeue buffer
            if (that->xioctl(that->dev->fd, VIDIOC_DQBUF, &buf) < 0) {
                logger::instance()->e(TAG, __LINE__, "VIDIOC_DQBUF");
                if (that->VIDIOC_DQBUF_Fail_Flag) {
                    that->VIDIOC_DQBUF_Fail_Flag = false;
                    pthread_mutex_unlock(&that->testlock);
                    break;
                }
                that->VIDIOC_DQBUF_Fail_Flag = true;
                pthread_mutex_unlock(&that->testlock);
                logger::instance()->e(TAG, __LINE__, "VIDIOC_DQBUF continue");
                continue;
            }
            that->VIDIOC_DQBUF_Fail_Flag = false;
            assert(buf.index < that->dev->n_buffers);

            // processing buffer

            //	fwrite(dev->buffer[buf.index].data, buf.bytesused, 1, stdout);

            unsigned char *pointer;

            pointer = that->dev->buffer[buf.index].data;
            that->yuyv_to_rgb(pointer, that->frame_buffer);
            // put buffer
            if (that->xioctl(that->dev->fd, VIDIOC_QBUF, &buf) < 0) {
                logger::instance()->e(TAG, __LINE__, "VIDIOC_QBUF");
                pthread_mutex_unlock(&that->testlock);
                continue;
            }
            pthread_mutex_unlock(&that->testlock);
            // logger::instance()->i(__FILENAME__,__LINE__, "VIDIOC_QBUF once");
        }
        that->running = 0;

        if (that->frame_buffer != NULL) {
            int ret = pthread_mutex_lock(&that->testlock);
            if (ret != 0)logger::instance()->d(TAG, __LINE__, "获取锁返回值 ：%d", ret);
            logger::instance()->d(TAG, __LINE__, "free frame_buffer");
            if (that->frame_buffer != NULL)
                free(that->frame_buffer);
            logger::instance()->d(TAG, __LINE__, "freeEnd frame_buffer");
            that->frame_buffer = NULL;
            pthread_mutex_unlock(&that->testlock);
        }

        logger::instance()->d(TAG, __LINE__, "VIDIOC_QBUF is exit");
        pthread_exit(NULL);
        return NULL;
    }

    /**
     * @brief 关闭摄像头
     *
     */
    void v4l2Tool::Close() {
        logger::instance()->d(TAG, __LINE__, "Close");
        isClose = true;
        void *status;
        while (running != 0) {
            pthread_join(*test_thread, &status);
        }
        stop_capturing();

        int ret = pthread_mutex_lock(&testlock);
        if (ret != 0)logger::instance()->d(TAG, __LINE__, "获取锁返回值 ：%d", ret);
        // if (frame_buffer != NULL)
        // {
        //     // logger::instance()->d("free","file : %s , line : %d",__FILE__,__LINE__);
        //     free(frame_buffer);
        //     // logger::instance()->d("freeE","file : %s , line : %d",__FILE__,__LINE__);
        //     frame_buffer = NULL;
        // }

        if (dev != nullptr) {
            // logger::instance()->i(__FILENAME__,__LINE__, "video fd : %d ",dev->fd);
            close(dev->fd);
            close(dev->fd);
            if (dev->buffer != nullptr) {
                for (int _i = 0; _i < dev->n_buffers; _i++) {
                    if (dev->buffer[_i].data != nullptr)
                        munmap(dev->buffer[_i].data, dev->buffer[_i].length);
                    dev->buffer[_i].data = nullptr;
                }
                free(dev->buffer);
                dev->buffer = nullptr;
            }
            // logger::instance()->d("free","file : %s , line : %d",__FILE__,__LINE__);
            free(dev);
            // logger::instance()->d("freeE","file : %s , line : %d",__FILE__,__LINE__);
            dev = nullptr;
        }
        pthread_mutex_unlock(&testlock);
    }

    /**
     * @brief Destroy the v4l2 Tool::v4l2 Tool object
     *
     */
    v4l2Tool::~v4l2Tool() {
        Close();
        pthread_mutex_destroy(&testlock);
        delete test_thread;
        return;
    }

    // void v4l2Tool::SetVideoName(char *name)
    // {
    // 	memset(VideoName, 0x00, sizeof(VideoName));
    // 	memcpy(VideoName, name, strlen(name));
    // 	return;
    // }

#endif // !WIN32
} // namespace clangTools

#endif
