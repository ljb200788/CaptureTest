// CaptureTestDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "CaptureTest.h"
#include "CaptureTestDlg.h"
#include "MD5.h"
#include <string>
#include <sstream>
#include <iostream> 
#include <fstream>

#include <gdiplus.h>
#include <stdio.h>
using namespace Gdiplus;
using namespace std;
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma warning(disable:4482)
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

wstring GetCurrentPath()
{
	WCHAR wcsPath[MAX_PATH] = {0};
	GetModuleFileNameW(NULL, wcsPath, MAX_PATH);
	wstring strPath = wcsPath;
	int nPos = strPath.rfind(L'\\');
	strPath = strPath.substr(0, nPos + 1);
	return strPath;
}

string GetCurrentPathA()
{
	char szPath[MAX_PATH] = {0};
	GetModuleFileName(NULL, szPath, MAX_PATH);
	string strPath = szPath;
	int nPos = strPath.rfind(L'\\');
	strPath = strPath.substr(0, nPos + 1);
	return strPath;
}


std::wstring Ansi2WChar(LPCSTR pszSrc, int nLen)
{
	int nSize = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pszSrc, nLen, 0, 0);
	if(nSize <= 0) return L"";

	WCHAR *pwszDst = new WCHAR[nSize+1];
	if( NULL == pwszDst) return L"";

	MultiByteToWideChar(CP_ACP, 0,(LPCSTR)pszSrc, nLen, pwszDst, nSize);
	pwszDst[nSize] = 0;

	if( pwszDst[0] == 0xFEFF)                    // skip Oxfeff
		for(int i = 0; i < nSize; i ++) 
			pwszDst[i] = pwszDst[i+1]; 

	wstring wcharString(pwszDst);
	delete[] pwszDst;

	return wcharString;
}


std::wstring s2ws(const string& s)
{
	return Ansi2WChar(s.c_str(),s.size());
}

string GenerateAuthCode(string strAuthCode)
{
	SYSTEMTIME m_time;
	GetLocalTime(&m_time);
	int year=m_time.wYear;
	int month=m_time.wMonth;
	int day=m_time.wDay;
	ostringstream oss;
	oss << year << month << day;

	string strpwd = strAuthCode;
	strpwd += oss.str();
	return MD5Encode((char*)strpwd.c_str());
}
class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CCaptureTestDlg 对话框
//typedef int (__stdcall *FnInitScreenCaptureW)  (const char* szAuth);

typedef int (__stdcall *FnStartScreenCaptureW)(const wchar_t* szDefaultSavePath, void* pCallBack, UINT_PTR hWndNotice, UINT_PTR noticeMsg, UINT_PTR hwndHideWhenCapture, int autoCapture, int x, int y, int width, int height);
FnStartScreenCaptureW gl_StartScreenCapture = NULL;
typedef int (__stdcall *FnInitScreenCaptureW)(const wchar_t* szAuth);
FnInitScreenCaptureW gl_InitCapture = NULL;

typedef int (__stdcall *FnInitCaptureParamW)(int flag, UINT_PTR flagvalue);
FnInitCaptureParamW gl_InitCaptureParam = NULL;



typedef enum ExtendFlagTypeEnum
{
	emPensize = 1,		//设置画笔大小
	emDrawType,			//设置是腾讯风格还是360风格
	emTrackColor,		//自动识别的边框的颜色
	emEditBorderColor,	//文本输入的边框颜色
	emTransparent,		//工具栏的透明度
	emWindowAware,		//设置是否禁用随着DPI放大
	emDetectSubWindowRect,	//是否自动检测子窗口，暂时无用 
	emSetSaveName,		//设置保存时的开始文字
	emSetMagnifierBkColor, //设置放大镜的背景色，不设置则透明
	emSetMagnifierLogoText, //设置放大镜上的LOGO字符，可提示快捷键，如： 牛牛截图(CTRL + SHIFT + A)
	emSetNoticeCallback = 19,				//用于设置控件通知信息的回调函数 
	emSetWatermarkPictureType,						//设置水印的类型 
	emSetWatermarkPicturePath,						//设置水印的路径 
	emSetWatermarkTextType,						//设置水印文字的类型 
	emSetWatermarkTextValue,						//设置水印文字
	emSetMosaicType,							//设置马赛克类型，1为矩形，2为画线 
	emSetToolbarText,							//设置工具栏上的各按钮的tooltip及显示的文字  

};

