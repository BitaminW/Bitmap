#include <stdio.h>
#include <iostream>
#include <crtdbg.h>
#include <Windows.h>
#include <fstream>
#if _DEBUG
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define malloc(s) _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
#endif
// 흑백 모니터가 아닌 이상 요즘엔 보통 bit count가 24
#define BIT_COUNT 24;
using namespace std;
// HBITMAP 핸들을 파일로 저장하는 함수
BOOL SaveHBITMAPToFile(HBITMAP hBitmap, LPCSTR lpszFileName)
{
  // output stream 선언
  ofstream stream;
  // stream open (바이너리 모드)
  stream.open(lpszFileName, ios::binary);
  // stream이 열리지 않으면 에러.
  if (!stream.is_open())
  {
    cout << "File open error!!" << endl;
    return FALSE;
  }
  // 저장할 bitmap 선언
  BITMAP bitmap;
  // hBitmap으로 bitmap을 가져온다.
  GetObject(hBitmap, sizeof(bitmap), (LPSTR)&bitmap);
  // Bitmap Header 정보 설정
  BITMAPINFOHEADER bi;
  bi.biSize = sizeof(BITMAPINFOHEADER);
  bi.biWidth = bitmap.bmWidth;
  bi.biHeight = bitmap.bmHeight;
  bi.biPlanes = 1;
  bi.biBitCount = BIT_COUNT;
  bi.biCompression = BI_RGB;
  bi.biSizeImage = 0;
  bi.biXPelsPerMeter = 0;
  bi.biYPelsPerMeter = 0;
  bi.biClrUsed = 0;
  bi.biClrImportant = 0;
  // 컬러 사이즈
  int PalSize = (bi.biBitCount == 24 ? 0 : 1 << bi.biBitCount) * sizeof(RGBQUAD);
  int Size = bi.biSize + PalSize + bi.biSizeImage;
 
  BITMAPFILEHEADER fh;
  fh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + PalSize;
  fh.bfReserved1 = 0;
  fh.bfReserved2 = 0;
  fh.bfSize = Size + sizeof(BITMAPFILEHEADER);
  fh.bfType = 0x4d42;
 
  // 파일에 bitmap 해더 작성
  stream.write((LPSTR)&fh, sizeof(BITMAPFILEHEADER));
  // DC 취득
  HDC hDC = GetDC(NULL);
  // 메모리 할당 (bitmap header)
  BITMAPINFO* header = (BITMAPINFO*)malloc(bi.biSize + PalSize);
  header->bmiHeader = bi;
  // hBitmap으로 부터 해더를 가져온다.
  GetDIBits(hDC, hBitmap, 0, bitmap.bmHeight, NULL, header, DIB_RGB_COLORS);
  // 이미지 전체 사이즈를 취득한다.
  bi = header->bmiHeader;
  if (bi.biSizeImage == 0)
  {
    // 해더 사이즈 설정이 안되면 강제 계산 설정
    bi.biSizeImage = ((bitmap.bmWidth * bi.biBitCount + 31) & ~31) / 8 * bitmap.bmHeight;
  }
  // 이미지 영역 메모리 할당
  Size = bi.biSize + PalSize + bi.biSizeImage;
  void* body = malloc(header->bmiHeader.biSizeImage);
  // hBitmap의 데이터를 저장
  GetDIBits(hDC, hBitmap, 0, header->bmiHeader.biHeight, body, header, DIB_RGB_COLORS);
  // 데이터 작성
  stream.write((LPSTR)&header->bmiHeader, sizeof(BITMAPINFOHEADER));
  stream.write((LPSTR)body, Size);
  // DC 해제
  ReleaseDC(NULL, hDC);
  // stream 해제
  stream.close();
  // 메모리 해제
  delete header;
  delete body;
}
// 스크린 샷
void screenshot(LPCSTR filepath)
{
  // 스크린 크기를 저장하기 위한 변수
  RECT desktop;
  // 데스크 탑의 핸들
  const HWND hDesktop = GetDesktopWindow();
  // 핸들로 부터 크기를 받는다.
  GetWindowRect(hDesktop, &desktop);
  // 너비
  int width = desktop.right;
  // 높이
  int height = desktop.bottom;
 
  // 스크린 핸들러 취득
  HDC hScreen = GetDC(NULL);
  // 핸들러 생성
  HDC hDC = CreateCompatibleDC(hScreen);
  // 핸들러에 bitmap 핸들러 생성
  HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, width, height);
  // 핸들러 -> bitmap 핸들러 선택
  HGDIOBJ old_obj = SelectObject(hDC, hBitmap);
  // hDC는 출력될 핸들러.
  // x, y, width, height -> 이건 위 hDC에 표시될 위치와 크기입니다.
  // 스크린 핸들러의 좌표 x, y위치부터
  // SRCCOPY - 원본 복사
  BOOL bRet = BitBlt(hDC, 0, 0, width, height, hScreen, 0, 0, SRCCOPY);
 
  // bitmap 데이터를 저장한다.
  SaveHBITMAPToFile(hBitmap, filepath);
 
  //핸들러 -> 리소스 돌리기
  SelectObject(hDC, old_obj);
  // 핸들러 삭제 (리소스 해제)
  DeleteDC(hDC);
  // 리소스 해제
  ReleaseDC(NULL, hScreen);
  // bitmap 리소스 해제
  DeleteObject(hBitmap);
}
// 실행 함수
int main()
{
  // 스크린 샷을 실행한다.
  screenshot("d:\\work\\test.bmp");
 
  _CrtDumpMemoryLeaks();
  return 0;
}
