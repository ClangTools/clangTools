#ifndef __V4L2TOOL__H__
#define __V4L2TOOL__H__
#ifndef __APPLE__
// #pragma pack(2)
#ifdef WIN32
#pragma warning(disable : 4995)
#pragma warning(disable : 4996)
#pragma comment(lib, "strmiids")
#define SKIP_DXTRANS
//#define SHOW_DEBUG_RENDERER
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <windows.h>
#include <dshow.h>
#include <dvdmedia.h>
#pragma include_alias("dxtrans.h", "qedit.h")
#define __IDxtCompositor_INTERFACE_DEFINED__
#define __IDxtAlphaSetter_INTERFACE_DEFINED__
#define __IDxtJpeg_INTERFACE_DEFINED__
#define __IDxtKey_INTERFACE_DEFINED__
#ifndef __qedit_h__
#define __qedit_h__
interface ISampleGrabberCB : public IUnknown
{
	virtual STDMETHODIMP SampleCB(double SampleTime, IMediaSample *pSample) = 0;
	virtual STDMETHODIMP BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen) = 0;
};
static const IID IID_ISampleGrabberCB = {0x0579154A, 0x2B53, 0x4994, {0xB0, 0xD0, 0xE7, 0x73, 0x14, 0x8E, 0xFF, 0x85}};
interface ISampleGrabber : public IUnknown
{
	virtual HRESULT STDMETHODCALLTYPE SetOneShot(BOOL OneShot) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetMediaType(const AM_MEDIA_TYPE *pType) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetConnectedMediaType(AM_MEDIA_TYPE * pType) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetBufferSamples(BOOL BufferThem) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetCurrentBuffer(long *pBufferSize, long *pBuffer) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetCurrentSample(IMediaSample * *ppSample) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetCallback(ISampleGrabberCB * pCallback, long WhichMethodToCallback) = 0;
};
static const IID IID_ISampleGrabber = {0x6B652FFF, 0x11FE, 0x4fce, {0x92, 0xAD, 0x02, 0x66, 0xB5, 0xD7, 0xC7, 0x8F}};
static const CLSID CLSID_SampleGrabber = {0xC1F400A0, 0x3F08, 0x11d3, {0x9F, 0x0B, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37}};
static const CLSID CLSID_NullRenderer = {0xC1F400A4, 0x3F08, 0x11d3, {0x9F, 0x0B, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37}};
static const CLSID CLSID_VideoEffects1Category = {0xcc7bfb42, 0xf175, 0x11d1, {0xa3, 0x92, 0x0, 0xe0, 0x29, 0x1f, 0x39, 0x59}};
static const CLSID CLSID_VideoEffects2Category = {0xcc7bfb43, 0xf175, 0x11d1, {0xa3, 0x92, 0x0, 0xe0, 0x29, 0x1f, 0x39, 0x59}};
static const CLSID CLSID_AudioEffects1Category = {0xcc7bfb44, 0xf175, 0x11d1, {0xa3, 0x92, 0x0, 0xe0, 0x29, 0x1f, 0x39, 0x59}};
static const CLSID CLSID_AudioEffects2Category = {0xcc7bfb45, 0xf175, 0x11d1, {0xa3, 0x92, 0x0, 0xe0, 0x29, 0x1f, 0x39, 0x59}};
#endif

#ifndef MAXLONGLONG
#define MAXLONGLONG 0x7FFFFFFFFFFFFFFF
#endif

#ifndef MAX_DEVICES
#define MAX_DEVICES 8
#endif

#ifndef MAX_DEVICE_NAME
#define MAX_DEVICE_NAME 80
#endif

#ifndef BITS_PER_PIXEL
#define BITS_PER_PIXEL 24
#endif
#else

/**
 * @brief 摄像头对象
 *
 */
struct VideoDevice {
    char name[50];
    char path[256];
    char vid[10];
    char pid[10];
};
#endif // WIN32