CCaptureTestDlg::CCaptureTestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCaptureTestDlg::IDD, pParent)
{
	//加载截图控件
	string strDllPath = GetCurrentPathA() + "NiuniuCapture.dll";
#ifdef __X64
	strDllPath = GetCurrentPathA() + "NiuniuCapturex64.dll";
#endif
	m_hModule = LoadLibrary(strDllPath.c_str());
	DWORD dRet = GetLastError();
	gl_StartScreenCapture = (FnStartScreenCaptureW)GetProcAddress(m_hModule, "StartScreenCaptureW");
	gl_InitCapture = (FnInitScreenCaptureW)GetProcAddress(m_hModule, "InitScreenCaptureW");
	gl_InitCaptureParam = (FnInitCaptureParamW)GetProcAddress(m_hModule, "InitCaptureParamW");

	//此处可直接传递 niuniu 作为授权密码
	//如果是授权版本，请输入你的授权密码作为Key, 并以当前日期结尾的字符串经过md5编码后的字符串 
	string authCode = "niuniu";
	authCode = GenerateAuthCode(authCode);
	
	wstring wcsCode = s2ws(authCode);
	//此函数需要先于其他设置函数调用  
	gl_InitCapture(wcsCode.c_str());
	//gl_InitCapture(authCode.c_str());
	
	gl_InitCaptureParam(ExtendFlagTypeEnum::emPensize, 2);	//画笔线宽
	gl_InitCaptureParam(ExtendFlagTypeEnum::emDrawType, 0);	//设置放大镜风格：0： 腾讯风格   1： 360风格 
	gl_InitCaptureParam(ExtendFlagTypeEnum::emTrackColor, RGB(255, 0, 0));	//自动识别的边框颜色
	gl_InitCaptureParam(ExtendFlagTypeEnum::emEditBorderColor, RGB(0, 174, 255));	//文字编辑框边框颜色
	gl_InitCaptureParam(ExtendFlagTypeEnum::emTransparent, 240); //设置工具栏窗口透明度


/***********************************************这部分为设置水印图片和水印文字，不需要则去掉 **************************/	
	//nShowType|nMinWidth|nMinHeight|nMaxWidth|nMaxHeight|nShowOffset
	const wchar_t* szWatermarkFlag = L"3|100|100|400|400|20";
	gl_InitCaptureParam(ExtendFlagTypeEnum::emSetWatermarkPictureType, (UINT_PTR)szWatermarkFlag);
	
	//设置水印图片路径  
	wstring strWaterPath = GetCurrentPath() + L"watermark.png";
	//gl_InitCaptureParam(ExtendFlagTypeEnum::emSetWatermarkPicturePath, (UINT_PTR)strWaterPath.c_str());

	
	//nShowType|nMinWidth|nMinHeight|nVerticalInterval|nOffset|nFontSize|nIsBold|nTextWidth|nTextHeight|colorText
	//colorText的值为： A,R,G,B
	wstring strWaterTextFlag = L"3|60|60|150|20|20|0|200|50|80,55,55,55";
	gl_InitCaptureParam(ExtendFlagTypeEnum::emSetWatermarkTextType, (UINT_PTR)strWaterTextFlag.c_str());

	wstring strWaterText = L"测试水印，请勿修改。 2015-07-25 23:00:00";
//	gl_InitCaptureParam(ExtendFlagTypeEnum::emSetWatermarkTextValue, (UINT_PTR)strWaterText.c_str());
/***********************************************以上部分为设置水印图片和水印文字，不需要则去掉 **************************/


	wstring strSavePath = L"牛牛截图";
	gl_InitCaptureParam(ExtendFlagTypeEnum::emSetSaveName, (UINT_PTR)strSavePath.c_str()); //设置保存时的开始文字
	gl_InitCaptureParam(ExtendFlagTypeEnum::emSetMagnifierBkColor, RGB(255, 255, 255)); //设置放大镜的背景色，不设置则透明
	
	//以下可以设置放大镜上的LOGO文字，如果不设置，默认显示“牛牛截图” 
	//gl_InitCaptureParam(ExtendFlagTypeEnum::emSetMagnifierLogoText, (UINT_PTR)"牛牛截图(Ctrl+Shift+A)");
	wstring strMagnifierLogoText = L"  可通过接口设置名称";
	gl_InitCaptureParam(ExtendFlagTypeEnum::emSetMagnifierLogoText, (UINT_PTR)strMagnifierLogoText.c_str());
	
	//以下设置工具栏上的各个按钮的tooltip文字以及完成按钮显示的文字信息 
	//tipRectangle|tipCircle|tipArrow|tipBrush|tipGlitter|tipMosaic|tipText|tipUndo|tipSave|tipCancel|tipFinish|txtFinish
	wstring strToolbarText = L"Rectangle|Circle|Arrow|Brush|Glitter|Mosaic|Text|Undo|Save|Cancel|Finish|Finish";
	gl_InitCaptureParam(ExtendFlagTypeEnum::emSetToolbarText, (UINT_PTR)strToolbarText.c_str());
	gl_InitCaptureParam(ExtendFlagTypeEnum::emWindowAware, 1); //此函数必需窗口创建前调用，其等同于如下代码

	//通过这段代码调用后，应用程序将不随DPI进行放大，在设置了DPI放大的机器上，需要调用此API； 一定要在窗口创建前进行调用，建议放在应用程序最开始初始化的地方 
	/*
	HINSTANCE hUser32 = LoadLibrary( "user32.dll" );
	if( hUser32 )
	{
		typedef BOOL ( WINAPI* LPSetProcessDPIAware )( void );
		LPSetProcessDPIAware pSetProcessDPIAware = ( LPSetProcessDPIAware )GetProcAddress(hUser32,
			"SetProcessDPIAware" );
		if( pSetProcessDPIAware )
		{
			pSetProcessDPIAware();
		}
		FreeLibrary( hUser32 );
	}
	*/

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);	
}

void CCaptureTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CCaptureTestDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_USER + 1111,OnCaptureFinish)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, &CCaptureTestDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_RADIO1, &CCaptureTestDlg::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO2, &CCaptureTestDlg::OnBnClickedRadio2)
END_MESSAGE_MAP()



// CCaptureTestDlg 消息处理程序

BOOL CCaptureTestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	//CenterWindow();
	ShowWindow(SW_HIDE);

	CButton* btn = (CButton*)GetDlgItem(IDC_RADIO1);
	btn->SetCheck(TRUE);
	
	CRect   rcTemp;
	rcTemp.BottomRight() = CPoint(5, 5);
	rcTemp.TopLeft() = CPoint(0, 0);
	MoveWindow(&rcTemp);

	SetWindowText("辅助诊断截图工具");

	OnBnClickedOk();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CCaptureTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CCaptureTestDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标显示。
//
HCURSOR CCaptureTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

HWND gl_hWnd = NULL;
//szInfo为UTF8编码的字符串，仅在保存截图时返回保存的路径   
void CaptureNotice(int nType, int x, int y, int width, int height, const char* szInfo)
{
	if (nType == 1)	//表示截图完成 
	{
	}
	else if(nType == 2)	//表示取消截图
	{

	}
	else		//保存截图 
	{

	}
	::PostMessage(gl_hWnd, WM_USER + 1111, 1, nType);
}

void CCaptureTestDlg::OnBnClickedOk()
{
	SetDlgItemText(IDC_STATIC_NOTICE, "");
// 
// 	string strPath = _TEXT("c:\\测试一下\\test.jpg");
// 	locale loc = locale::global(locale(""));
// 	ifstream  fin(strPath.c_str(), ios::binary);  
// 	locale::global(loc);
// 	fin.exceptions ( ifstream::eofbit | ifstream::failbit | ifstream::badbit );  
// 
// 	istream::pos_type current_pos = fin.tellg();//记录下当前位置   
// 	fin.seekg(0,ios_base::end);//移动到文件尾  
// 	istream::pos_type file_size = fin.tellg();//取得当前位置的指针长度->即文件长度   
// 	fin.seekg(current_pos);//移动到原来的位置  
// 	return;
	/*
	参数1的定义如下 
	typedef enum ExtendFlagTypeEnum
	{
	emPensize = 1,
	emDrawType,	
	};
	*/
	CButton* btn = (CButton*)GetDlgItem(IDC_RADIO2);
	BOOL bCheck = btn->GetCheck();
	
	btn = (CButton*)GetDlgItem(IDC_CHECK_WATERTEXT);
	BOOL bShowWaterText = btn->GetCheck();
	
	if (bShowWaterText)
	{
		wstring strText = L"专属水印文字，盗版必究!";
		gl_InitCaptureParam(ExtendFlagTypeEnum::emSetWatermarkTextValue, (UINT_PTR)strText.c_str());
	}
	else
	{
		gl_InitCaptureParam(ExtendFlagTypeEnum::emSetWatermarkTextValue, (UINT_PTR)"");
	}
	gl_InitCaptureParam(2, bCheck);	//参数2的意义，0： 腾讯风格   1： 360风格 
	//gl_InitCaptureParam(24, bCheck ? 1 : 2);
	//通过回调函数来通知截图完成事件
	gl_hWnd = m_hWnd;
	const wchar_t* szSavePath = L"c:\\测试一下\\test.jpg";
	gl_StartScreenCapture(szSavePath, CaptureNotice, 0, 0, 
		0,
		0, 0, 0, 0, 0);

	//通过 WINDOWS 消息来通知截图完成事件

	//gl_StartScreenCapture("niuniu", "", NULL, (UINT_PTR)m_hWnd, WM_USER + 1111);
}

