// CaptureTestDlg.cpp : ʵ���ļ�
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
// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

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

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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


// CCaptureTestDlg �Ի���
//typedef int (__stdcall *FnInitScreenCaptureW)  (const char* szAuth);

typedef int (__stdcall *FnStartScreenCaptureW)(const wchar_t* szDefaultSavePath, void* pCallBack, UINT_PTR hWndNotice, UINT_PTR noticeMsg, UINT_PTR hwndHideWhenCapture, int autoCapture, int x, int y, int width, int height);
FnStartScreenCaptureW gl_StartScreenCapture = NULL;
typedef int (__stdcall *FnInitScreenCaptureW)(const wchar_t* szAuth);
FnInitScreenCaptureW gl_InitCapture = NULL;

typedef int (__stdcall *FnInitCaptureParamW)(int flag, UINT_PTR flagvalue);
FnInitCaptureParamW gl_InitCaptureParam = NULL;



typedef enum ExtendFlagTypeEnum
{
	emPensize = 1,		//���û��ʴ�С
	emDrawType,			//��������Ѷ�����360���
	emTrackColor,		//�Զ�ʶ��ı߿����ɫ
	emEditBorderColor,	//�ı�����ı߿���ɫ
	emTransparent,		//��������͸����
	emWindowAware,		//�����Ƿ��������DPI�Ŵ�
	emDetectSubWindowRect,	//�Ƿ��Զ�����Ӵ��ڣ���ʱ���� 
	emSetSaveName,		//���ñ���ʱ�Ŀ�ʼ����
	emSetMagnifierBkColor, //���÷Ŵ󾵵ı���ɫ����������͸��
	emSetMagnifierLogoText, //���÷Ŵ��ϵ�LOGO�ַ�������ʾ��ݼ����磺 ţţ��ͼ(CTRL + SHIFT + A)
	emSetNoticeCallback = 19,				//�������ÿؼ�֪ͨ��Ϣ�Ļص����� 
	emSetWatermarkPictureType,						//����ˮӡ������ 
	emSetWatermarkPicturePath,						//����ˮӡ��·�� 
	emSetWatermarkTextType,						//����ˮӡ���ֵ����� 
	emSetWatermarkTextValue,						//����ˮӡ����
	emSetMosaicType,							//�������������ͣ�1Ϊ���Σ�2Ϊ���� 
	emSetToolbarText,							//���ù������ϵĸ���ť��tooltip����ʾ������  

};

