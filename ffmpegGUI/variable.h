#include "header.h"

HINSTANCE g_hInst;
LPCTSTR lpszClass = TEXT("플레이어");
HWND hWndMain;
HWND hPanel;
HWND hStage;
HWND hListWnd;
HWND hBtnOpen, hBtnPause, hStTime, hVolume;
HWND hBtnExit, hBtnFull, hBtnMax, hBtnMin;


AVFormatContext* fmtCtx;
int vidx, aidx;
AVStream* vStream, * aStream;
AVCodecParameters* vPara, * aPara;
AVCodec* vCodec, * aCodec;
AVCodecContext* vCtx, * aCtx;

SwsContext* swsCtx;
AVFrame RGBFrame;
uint8_t* rgbbuf;

bool isOpen;
DWORD ThreadID;
HANDLE hPlayThread;
enum ePlayStatus { P_STOP, P_RUN, P_EXIT, P_EOF };
ePlayStatus status = ePlayStatus::P_STOP;
LARGE_INTEGER frequency;


//파일의 가변적인 배열을 다루기 위한 클래스
class MediaArray {
public:
	sMedia* ar;
	int size;
	int num;
	TCHAR name[64];
	int nowsel;

	MediaArray() { Init(); }
	~MediaArray() { Uninit(); }
	void Init() {
		size = 20;
		num = 0;
		ar = (sMedia*)malloc(size * sizeof(sMedia));
		lstrcpy(name, TEXT(""));
		nowsel = -1;
	}
	void Uninit() {
		if (ar) {
			free(ar);
			ar = NULL;
		}
	}
	void Insert(int idx, sMedia value) {
		int need;

		need = num + 1;
		if (need > size) {
			size = need + 10;
			ar = (sMedia*)realloc(ar, size * sizeof(sMedia));
		}
		memmove(ar + idx + 1, ar + idx, (num - idx) * sizeof(sMedia));
		ar[idx] = value;
		num++;
	}
	void Delete(int idx) {
		memmove(ar + idx, ar + idx + 1, (num - idx - 1) * sizeof(sMedia));
		num--;
	}
	void Append(sMedia value) { Insert(num, value); }
	void Clear() { Uninit(); Init(); }
	sMedia& operator [](int idx) { return ar[idx]; }
};

//경로,재생시간,파일 크기
const int maxlist = 10;
int listnum = 1;
int nowlist = 0;
MediaArray arMedia[maxlist];

//리스트 핸들 선언
HWND hList, hListTab;
HWND hBtnListMenu, hBtnListAdd, hBtnListRemove, hBtnListDel;
enum { ID_BTNLISTMENU = 101, ID_BTNLISTADD, ID_BTNLISTREMOVE, ID_BTNLISTDEL };