BOOL SaveBitmapToFile(HBITMAP   hBitmap, CString szfilename)
{
	HDC     hDC;
	//当前分辨率下每象素所占字节数            
	int     iBits;
	//位图中每象素所占字节数            
	WORD     wBitCount;
	//定义调色板大小，     位图中像素字节大小     ，位图文件大小     ，     写入文件字节数                
	DWORD     dwPaletteSize = 0, dwBmBitsSize = 0, dwDIBSize = 0, dwWritten = 0;
	//位图属性结构                
	BITMAP     Bitmap;
	//位图文件头结构            
	BITMAPFILEHEADER     bmfHdr;
	//位图信息头结构                
	BITMAPINFOHEADER     bi;
	//指向位图信息头结构                    
	LPBITMAPINFOHEADER     lpbi;
	//定义文件，分配内存句柄，调色板句柄                
	HANDLE     fh, hDib, hPal, hOldPal = NULL;

	//计算位图文件每个像素所占字节数                
	hDC = CreateDC("DISPLAY", NULL, NULL, NULL);
	iBits = GetDeviceCaps(hDC, BITSPIXEL)     *     GetDeviceCaps(hDC, PLANES);
	DeleteDC(hDC);
	if (iBits <= 1)
		wBitCount = 1;
	else  if (iBits <= 4)
		wBitCount = 4;
	else if (iBits <= 8)
		wBitCount = 8;
	else
		wBitCount = 24;

	GetObject(hBitmap, sizeof(Bitmap), (LPSTR)&Bitmap);
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = Bitmap.bmWidth;
	bi.biHeight = Bitmap.bmHeight;
	bi.biPlanes = 1;
	bi.biBitCount = wBitCount;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrImportant = 0;
	bi.biClrUsed = 0;

	dwBmBitsSize = ((Bitmap.bmWidth *wBitCount + 31) / 32) * 4 * Bitmap.bmHeight;

	//为位图内容分配内存                
	hDib = GlobalAlloc(GHND, dwBmBitsSize + dwPaletteSize + sizeof(BITMAPINFOHEADER));
	lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);
	*lpbi = bi;

	//     处理调色板                    
	hPal = GetStockObject(DEFAULT_PALETTE);
	if (hPal)
	{
		hDC = ::GetDC(NULL);
		hOldPal = ::SelectPalette(hDC, (HPALETTE)hPal, FALSE);
		RealizePalette(hDC);
	}

	//     获取该调色板下新的像素值                
	GetDIBits(hDC, hBitmap, 0, (UINT)Bitmap.bmHeight,
		(LPSTR)lpbi + sizeof(BITMAPINFOHEADER) + dwPaletteSize,
		(BITMAPINFO *)lpbi, DIB_RGB_COLORS);

	//恢复调色板                    
	if (hOldPal)
	{
		::SelectPalette(hDC, (HPALETTE)hOldPal, TRUE);
		RealizePalette(hDC);
		::ReleaseDC(NULL, hDC);
	}

	//创建位图文件                    
	fh = CreateFile(szfilename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

	if (fh == INVALID_HANDLE_VALUE)         return     FALSE;

	//     设置位图文件头                
	bmfHdr.bfType = 0x4D42;     //     "BM"                
	dwDIBSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwPaletteSize + dwBmBitsSize;
	bmfHdr.bfSize = dwDIBSize;
	bmfHdr.bfReserved1 = 0;
	bmfHdr.bfReserved2 = 0;
	bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER) + dwPaletteSize;
	//     写入位图文件头                
	WriteFile(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);
	//     写入位图文件其余内容                
	WriteFile(fh, (LPSTR)lpbi, dwDIBSize, &dwWritten, NULL);
	//清除                    
	GlobalUnlock(hDib);
	GlobalFree(hDib);
	CloseHandle(fh);

	return     TRUE;

}