CCaptureTestDlg::CCaptureTestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCaptureTestDlg::IDD, pParent)
{
	//���ؽ�ͼ�ؼ�
	string strDllPath = GetCurrentPathA() + "NiuniuCapture.dll";
#ifdef __X64
	strDllPath = GetCurrentPathA() + "NiuniuCapturex64.dll";
#endif
	m_hModule = LoadLibrary(strDllPath.c_str());
	DWORD dRet = GetLastError();
	gl_StartScreenCapture = (FnStartScreenCaptureW)GetProcAddress(m_hModule, "StartScreenCaptureW");
	gl_InitCapture = (FnInitScreenCaptureW)GetProcAddress(m_hModule, "InitScreenCaptureW");
	gl_InitCaptureParam = (FnInitCaptureParamW)GetProcAddress(m_hModule, "InitCaptureParamW");

	//�˴���ֱ�Ӵ��� niuniu ��Ϊ��Ȩ����
	//�������Ȩ�汾�������������Ȩ������ΪKey, ���Ե�ǰ���ڽ�β���ַ�������md5�������ַ��� 
	string authCode = "niuniu";
	authCode = GenerateAuthCode(authCode);
	
	wstring wcsCode = s2ws(authCode);
	//�˺�����Ҫ�����������ú�������  
	gl_InitCapture(wcsCode.c_str());
	//gl_InitCapture(authCode.c_str());
	
	gl_InitCaptureParam(ExtendFlagTypeEnum::emPensize, 2);	//�����߿�
	gl_InitCaptureParam(ExtendFlagTypeEnum::emDrawType, 0);	//���÷Ŵ󾵷��0�� ��Ѷ���   1�� 360��� 
	gl_InitCaptureParam(ExtendFlagTypeEnum::emTrackColor, RGB(255, 0, 0));	//�Զ�ʶ��ı߿���ɫ
	gl_InitCaptureParam(ExtendFlagTypeEnum::emEditBorderColor, RGB(0, 174, 255));	//���ֱ༭��߿���ɫ
	gl_InitCaptureParam(ExtendFlagTypeEnum::emTransparent, 240); //���ù���������͸����


/***********************************************�ⲿ��Ϊ����ˮӡͼƬ��ˮӡ���֣�����Ҫ��ȥ�� **************************/	
	//nShowType|nMinWidth|nMinHeight|nMaxWidth|nMaxHeight|nShowOffset
	const wchar_t* szWatermarkFlag = L"3|100|100|400|400|20";
	gl_InitCaptureParam(ExtendFlagTypeEnum::emSetWatermarkPictureType, (UINT_PTR)szWatermarkFlag);
	
	//����ˮӡͼƬ·��  
	wstring strWaterPath = GetCurrentPath() + L"watermark.png";
	//gl_InitCaptureParam(ExtendFlagTypeEnum::emSetWatermarkPicturePath, (UINT_PTR)strWaterPath.c_str());

	
	//nShowType|nMinWidth|nMinHeight|nVerticalInterval|nOffset|nFontSize|nIsBold|nTextWidth|nTextHeight|colorText
	//colorText��ֵΪ�� A,R,G,B
	wstring strWaterTextFlag = L"3|60|60|150|20|20|0|200|50|80,55,55,55";
	gl_InitCaptureParam(ExtendFlagTypeEnum::emSetWatermarkTextType, (UINT_PTR)strWaterTextFlag.c_str());

	wstring strWaterText = L"����ˮӡ�������޸ġ� 2015-07-25 23:00:00";
//	gl_InitCaptureParam(ExtendFlagTypeEnum::emSetWatermarkTextValue, (UINT_PTR)strWaterText.c_str());
/***********************************************���ϲ���Ϊ����ˮӡͼƬ��ˮӡ���֣�����Ҫ��ȥ�� **************************/


	wstring strSavePath = L"ţţ��ͼ";
	gl_InitCaptureParam(ExtendFlagTypeEnum::emSetSaveName, (UINT_PTR)strSavePath.c_str()); //���ñ���ʱ�Ŀ�ʼ����
	gl_InitCaptureParam(ExtendFlagTypeEnum::emSetMagnifierBkColor, RGB(255, 255, 255)); //���÷Ŵ󾵵ı���ɫ����������͸��
	
	//���¿������÷Ŵ��ϵ�LOGO���֣���������ã�Ĭ����ʾ��ţţ��ͼ�� 
	//gl_InitCaptureParam(ExtendFlagTypeEnum::emSetMagnifierLogoText, (UINT_PTR)"ţţ��ͼ(Ctrl+Shift+A)");
	wstring strMagnifierLogoText = L"  ��ͨ���ӿ���������";
	gl_InitCaptureParam(ExtendFlagTypeEnum::emSetMagnifierLogoText, (UINT_PTR)strMagnifierLogoText.c_str());
	
	//�������ù������ϵĸ�����ť��tooltip�����Լ���ɰ�ť��ʾ��������Ϣ 
	//tipRectangle|tipCircle|tipArrow|tipBrush|tipGlitter|tipMosaic|tipText|tipUndo|tipSave|tipCancel|tipFinish|txtFinish
	wstring strToolbarText = L"Rectangle|Circle|Arrow|Brush|Glitter|Mosaic|Text|Undo|Save|Cancel|Finish|Finish";
	gl_InitCaptureParam(ExtendFlagTypeEnum::emSetToolbarText, (UINT_PTR)strToolbarText.c_str());
	gl_InitCaptureParam(ExtendFlagTypeEnum::emWindowAware, 1); //�˺������贰�ڴ���ǰ���ã����ͬ�����´���

	//ͨ����δ�����ú�Ӧ�ó��򽫲���DPI���зŴ���������DPI�Ŵ�Ļ����ϣ���Ҫ���ô�API�� һ��Ҫ�ڴ��ڴ���ǰ���е��ã��������Ӧ�ó����ʼ��ʼ���ĵط� 
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



// CCaptureTestDlg ��Ϣ�������

BOOL CCaptureTestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	//CenterWindow();
	ShowWindow(SW_HIDE);

	CButton* btn = (CButton*)GetDlgItem(IDC_RADIO1);
	btn->SetCheck(TRUE);
	
	CRect   rcTemp;
	rcTemp.BottomRight() = CPoint(5, 5);
	rcTemp.TopLeft() = CPoint(0, 0);
	MoveWindow(&rcTemp);

	SetWindowText("������Ͻ�ͼ����");

	OnBnClickedOk();

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CCaptureTestDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ��������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù����ʾ��
//
HCURSOR CCaptureTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

HWND gl_hWnd = NULL;
//szInfoΪUTF8������ַ��������ڱ����ͼʱ���ر����·��   
void CaptureNotice(int nType, int x, int y, int width, int height, const char* szInfo)
{
	if (nType == 1)	//��ʾ��ͼ��� 
	{
	}
	else if(nType == 2)	//��ʾȡ����ͼ
	{

	}
	else		//�����ͼ 
	{

	}
	::PostMessage(gl_hWnd, WM_USER + 1111, 1, nType);
}

void CCaptureTestDlg::OnBnClickedOk()
{
	SetDlgItemText(IDC_STATIC_NOTICE, "");
// 
// 	string strPath = _TEXT("c:\\����һ��\\test.jpg");
// 	locale loc = locale::global(locale(""));
// 	ifstream  fin(strPath.c_str(), ios::binary);  
// 	locale::global(loc);
// 	fin.exceptions ( ifstream::eofbit | ifstream::failbit | ifstream::badbit );  
// 
// 	istream::pos_type current_pos = fin.tellg();//��¼�µ�ǰλ��   
// 	fin.seekg(0,ios_base::end);//�ƶ����ļ�β  
// 	istream::pos_type file_size = fin.tellg();//ȡ�õ�ǰλ�õ�ָ�볤��->���ļ�����   
// 	fin.seekg(current_pos);//�ƶ���ԭ����λ��  
// 	return;
	/*
	����1�Ķ������� 
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
		wstring strText = L"ר��ˮӡ���֣�����ؾ�!";
		gl_InitCaptureParam(ExtendFlagTypeEnum::emSetWatermarkTextValue, (UINT_PTR)strText.c_str());
	}
	else
	{
		gl_InitCaptureParam(ExtendFlagTypeEnum::emSetWatermarkTextValue, (UINT_PTR)"");
	}
	gl_InitCaptureParam(2, bCheck);	//����2�����壬0�� ��Ѷ���   1�� 360��� 
	//gl_InitCaptureParam(24, bCheck ? 1 : 2);
	//ͨ���ص�������֪ͨ��ͼ����¼�
	gl_hWnd = m_hWnd;
	const wchar_t* szSavePath = L"c:\\����һ��\\test.jpg";
	gl_StartScreenCapture(szSavePath, CaptureNotice, 0, 0, 
		0,
		0, 0, 0, 0, 0);

	//ͨ�� WINDOWS ��Ϣ��֪ͨ��ͼ����¼�

	//gl_StartScreenCapture("niuniu", "", NULL, (UINT_PTR)m_hWnd, WM_USER + 1111);
}

BOOL SaveBitmapToFile(HBITMAP   hBitmap, CString szfilename)
{
	HDC     hDC;
	//��ǰ�ֱ�����ÿ������ռ�ֽ���            
	int     iBits;
	//λͼ��ÿ������ռ�ֽ���            
	WORD     wBitCount;
	//�����ɫ���С��     λͼ�������ֽڴ�С     ��λͼ�ļ���С     ��     д���ļ��ֽ���                
	DWORD     dwPaletteSize = 0, dwBmBitsSize = 0, dwDIBSize = 0, dwWritten = 0;
	//λͼ���Խṹ                
	BITMAP     Bitmap;
	//λͼ�ļ�ͷ�ṹ            
	BITMAPFILEHEADER     bmfHdr;
	//λͼ��Ϣͷ�ṹ                
	BITMAPINFOHEADER     bi;
	//ָ��λͼ��Ϣͷ�ṹ                    
	LPBITMAPINFOHEADER     lpbi;
	//�����ļ��������ڴ�������ɫ����                
	HANDLE     fh, hDib, hPal, hOldPal = NULL;

	//����λͼ�ļ�ÿ��������ռ�ֽ���                
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

	//Ϊλͼ���ݷ����ڴ�                
	hDib = GlobalAlloc(GHND, dwBmBitsSize + dwPaletteSize + sizeof(BITMAPINFOHEADER));
	lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);
	*lpbi = bi;

	//     �����ɫ��                    
	hPal = GetStockObject(DEFAULT_PALETTE);
	if (hPal)
	{
		hDC = ::GetDC(NULL);
		hOldPal = ::SelectPalette(hDC, (HPALETTE)hPal, FALSE);
		RealizePalette(hDC);
	}

	//     ��ȡ�õ�ɫ�����µ�����ֵ                
	GetDIBits(hDC, hBitmap, 0, (UINT)Bitmap.bmHeight,
		(LPSTR)lpbi + sizeof(BITMAPINFOHEADER) + dwPaletteSize,
		(BITMAPINFO *)lpbi, DIB_RGB_COLORS);

	//�ָ���ɫ��                    
	if (hOldPal)
	{
		::SelectPalette(hDC, (HPALETTE)hOldPal, TRUE);
		RealizePalette(hDC);
		::ReleaseDC(NULL, hDC);
	}

	//����λͼ�ļ�                    
	fh = CreateFile(szfilename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

	if (fh == INVALID_HANDLE_VALUE)         return     FALSE;

	//     ����λͼ�ļ�ͷ                
	bmfHdr.bfType = 0x4D42;     //     "BM"                
	dwDIBSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwPaletteSize + dwBmBitsSize;
	bmfHdr.bfSize = dwDIBSize;
	bmfHdr.bfReserved1 = 0;
	bmfHdr.bfReserved2 = 0;
	bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER) + dwPaletteSize;
	//     д��λͼ�ļ�ͷ                
	WriteFile(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);
	//     д��λͼ�ļ���������                
	WriteFile(fh, (LPSTR)lpbi, dwDIBSize, &dwWritten, NULL);
	//���                    
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
		if (IsClipboardFormatAvailable(CF_BITMAP))//�жϸ�ʽ�Ƿ�����������Ҫ  
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
			SetDlgItemText(IDC_STATIC_NOTICE, "��ͼ���");
			saveBitMap();
			PostMessage(WM_CLOSE, 0, 0);
			break;
		case 2:
			SetDlgItemText(IDC_STATIC_NOTICE, "��ȡ���˽�ͼ");
			PostMessage(WM_CLOSE, 0, 0);
			break;
		case 3:
			SetDlgItemText(IDC_STATIC_NOTICE, "�������˽�ͼ");
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