namespace clangTools
{
#ifdef WIN32

typedef struct
{
	unsigned char R;
	unsigned char G;
	unsigned char B;
} RGB;

class VideoDevice;

typedef void (*VideoCaptureCallback)(unsigned char *data, int len, LONG WIDTH, LONG HEIGHT, int bitsperpixel, VideoDevice *dev);

class VideoDevice
{
  public:
	VideoDevice();
	~VideoDevice();
	bool isMirror;
	/**
	 * @brief Get the Id object 获取 ID
	 * 
	 * @return int 
	 */
	int GetId();
	/**
	 * @brief Get the Friendly Name object
	 * 
	 * @return const char* 
	 */
	const char *GetFriendlyName();
	/**
	 * @brief Set the Callback object
	 * 
	 * @param cb 
	 */
	void SetCallback(VideoCaptureCallback cb);
	/**
	 * @brief 开始预览
	 * 
	 */
	void Start();
	/**
	 * @brief 停止预览
	 * 
	 */
	void Stop();
	/**
	 * @brief 检查摄像头vid以及pid
	 * 
	 * @param vid vid
	 * @param pid pid
	 * @return true 相同
	 * @return false 不同
	 */
	bool Check(char vid[], char pid[]);
	/**
	 * @brief 获取亮度值
	 * 
	 * @param val 当前亮度值
	 * @param Min 最小值
	 * @param Max 最大值
	 * @return true 成功
	 * @return false 失败
	 */
	bool GetBrightness(long *val,long *Min,long *Max);/**
	* @brief 设置亮度
	* 
	* @param val 需要设置的值
	* @return true 设置成功
	* @return false 设置失败
	*/
	bool SetBrightness(long *val);
	/**
	 * @brief 获取对比度
	 * 
	 * @param val 当前对比度值
	 * @param Min 最小对比度的值
	 * @param Max 最大对比度值
	 * @return true 获取成功
	 * @return false 失败
	 */
	bool GetContrast(long *val,long *Min,long *Max);
	/**
	 * @brief 对比度
	 * 
	 * @param val 需要设置的对比度值
	 * @return true 设置成功
	 * @return false 设置失败
	 */
	bool SetContrast(long *val);
	/**
	 * @brief 获取饱和度值以及范围
	 * 
	 * @param val 当前饱和度的值
	 * @param Min 最小值
	 * @param Max 最大值
	 * @return true 获取成功
	 * @return false 失败
	 */
	bool GetSaturation(long *val,long *Min,long *Max);
	/**
	 * @brief 设置饱和度
	 * 
	 * @param val 需要设置的饱和度的值
	 * @return true 成功
	 * @return false 失败
	 */
	bool SetSaturation(long *val);

    char vid[20];
    char pid[20];
  private:
	int id;
	char *friendlyname;
	WCHAR *filtername;


	LONG Width;
	LONG Height;

	IBaseFilter *capture_filter;
	IBaseFilter *sourcefilter;
	IBaseFilter *samplegrabberfilter;
	IBaseFilter *nullrenderer;


	ISampleGrabber *samplegrabber;

	IFilterGraph2 *graph;

	class CallbackHandler : public ISampleGrabberCB
	{
	  public:
		CallbackHandler(VideoDevice *parent);
		~CallbackHandler();

		void SetCallback(VideoCaptureCallback cb);

		virtual HRESULT __stdcall SampleCB(double time, IMediaSample *sample);
		virtual HRESULT __stdcall BufferCB(double time, BYTE *buffer, long len);
		virtual HRESULT __stdcall QueryInterface(REFIID iid, LPVOID *ppv);
		virtual ULONG __stdcall AddRef();
		virtual ULONG __stdcall Release();

	  private:
		VideoCaptureCallback callback;
		VideoDevice *parent;

	} * callbackhandler;

	friend class v4l2Tool;
};

class v4l2Tool
{
  public:
	v4l2Tool();
	~v4l2Tool();

	void Init();

	VideoDevice *GetDevices();
	int NumDevices();

	/**
	 * @brief 设置分辨率
	 * 
	 * @param width 
	 * @param height 
	 */
	void SetWH(LONG width,LONG height);

  protected:
	void InitializeGraph();
	void InitializeVideo();

  private:
	IFilterGraph2 *graph = nullptr;
	ICaptureGraphBuilder2 *capture = nullptr;
	IMediaControl *control = nullptr;

	bool playing;

	VideoDevice *devices = nullptr;
	VideoDevice *current = nullptr;

	LONG wWidth = 800;  //修改采集视频的宽为320
	LONG wHeight = 600; //修改采集视频的高为240

	int num_devices = 0;
};
#else
#include <stddef.h>
#include <stdio.h>

#define CLEAR(x) memset(&(x), 0, sizeof(x))

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef int LONG;

/**/
#define NEAR
#define FAR

typedef struct
{
	unsigned char *data;
	size_t length;
} buffer_t;

/**
 * @brief v4l2 设备数据
 * 
 */
typedef struct
{
	int fd;			   // device file descriptor
	int fdout_devname; // virtual device file descriptor
	int fmt;
	unsigned int timeout;
	unsigned int width; //宽度
	unsigned int height; //高度
	unsigned int fps; //fps
	unsigned int n_buffers;
	char *in_devname;
	char *out_devname;
	buffer_t *buffer;
} v4l2device;
/**
 * @brief 摄像头工具类
 * 
 */
class v4l2Tool
{
  public:
	v4l2Tool();
	~v4l2Tool();

	int Open(const char *devname);