int SaveHBITMAP(HBITMAP hB, char * lpsz_FileName) {

	BITMAP csBitmap;
	int nRetValue = GetObject(hB, sizeof(csBitmap), &csBitmap);
	unsigned long n_BPP, n_Width, n_Height;

	if (nRetValue) {
		n_Width = (long)csBitmap.bmWidth;
		n_Height = (long)csBitmap.bmHeight;
		n_BPP = (long)csBitmap.bmBitsPixel;
		long sz = csBitmap.bmWidth*csBitmap.bmHeight*(csBitmap.bmBitsPixel >> 3);
		csBitmap.bmBits = (void *) new BYTE[sz];
		GetBitmapBits((HBITMAP)hB, sz, csBitmap.bmBits);

		printf("Proceeding Image %dx%d, BPP=%d", n_Width, n_Height, n_BPP, csBitmap.bmBits);
	}
	else {
		printf("Invalid Object in Clipboard Buffer"); return 1;
	}

	DWORD *lp_Canvas = new DWORD[n_Width * n_Height];
	if (n_BPP == 32) {
		for (unsigned long y = 0; y < n_Height; y++) {
			for (unsigned long x = 0; x < n_Width; x++) {
				RGBQUAD * rgb = ((RGBQUAD *)((char*)(csBitmap.bmBits)
					+ csBitmap.bmWidthBytes*y + x * sizeof(DWORD)));
				lp_Canvas[(n_Height - 1 - y)*n_Width + x] = *((DWORD *)rgb);
			}
		}
	}
	else if (n_BPP == 24) {
		for (unsigned long y = 0; y < n_Height; y++) {
			for (unsigned long x = 0; x < n_Width; x++) {

				RGBTRIPLE rgbi = *((RGBTRIPLE *)((char*)(csBitmap.bmBits)
					+ csBitmap.bmWidthBytes*y + x * 3));

				RGBQUAD rgbq;
				rgbq.rgbRed = rgbi.rgbtRed;
				rgbq.rgbGreen = rgbi.rgbtGreen;
				rgbq.rgbBlue = rgbi.rgbtBlue;
				lp_Canvas[(n_Height - 1 - y)*n_Width + x] = *((DWORD *)(&rgbq));
			}
		}
	}
	else {
		// here I could handle other resultions also, but I think it is 
		// too obvoius to add them here.... 
	}

	unsigned long n_Bits = 32;
	FILE *pFile;
	fopen_s(&pFile, lpsz_FileName, "wb");
	if (pFile == NULL) { printf("File Cannot Be Written"); return 1; }

	// save bitmap file header
	BITMAPFILEHEADER fileHeader;
	fileHeader.bfType = 0x4d42;
	fileHeader.bfSize = 0;
	fileHeader.bfReserved1 = 0;
	fileHeader.bfReserved2 = 0;
	fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	fwrite((char*)&fileHeader, sizeof(fileHeader), 1, pFile);

	// save bitmap info header
	BITMAPINFOHEADER infoHeader;
	infoHeader.biSize = sizeof(infoHeader);
	infoHeader.biWidth = n_Width;
	infoHeader.biHeight = n_Height;
	infoHeader.biPlanes = 1;
	infoHeader.biBitCount = (unsigned short)(n_Bits & 0xffff);
	infoHeader.biCompression = BI_RGB;
	infoHeader.biSizeImage = 0;
	infoHeader.biXPelsPerMeter = 0;
	infoHeader.biYPelsPerMeter = 0;
	infoHeader.biClrUsed = 0;
	infoHeader.biClrImportant = 0;
	fwrite((char*)&infoHeader, sizeof(infoHeader), 1, pFile);
	fwrite((char*)lp_Canvas, 1, (n_Bits >> 3)*n_Width*n_Height, pFile);
	fclose(pFile);
	return 0;
}

BOOL GetEncoderClsid(WCHAR* pFormat, CLSID* pClsid)
{
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR			gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	UINT num = 0, size = 0;
	ImageCodecInfo* pImageCodecInfo = NULL;
	GetImageEncodersSize(&num, &size);
	if (size == 0)
	{
		return FALSE;
	}
	pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
	{
		return FALSE;
	}
	GetImageEncoders(num, size, pImageCodecInfo);
	BOOL bfound = FALSE;
	for (UINT i = 0; !bfound && i < num; i++)
	{
		if (_wcsicmp(pImageCodecInfo[i].MimeType, pFormat) == 0)
		{
			*pClsid = pImageCodecInfo[i].Clsid;
			bfound = TRUE;
		}
	}
	GdiplusShutdown(gdiplusToken);
	free(pImageCodecInfo);
	return bfound;
}

BOOL BMptoPNG(LPCWSTR StrBMp, LPCWSTR StrPNG)
{
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR			gdiplusToken;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);


	CLSID encoderClsid;
	Status stat;
	Image* image = NULL;
	image = Gdiplus::Bitmap::FromFile(StrBMp, TRUE);
	if (!GetEncoderClsid(L"image/png", &encoderClsid))
	{
		return FALSE;
	}
	stat = image->Save(StrPNG, &encoderClsid, NULL);
	if (stat != Gdiplus::Ok)
	{
		return FALSE;
	}
	//Gdiplus::GdiplusShutdown(gdiplusToken);
	delete image;
	return TRUE;
}
void saveBitMap()
{
	if (OpenClipboard(0))
	{
		if (IsClipboardFormatAvailable(CF_BITMAP))//判断格式是否是我们所需要  
		{
			HBITMAP hBitmap = (HBITMAP)GetClipboardData(CF_BITMAP);
			
			if (hBitmap)
			{
				GlobalLock(hBitmap);
				char outfile[] = "clipboard.bmp";
				//SaveHBITMAP(hBitmap, outfile);

				SaveBitmapToFile(hBitmap, outfile);
				GlobalUnlock(hBitmap);

				CloseClipboard();
				
				WCHAR wszOutName[256];
				memset(wszOutName, 0, sizeof(wszOutName));

				MultiByteToWideChar(CP_ACP, 0, outfile, strlen(outfile) + 1, wszOutName,
					sizeof(wszOutName) / sizeof(wszOutName[0]));

				char pngName[] = "clipboard.png";
				WCHAR wszClassName[256];
				memset(wszClassName, 0, sizeof(wszClassName));

				MultiByteToWideChar(CP_ACP, 0, pngName, strlen(pngName) + 1, wszClassName,
					sizeof(wszClassName) / sizeof(wszClassName[0]));

				BMptoPNG(wszOutName, wszClassName);
				
			}
			CloseClipboard();
		}
	}

}

LRESULT CCaptureTestDlg::OnCaptureFinish(WPARAM wParam,LPARAM lParam)
{
	if (wParam == 1)
	{
		switch(lParam)
		{
		case 1:
			SetDlgItemText(IDC_STATIC_NOTICE, "截图完毕");
			saveBitMap();
			PostMessage(WM_CLOSE, 0, 0);
			break;
		case 2:
			SetDlgItemText(IDC_STATIC_NOTICE, "您取消了截图");
			PostMessage(WM_CLOSE, 0, 0);
			break;
		case 3:
			SetDlgItemText(IDC_STATIC_NOTICE, "您保存了截图");
			saveBitMap();
			PostMessage(WM_CLOSE, 0, 0);
			break;
		}
	}	
	return 0;
}

void CCaptureTestDlg::OnBnClickedRadio1()
{
	CButton* btn = (CButton*)GetDlgItem(IDC_RADIO1);
	btn->SetCheck(TRUE);

	btn = (CButton*)GetDlgItem(IDC_RADIO2);
	btn->SetCheck(FALSE);
}

void CCaptureTestDlg::OnBnClickedRadio2()
{
	CButton* btn = (CButton*)GetDlgItem(IDC_RADIO1);
	btn->SetCheck(FALSE);

	btn = (CButton*)GetDlgItem(IDC_RADIO2);
	btn->SetCheck(TRUE);
}