	/**
	* 
	*/
	bool isClose;
	void Close();
	/**
	 * @brief 获取摄像头数据
	 * 
	 * @return int 
	 */
	int v4l2_capabilities();
	/**
	 * @brief 设置摄像头输入
	 * 
	 * @return int 
	 */
	int v4l2_set_input();
	/**
	 * @brief 设置格式 分辨率之类
	 * 
	 * @return int 
	 */
	int v4l2_set_pixfmt();
	/**
	 * @brief 设置相机帧速率
	 * 
	 * @return int 
	 */
	int v4l2_set_fps();
	/**
	 * @brief 设置内存映射模式缓冲器
	 * 
	 * @return int 
	 */
	int v4l2_init_mmap();
	/**
	 * @brief 开始预览
	 * 
	 * @return int 
	 */
	int prepare_cap();
	/**
	 * @brief 停止预览
	 * 
	 * @return int 
	 */
	int stop_capturing();
	/**
	 * @brief 拍照，保存到路径
	 * 
	 * @param BmpPath 需要保存的路径，进支持bmp格式
	 * @return int < 0 保存失败
	 */
	int grabFrame(const char *BmpPath = "ocr.bmp");
	/**
	 * @brief 获取相机数据
	 * 
	 * @param colors 像素数据
	 * @param BitPerPixel 位数
	 * @return int 
	 */
	int grabFrame(unsigned char *colors,int BitPerPixel = 32);
	/* configure virtual video device */
	/**
	 * @brief 配置虚拟视频设备
	 * 
	 */
	int v4l2loopbackDevice();
	unsigned int running = 0;

	/**
	 * @brief 获取白平衡模式
	 * 
	 * @param fd 摄像头设备
	 * @return int 摄像头值
	 */
	int GetAutoWhiteBalance();
	/**
	 * @brief 设置摄像头白平衡
	 * 
	 * @param fd 摄像头设备
	 * @param enable 设置摄像头模式
	 * @return int 是否设置成功
	 */
	int SetAutoWhiteBalance(int enable);

	/**
	 * @brief 获取亮度
	 * 
	 * @param fd 摄像头设备
	 * @return int 亮度值
	 */
	int GetBrightness();
	/**
	 * @brief 设置摄像头白平衡
	 * 
	 * @param fd 摄像头设备
	 * @param enable 设置亮度值
	 *      struct v4l2_queryctrl queryctrl 
	 *      queryctrl.default_value;
	 *          
	 * @return int 是否设置成功
	 */
	int SetBrightness(int enable);

	/**
	 * @brief 获取对比度
	 * 
	 * @param fd 摄像头设备
	 * @return int 对比度
	 */
	int GetContrast();
	/**
	 * @brief 设置摄像头对比度
	 * 
	 * @param fd 摄像头设备
	 * @param enable 设置对比度
	 *          
	 * @return int 是否设置成功
	 */
	int SetContrast(int enable);

	int width = 640;
	int height = 480;
	int fps = 10;
	bool isMirror = false;
	v4l2device *dev = NULL;

	/**
	 * @brief 获取摄像头列表
	 * 
	 * @param vd 出參 需要事先分配
	 * @param vdLen 入参 vd的分配大小
	 * @return int 实际找到的摄像头数
	 */
	static int GetDevices(VideoDevice vd[], int vdLen);
	// static void SetVideoName(char *name);
	// static char VideoName[100];

	/**
	 * @brief 摄像头是否掉线 Flag
	 * 
	 */
	bool VIDIOC_DQBUF_Fail_Flag = false;
	/**
	 * @brief 锁
	 * 
	 */
	pthread_mutex_t testlock ;//= PTHREAD_MUTEX_INITIALIZER;
	/**
	 * @brief 摄像头线程
	 * 
	 */
	pthread_t *test_thread;
	unsigned char *frame_buffer = NULL;
	/**
	 * @brief 设置摄像头 函数
	 * 
	 * @param fd 
	 * @param request 
	 * @param arg 
	 * @return int 
	 */
	int xioctl(int fd, int request, void *arg);
	/**
	 * @brief 获取摄像头数据线程
	 * 
	 * @param arg 当前对象指针
	 * @return void* 
	 */
	static void *ReadVideo(void *arg);
	/**
	 * @brief  YUYV 数据转 RGB 数据格式
	 * 
	 * @param pointer YUYV 数据
	 * @param frame_buffer 转换后的RGB格式
	 */
	void yuyv_to_rgb(unsigned char *yuv, unsigned char *rgb);
		
	/**
	 * @brief YUYV 数据转 RGB 数据格式
	 * 
	 * @param pointer 
	 * @param frame_buffer 
	 * @param width_2 
	 * @param i 
	 * @param j 
	 */
	void yuyv_to_rgb_G(unsigned char *pointer, unsigned char *frame_buffer,
						int width_2,int i,int j);
};


#endif
} // namespace clangTools
#endif
#endif //__V4L2TOOL__H__
