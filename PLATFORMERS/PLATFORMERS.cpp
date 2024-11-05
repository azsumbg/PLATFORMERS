#include "D2BMPLOADER.h"
#include "ErrH.h"
#include "FCheck.h"
#include "framework.h"
#include "gifresizer.h"
#include "platgame.h"
#include "PLATFORMERS.h"
#include <chrono>
#include <d2d1.h>
#include <dwrite.h>
#include <fstream>
#include <mmsystem.h>


#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "platgame.lib")
#pragma comment(lib, "errh.lib")
#pragma comment(lib, "fcheck.lib")
#pragma comment(lib, "gifresizer.lib")
#pragma comment(lib, "d2bmploader.lib")

constexpr int mNew{ 1001 };
constexpr int mLvl{ 1002 };
constexpr int mExit{ 1003 };
constexpr int mSave{ 1004 };
constexpr int mLoad{ 1005 };
constexpr int mHoF{ 1006 };

constexpr int record{ 2001 };
constexpr int first_record{ 2002 };
constexpr int no_record{ 2003 };

constexpr wchar_t bWinClassName[]{ L"MyPlatformGame" };

constexpr char tmp_file[]{ ".\\res\\data\\temp.dat" };
constexpr wchar_t Ltmp_file[]{ L".\\res\\data\\temp.dat" };
constexpr wchar_t save_file[]{ L".\\res\\data\\save.dat" };
constexpr wchar_t load_file[]{ L".\\res\\data\\load.dat" };
constexpr wchar_t help_file[]{ L".\\res\\data\\help.dat" };
constexpr wchar_t sound_file[]{ L".\\res\\snd\\main.wav" };
////////////////////////////////////////////////////////////

WNDCLASS bWinClass{};
HINSTANCE bIns = nullptr;
HWND bHwnd = nullptr;
HICON Icon = nullptr;
HCURSOR mainCursor = nullptr;
HCURSOR outCursor = nullptr;
HDC PaintDC = nullptr;
PAINTSTRUCT bPaint{};
HMENU bBar = nullptr;
HMENU bMain = nullptr;
HMENU bStore = nullptr;
MSG bMsg{};
BOOL bRet = 0;

POINT cur_pos{};

D2D1_RECT_F b1Rect{ 20.0f, 0, scr_width / 3 - 50.0f, 50.0f };
D2D1_RECT_F b2Rect{ scr_width / 3 + 20.0f, 0, scr_width * 2 / 3 - 50.0f, 50.0f };
D2D1_RECT_F b3Rect{ scr_width * 2 / 3 + 20.0f, 0, scr_width, 50.0f };

wchar_t current_player[16] = L"One Tarlyo";

bool pause = false;
bool in_client = true;
bool sound = true;
bool show_help = false;
bool name_set = false;
bool b1Hglt = false;
bool b2Hglt = false;
bool b3Hglt = false;

bool hero_attacking = false;

int score = 0;
int level = 1;

gamedll::RANDENGINE Randomizer{};
std::vector<gamedll::FIELD>vFields;
dirs ambient_dir = dirs::stop;
std::vector<gamedll::FIELD>vPlatforms;

gamedll::Creature Hero = nullptr;

///////////////////////////////////////

ID2D1Factory* iFactory = nullptr;
ID2D1HwndRenderTarget* Draw = nullptr;
ID2D1RadialGradientBrush* BckgBrush = nullptr;
ID2D1SolidColorBrush* TextBrush = nullptr;
ID2D1SolidColorBrush* HgltBrush = nullptr;
ID2D1SolidColorBrush* InactBrush = nullptr;

IDWriteFactory* iWriteFactory = nullptr;
IDWriteTextFormat* nrmFormat = nullptr;
IDWriteTextFormat* midFormat = nullptr;
IDWriteTextFormat* bigFormat = nullptr;

ID2D1Bitmap* bmpGround = nullptr;
ID2D1Bitmap* bmpPlatform = nullptr;
ID2D1Bitmap* bmpPortal = nullptr;
ID2D1Bitmap* bmpRIP = nullptr;
ID2D1Bitmap* bmpVineD = nullptr;
ID2D1Bitmap* bmpVineL = nullptr;
ID2D1Bitmap* bmpVineR = nullptr;

ID2D1Bitmap* bmpField1[11] = { nullptr };
ID2D1Bitmap* bmpField2[8] = { nullptr };
ID2D1Bitmap* bmpField3[35] = { nullptr };
ID2D1Bitmap* bmpIntro[35] = { nullptr };

ID2D1Bitmap* bmpHeroAttL[24] = { nullptr };
ID2D1Bitmap* bmpHeroAttR[24] = { nullptr };
ID2D1Bitmap* bmpHeroWalkL[24] = { nullptr };
ID2D1Bitmap* bmpHeroWalkR[24] = { nullptr };

ID2D1Bitmap* bmpEvil1L[24] = { nullptr };
ID2D1Bitmap* bmpEvil1R[24] = { nullptr };

ID2D1Bitmap* bmpEvil2L[24] = { nullptr };
ID2D1Bitmap* bmpEvil2R[24] = { nullptr };

ID2D1Bitmap* bmpEvil3L[24] = { nullptr };
ID2D1Bitmap* bmpEvil3R[24] = { nullptr };

ID2D1Bitmap* bmpEvil4L[24] = { nullptr };
ID2D1Bitmap* bmpEvil4R[24] = { nullptr };

ID2D1Bitmap* bmpEvil5L[24] = { nullptr };
ID2D1Bitmap* bmpEvil5R[24] = { nullptr };

ID2D1Bitmap* bmpEvil6L[24] = { nullptr };
ID2D1Bitmap* bmpEvil6R[24] = { nullptr };
////////////////////////////////////////




/////////////////////////////////////////
template<typename T> concept CanBeReleased = requires(T check_var)
{
    check_var.Release();
};
template<CanBeReleased T> bool ClearHeap(T** what)
{
    if (*what)
    {
        (*what)->Release();
        (*what) = nullptr;
        return true;
    }
    return false;
}
void LogError(LPCWSTR what)
{
    std::wofstream log(L".\\res\\data\\error.log", std::ios::app);
    log << what << L" Time stamp: " << std::chrono::system_clock::now() << std::endl;
    log.close();
}
void ReleaseResources()
{
    if (!ClearHeap(&iFactory))LogError(L"Error releasing iFactory !");
    if (!ClearHeap(&Draw))LogError(L"Error releasing Draw !");
    if (!ClearHeap(&BckgBrush))LogError(L"Error releasing bckgBrush !");
    if (!ClearHeap(&HgltBrush))LogError(L"Error releasing HgltBrush !");
    if (!ClearHeap(&InactBrush))LogError(L"Error releasing InactBrush !");
    if (!ClearHeap(&TextBrush))LogError(L"Error releasing TextBrush !");

    if (!ClearHeap(&iWriteFactory))LogError(L"Error releasing iWriteFactory !");
    if (!ClearHeap(&nrmFormat))LogError(L"Error releasing nrmFormat !");
    if (!ClearHeap(&midFormat))LogError(L"Error releasing midFormat !");
    if (!ClearHeap(&bigFormat))LogError(L"Error releasing bigFormat !");

    if (!ClearHeap(&bmpGround))LogError(L"Error releasing bmpGround !");
    if (!ClearHeap(&bmpPlatform))LogError(L"Error releasing bmpPlatform !");
    if (!ClearHeap(&bmpPortal))LogError(L"Error releasing bmpPortal !");
    if (!ClearHeap(&bmpRIP))LogError(L"Error releasing bmpRIP !");
    if (!ClearHeap(&bmpVineD))LogError(L"Error releasing bmpVineD !");
    if (!ClearHeap(&bmpVineL))LogError(L"Error releasing bmpVineL !");
    if (!ClearHeap(&bmpVineR))LogError(L"Error releasing bmpVineR !");

    for (int i = 0; i < 11; i++)if (!ClearHeap(&bmpField1[i]))LogError(L"Error releasing bmpField1 !");
    for (int i = 0; i < 8; i++)if (!ClearHeap(&bmpField2[i]))LogError(L"Error releasing bmpField2 !");
    for (int i = 0; i < 35; i++)if (!ClearHeap(&bmpField3[i]))LogError(L"Error releasing bmpField3 !");
    for (int i = 0; i < 35; i++)if (!ClearHeap(&bmpIntro[i]))LogError(L"Error releasing bmpIntro !");

    for (int i = 0; i < 24; i++)if (!ClearHeap(&bmpHeroAttL[i]))LogError(L"Error releasing bmpHeroAttL !");
    for (int i = 0; i < 24; i++)if (!ClearHeap(&bmpHeroAttR[i]))LogError(L"Error releasing bmpHeroAttR !");
    for (int i = 0; i < 24; i++)if (!ClearHeap(&bmpHeroWalkL[i]))LogError(L"Error releasing bmpHeroWalkL !");
    for (int i = 0; i < 24; i++)if (!ClearHeap(&bmpHeroWalkR[i]))LogError(L"Error releasing bmpHeroWalkR !");

    for (int i = 0; i < 24; i++)if (!ClearHeap(&bmpEvil1L[i]))LogError(L"Error releasing bmpEvil1L !");
    for (int i = 0; i < 24; i++)if (!ClearHeap(&bmpEvil1R[i]))LogError(L"Error releasing bmpEvil1R !");

    for (int i = 0; i < 24; i++)if (!ClearHeap(&bmpEvil2L[i]))LogError(L"Error releasing bmpEvil2L !");
    for (int i = 0; i < 24; i++)if (!ClearHeap(&bmpEvil2R[i]))LogError(L"Error releasing bmpEvil2R !");

    for (int i = 0; i < 24; i++)if (!ClearHeap(&bmpEvil3L[i]))LogError(L"Error releasing bmpEvil3L !");
    for (int i = 0; i < 24; i++)if (!ClearHeap(&bmpEvil3R[i]))LogError(L"Error releasing bmpEvil3R !");

    for (int i = 0; i < 24; i++)if (!ClearHeap(&bmpEvil4L[i]))LogError(L"Error releasing bmpEvil4L !");
    for (int i = 0; i < 24; i++)if (!ClearHeap(&bmpEvil4R[i]))LogError(L"Error releasing bmpEvil4R !");

    for (int i = 0; i < 24; i++)if (!ClearHeap(&bmpEvil5L[i]))LogError(L"Error releasing bmpEvil5L !");
    for (int i = 0; i < 24; i++)if (!ClearHeap(&bmpEvil5R[i]))LogError(L"Error releasing bmpEvil5R !");

    for (int i = 0; i < 24; i++)if (!ClearHeap(&bmpEvil6L[i]))LogError(L"Error releasing bmpEvil6L !");
    for (int i = 0; i < 24; i++)if (!ClearHeap(&bmpEvil6R[i]))LogError(L"Error releasing bmpEvil6R !");

}
void ErrExit(int what)
{
    MessageBeep(MB_ICONERROR);
    MessageBox(NULL, ErrHandle(what), L"Критична грешка !", MB_OK | MB_APPLMODAL | MB_ICONERROR);

    std::remove(tmp_file);
    ReleaseResources();
    exit(1);
}

void GameOver()
{
    PlaySound(NULL, NULL, NULL);

    bMsg.message = WM_QUIT;
    bMsg.wParam = 0;
}
void InitGame()
{
    score = 0;
    level = 1;
    wcscpy_s(current_player, L"One Tarlyo");
    name_set = false;

    vFields.clear();
    for (float i = -scr_width; i < scr_width * 2; i += scr_width)
        vFields.push_back(gamedll::FIELD(static_cast<fields>(Randomizer.generate(0, 2)), (float)(i)));
    vPlatforms.clear();
    vPlatforms.push_back(gamedll::FIELD(fields::ground_platform, 0));

    ClearHeap(&Hero);
    Hero = gamedll::CreatureFactory(creatures::hero, 100.0f, ground - 49.0f);
    Hero->dir = dirs::stop;

    
}

INT_PTR CALLBACK DlgProc(HWND hwnd, UINT ReceivedMsg, WPARAM wParam, LPARAM lParam)
{
    switch (ReceivedMsg)
    {
    case WM_INITDIALOG:
        SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)(Icon));
        return true;

    case WM_CLOSE:
        EndDialog(hwnd, IDCANCEL);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            break;

        case IDOK:
            if (GetDlgItemText(hwnd, IDC_NAME, current_player, 16) < 1)
            {
                wcscpy_s(current_player, L"One Tarlyo");
                if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
                MessageBox(bHwnd, L"Името си ли забрави ?", L"Забраватор !", MB_OK | MB_APPLMODAL | MB_ICONEXCLAMATION);
                EndDialog(hwnd, IDCANCEL);
                break;
            }
            EndDialog(hwnd, IDOK);
            break;
        }
        break;
    }

    return (INT_PTR)(FALSE);
}
LRESULT CALLBACK WinProc(HWND hwnd, UINT ReceivedMsg, WPARAM wParam, LPARAM lParam)
{
    switch (ReceivedMsg)
    {
    case WM_CREATE:
        bBar = CreateMenu();
        bMain = CreateMenu();
        bStore = CreateMenu();

        AppendMenuW(bBar, MF_POPUP, (UINT_PTR)(bMain), L"Основно меню");
        AppendMenuW(bBar, MF_POPUP, (UINT_PTR)(bStore), L"Меню за данни");

        AppendMenuW(bMain, MF_STRING, mNew, L"Нова игра");
        AppendMenuW(bMain, MF_STRING, mLvl, L"Следващо ниво");
        AppendMenuW(bMain, MF_SEPARATOR, NULL, NULL);
        AppendMenuW(bMain, MF_STRING, mExit, L"Изход");

        AppendMenuW(bStore, MF_STRING, mSave, L"Запази игра");
        AppendMenuW(bStore, MF_STRING, mLoad, L"Зареди игра");
        AppendMenuW(bStore, MF_SEPARATOR, NULL, NULL);
        AppendMenuW(bStore, MF_STRING, mHoF, L"Зала на славата");
        SetMenu(hwnd, bBar);
        InitGame();
        break;

    case WM_CLOSE:
        pause = true;
        if (sound)mciSendString(L"play .\\res\\exclamation.wav", NULL, NULL, NULL);
        if (MessageBox(hwnd, L"Ако излезеш, ще загубиш тази игра !\n\nНаистина ли излизаш ?",
            L"Изход !", MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)
        {
            pause = false;
            break;
        }
        GameOver();
        break;

    case WM_PAINT:
        PaintDC = BeginPaint(hwnd, &bPaint);
        FillRect(PaintDC, &bPaint.rcPaint, CreateSolidBrush(RGB(0, 0, 100)));
        EndPaint(hwnd, &bPaint);
        break;

    case WM_SETCURSOR:
        GetCursorPos(&cur_pos);
        ScreenToClient(hwnd, &cur_pos);
        if (LOWORD(lParam) == HTCLIENT)
        {
            if (!in_client)
            {
                in_client = true;
                pause = false;
            }
            if (cur_pos.y <= 50)
            {
                if (cur_pos.x >= b1Rect.left && cur_pos.x <= b1Rect.right)
                {
                    if (!b1Hglt)
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                        b1Hglt = true;
                        b2Hglt = false;
                        b3Hglt = false;
                    }
                }
                if (cur_pos.x >= b2Rect.left && cur_pos.x <= b2Rect.right)
                {
                    if (!b2Hglt)
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                        b1Hglt = false;
                        b2Hglt = true;
                        b3Hglt = false;
                    }
                }
                if (cur_pos.x >= b3Rect.left && cur_pos.x <= b3Rect.right)
                {
                    if (!b3Hglt)
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                        b1Hglt = false;
                        b2Hglt = false;
                        b3Hglt = true;
                    }
                }
                SetCursor(outCursor);
                return true;
            }
            else if (b1Hglt || b2Hglt || b3Hglt)
            {
                if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                b1Hglt = false;
                b2Hglt = false;
                b3Hglt = false;
            }
            SetCursor(mainCursor);
            return true;
        }
        else
        {
            if (!in_client)
            {
                in_client = false;
                pause = true;
            }
            if (b1Hglt || b2Hglt || b3Hglt)
            {
                if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                b1Hglt = false;
                b2Hglt = false;
                b3Hglt = false;
            }
            SetCursor(LoadCursorW(NULL, IDC_ARROW));
            return true;
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case mNew:
            pause = true;
            if (sound)mciSendString(L"play .\\res\\exclamation.wav", NULL, NULL, NULL);
            if (MessageBox(hwnd, L"Ако рестартираш, ще загубиш тази игра !\n\nНаистина ли рестартираш ?",
                L"Рестарт !", MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)
            {
                pause = false;
                break;
            }
            InitGame();
            pause = false;
            break;



        case mExit:
            SendMessage(hwnd, WM_CLOSE, NULL, NULL);
            break;

        }
        break;

    case WM_KEYDOWN:
        if (!Hero)break;
        if (Hero->GetFlag(JUMP_FLAG)) break;
        switch (wParam)
        {
        case VK_LEFT:
            Hero->dir = dirs::left;
            Hero->Move((float)(level), true, 0, Hero->y);
            break;

        case VK_RIGHT:
            Hero->dir = dirs::right;
            Hero->Move((float)(level), true, scr_width, Hero->y);
            break;

        case VK_UP:
            if (!Hero)break;
            else
            {
                gamedll::ATOM_CONTAINER conPlatforms((int)(vPlatforms.size()));
                
                for (int i = 0; i < vPlatforms.size(); i++)
                {
                    gamedll::ATOMS anAtom(vPlatforms[i].x, vPlatforms[i].y,
                        vPlatforms[i].GetWidth(), vPlatforms[i].GetHeight());
                    conPlatforms.push_back(anAtom);
                }
                Hero->Jumping(conPlatforms);
            }
            break;

        }
        break;

    default: return DefWindowProc(hwnd, ReceivedMsg, wParam, lParam);
    }

    return (LRESULT)(FALSE);
}

void CreateResources()
{
    int result = 0;
    CheckFile(Ltmp_file, &result);
    if (result == FILE_EXIST)ErrExit(eStarted);
    else
    {
        std::wofstream start(Ltmp_file);
        start << L"Game started at: " << std::chrono::system_clock::now();
        start.close();
    }

    Icon = (HICON)(LoadImage(NULL, L".\\res\\icon.ico", IMAGE_ICON, 128, 128, LR_LOADFROMFILE));
    if (!Icon)ErrExit(eIcon);
    mainCursor = LoadCursorFromFileW(L".\\res\\main.ani");
    outCursor = LoadCursorFromFileW(L".\\res\\out.ani");
    if (!mainCursor || !outCursor)ErrExit(eCursor);

    int win_x = GetSystemMetrics(SM_CXSCREEN) / 2 - (int)(scr_width / 2);
    if (GetSystemMetrics(SM_CXSCREEN) < win_x + (int)(scr_width) 
        || GetSystemMetrics(SM_CYSCREEN) < scr_height + 10)ErrExit(eScreen);

    bWinClass.lpszClassName = bWinClassName;
    bWinClass.hInstance = bIns;
    bWinClass.lpfnWndProc = &WinProc;
    bWinClass.hbrBackground = CreateSolidBrush(RGB(0, 0, 100));
    bWinClass.hIcon = Icon;
    bWinClass.hCursor = mainCursor;
    bWinClass.style = CS_DROPSHADOW;

    if (!RegisterClass(&bWinClass))ErrExit(eClass);

    bHwnd = CreateWindow(bWinClassName, L"НАШЕСТВИЕ НА ИЗВЪНЗЕМНИ !", WS_CAPTION | WS_SYSMENU, win_x, 10, (int)(scr_width),
        (int)(scr_height), NULL, NULL, bIns, NULL);
    if (!bHwnd)ErrExit(eWindow);
    else
    {
        ShowWindow(bHwnd, SW_SHOWDEFAULT);

        HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &iFactory);
        if (hr != S_OK)
        {
            LogError(L"Error creating iFactory object !");
            ErrExit(eD2D);
        }
        if (iFactory)
            hr = iFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(bHwnd,
                D2D1::SizeU((UINT32)(scr_width), (UINT32)(scr_height))), &Draw);
        if (hr != S_OK)
        {
            LogError(L"Error creating Draw object !");
            ErrExit(eD2D);
        }

        if (Draw)
        {
            D2D1_GRADIENT_STOP gStops[2]{};
            ID2D1GradientStopCollection* Coll = nullptr;

            gStops[0].position = 0;
            gStops[0].color = D2D1::ColorF(D2D1::ColorF::DarkViolet);
            gStops[1].position = 1.0f;
            gStops[1].color = D2D1::ColorF(D2D1::ColorF::LightBlue);

            hr = Draw->CreateGradientStopCollection(gStops, 2, &Coll);
            if (hr != S_OK)
            {
                LogError(L"Error creating GradientStopCollection object !");
                ErrExit(eD2D);
            }

            if (Coll)
            {
                hr = Draw->CreateRadialGradientBrush(D2D1::RadialGradientBrushProperties(D2D1::Point2F(scr_width / 2, 25.0f),
                    D2D1::Point2F(0, 0), scr_width / 2, 25.0f), Coll, &BckgBrush);
                if (hr != S_OK)
                {
                    LogError(L"Error creating RadialGradientBrush object !");
                    ErrExit(eD2D);
                }
                ClearHeap(&Coll);
            }

            hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &TextBrush);
            hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Yellow), &HgltBrush);
            hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkGray), &InactBrush);
            if (hr != S_OK)
            {
                LogError(L"Error creating Text Brushes object !");
                ErrExit(eD2D);
            }

            bmpGround = Load(L".\\res\\img\\field\\ground.png", Draw);
            if (!bmpGround)
            {
                LogError(L"Error loading bmpGround !");
                ErrExit(eD2D);
            }
            bmpPlatform = Load(L".\\res\\img\\field\\platform.png", Draw);
            if (!bmpPlatform)
            {
                LogError(L"Error loading bmpPlatform !");
                ErrExit(eD2D);
            }
            bmpPortal = Load(L".\\res\\img\\field\\portal.png", Draw);
            if (!bmpPortal)
            {
                LogError(L"Error loading bmpPortal !");
                ErrExit(eD2D);
            }
            bmpRIP = Load(L".\\res\\img\\field\\rip.png", Draw);
            if (!bmpRIP)
            {
                LogError(L"Error loading bmpRIP !");
                ErrExit(eD2D);
            }
            bmpVineD = Load(L".\\res\\img\\field\\vine_d.png", Draw);
            if (!bmpVineD)
            {
                LogError(L"Error loading bmpVineD !");
                ErrExit(eD2D);
            }
            bmpVineL = Load(L".\\res\\img\\field\\vine_l.png", Draw);
            if (!bmpVineL)
            {
                LogError(L"Error loading bmpVineL !");
                ErrExit(eD2D);
            }
            bmpVineR = Load(L".\\res\\img\\field\\vine_r.png", Draw);
            if (!bmpVineR)
            {
                LogError(L"Error loading bmpVineR !");
                ErrExit(eD2D);
            }

            for (int i = 0; i < 11; ++i)
            {
                wchar_t name[100]= L".\\res\\img\\field\\field1\\";
                wchar_t add[5] = L"\0";
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");
                bmpField1[i] = Load(name, Draw);
                if (!bmpField1[i])
                {
                    LogError(L"Error loading bmpField1 !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 8; ++i)
            {
                wchar_t name[100] = L".\\res\\img\\field\\field2\\";
                wchar_t add[5] = L"\0";
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");
                bmpField2[i] = Load(name, Draw);
                if (!bmpField2[i])
                {
                    LogError(L"Error loading bmpField2 !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 35; ++i)
            {
                wchar_t name[100] = L".\\res\\img\\field\\field3\\";
                wchar_t add[5] = L"\0";
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");
                bmpField3[i] = Load(name, Draw);
                if (!bmpField3[i])
                {
                    LogError(L"Error loading bmpField3 !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 35; ++i)
            {
                wchar_t name[100] = L".\\res\\img\\field\\intro\\";
                wchar_t add[5] = L"\0";
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");
                bmpIntro[i] = Load(name, Draw);
                if (!bmpIntro[i])
                {
                    LogError(L"Error loading bmpIntro !");
                    ErrExit(eD2D);
                }
            }

            for (int i = 0; i < 24; ++i)
            {
                wchar_t name[100] = L".\\res\\img\\hero\\attack_l\\";
                wchar_t add[5] = L"\0";
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");
                bmpHeroAttL[i] = Load(name, Draw);
                if (!bmpHeroAttL[i])
                {
                    LogError(L"Error loading bmpHeroAttL !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 24; ++i)
            {
                wchar_t name[100] = L".\\res\\img\\hero\\attack_r\\";
                wchar_t add[5] = L"\0";
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");
                bmpHeroAttR[i] = Load(name, Draw);
                if (!bmpHeroAttR[i])
                {
                    LogError(L"Error loading bmpHeroAttR !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 24; ++i)
            {
                wchar_t name[100] = L".\\res\\img\\hero\\walk_l\\";
                wchar_t add[5] = L"\0";
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");
                bmpHeroWalkL[i] = Load(name, Draw);
                if (!bmpHeroWalkL[i])
                {
                    LogError(L"Error loading bmpHeroWalkL !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 24; ++i)
            {
                wchar_t name[100] = L".\\res\\img\\hero\\walk_r\\";
                wchar_t add[5] = L"\0";
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");
                bmpHeroWalkR[i] = Load(name, Draw);
                if (!bmpHeroWalkR[i])
                {
                    LogError(L"Error loading bmpHeroWalkR !");
                    ErrExit(eD2D);
                }
            }

            for (int i = 0; i < 24; ++i)
            {
                wchar_t name[100] = L".\\res\\img\\evil1\\l\\";
                wchar_t add[5] = L"\0";
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");
                bmpEvil1L[i] = Load(name, Draw);
                if (!bmpEvil1L[i])
                {
                    LogError(L"Error loading bmpEvil1L !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 24; ++i)
            {
                wchar_t name[100] = L".\\res\\img\\evil1\\r\\";
                wchar_t add[5] = L"\0";
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");
                bmpEvil1R[i] = Load(name, Draw);
                if (!bmpEvil1R[i])
                {
                    LogError(L"Error loading bmpEvil1R !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 24; ++i)
            {
                wchar_t name[100] = L".\\res\\img\\evil2\\l\\";
                wchar_t add[5] = L"\0";
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");
                bmpEvil2L[i] = Load(name, Draw);
                if (!bmpEvil2L[i])
                {
                    LogError(L"Error loading bmpEvil2L !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 24; ++i)
            {
                wchar_t name[100] = L".\\res\\img\\evil2\\r\\";
                wchar_t add[5] = L"\0";
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");
                bmpEvil2R[i] = Load(name, Draw);
                if (!bmpEvil2R[i])
                {
                    LogError(L"Error loading bmpEvil2R !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 24; ++i)
            {
                wchar_t name[100] = L".\\res\\img\\evil3\\l\\";
                wchar_t add[5] = L"\0";
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");
                bmpEvil3L[i] = Load(name, Draw);
                if (!bmpEvil3L[i])
                {
                    LogError(L"Error loading bmpEvil3L !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 24; ++i)
            {
                wchar_t name[100] = L".\\res\\img\\evil3\\r\\";
                wchar_t add[5] = L"\0";
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");
                bmpEvil3R[i] = Load(name, Draw);
                if (!bmpEvil3R[i])
                {
                    LogError(L"Error loading bmpEvil3R !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 24; ++i)
            {
                wchar_t name[100] = L".\\res\\img\\evil4\\l\\";
                wchar_t add[5] = L"\0";
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");
                bmpEvil4L[i] = Load(name, Draw);
                if (!bmpEvil4L[i])
                {
                    LogError(L"Error loading bmpEvil4L !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 24; ++i)
            {
                wchar_t name[100] = L".\\res\\img\\evil4\\r\\";
                wchar_t add[5] = L"\0";
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");
                bmpEvil4R[i] = Load(name, Draw);
                if (!bmpEvil4R[i])
                {
                    LogError(L"Error loading bmpEvil4R !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 24; ++i)
            {
                wchar_t name[100] = L".\\res\\img\\evil5\\l\\";
                wchar_t add[5] = L"\0";
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");
                bmpEvil5L[i] = Load(name, Draw);
                if (!bmpEvil5L[i])
                {
                    LogError(L"Error loading bmpEvil5L !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 24; ++i)
            {
                wchar_t name[100] = L".\\res\\img\\evil5\\r\\";
                wchar_t add[5] = L"\0";
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");
                bmpEvil5R[i] = Load(name, Draw);
                if (!bmpEvil5R[i])
                {
                    LogError(L"Error loading bmpEvil5R !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 24; ++i)
            {
                wchar_t name[100] = L".\\res\\img\\evil6\\l\\";
                wchar_t add[5] = L"\0";
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");
                bmpEvil6L[i] = Load(name, Draw);
                if (!bmpEvil6L[i])
                {
                    LogError(L"Error loading bmpEvil6L !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 24; ++i)
            {
                wchar_t name[100] = L".\\res\\img\\evil6\\r\\";
                wchar_t add[5] = L"\0";
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");
                bmpEvil6R[i] = Load(name, Draw);
                if (!bmpEvil6R[i])
                {
                    LogError(L"Error loading bmpEvil6R !");
                    ErrExit(eD2D);
                }
            }
        }

        hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
            reinterpret_cast<IUnknown**> (&iWriteFactory));
        if (hr != S_OK)
        {
            LogError(L"Error creating iWriteFactory object !");
            ErrExit(eD2D);
        }

        if (iWriteFactory)
        {
            hr = iWriteFactory->CreateTextFormat(L"Segoe Print", NULL, DWRITE_FONT_WEIGHT_EXTRA_BLACK,
                DWRITE_FONT_STYLE_OBLIQUE, DWRITE_FONT_STRETCH_NORMAL, 24, L"", &nrmFormat);
            hr = iWriteFactory->CreateTextFormat(L"Segoe Print", NULL, DWRITE_FONT_WEIGHT_EXTRA_BLACK,
                DWRITE_FONT_STYLE_OBLIQUE, DWRITE_FONT_STRETCH_NORMAL, 36, L"", &midFormat);
            hr = iWriteFactory->CreateTextFormat(L"Segoe Print", NULL, DWRITE_FONT_WEIGHT_EXTRA_BLACK,
                DWRITE_FONT_STYLE_OBLIQUE, DWRITE_FONT_STRETCH_NORMAL, 72, L"", &bigFormat);

            if (hr != S_OK)
            {
                LogError(L"Error creating TextFormat object !");
                ErrExit(eD2D);
            }
        }
    }

    int intro_frame = 0;

    if (Draw && bigFormat && TextBrush)
    {
        PlaySound(L".\\res\\snd\\intro.wav", NULL, SND_ASYNC);
        for (int i = 0; i < 250; i++)
        {
            Draw->BeginDraw();
            Draw->DrawBitmap(bmpIntro[intro_frame], D2D1::RectF(0, 0, scr_width, scr_height));
            if (Randomizer.generate(0, 10) == 3)
            {
                Draw->DrawText(L"ИЗВЪНЗЕМНА АТАКА !\n\n    dev. Daniel", 36, bigFormat,
                    D2D1::RectF(10.0f, 100.0f, scr_width, scr_height), TextBrush);
                mciSendString(L"play .\\res\\snd\\buzz.wav", NULL, NULL, NULL);
                Draw->EndDraw();
                Sleep(70);
            }
            else Draw->EndDraw();
            ++intro_frame;
            if (intro_frame > 34)intro_frame = 0;
        }
        Draw->BeginDraw();
        Draw->DrawBitmap(bmpIntro[intro_frame], D2D1::RectF(0, 0, scr_width, scr_height));
        Draw->DrawText(L"ИЗВЪНЗЕМНА АТАКА !\n\n    dev. Daniel", 36, bigFormat,
            D2D1::RectF(10.0f, 100.0f, scr_width, scr_height), TextBrush);
        Draw->EndDraw();
        Sleep(2000);
    }

}

///////////////////////////////////////


int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    bIns = hInstance;
    if (!bIns)
    {
        LogError(L"Error obtaining hInstance from Windows !");
        ErrExit(eClass);
    }
    CreateResources();


    while (bMsg.message != WM_QUIT)
    {
        if ((bRet = PeekMessage(&bMsg, bHwnd, NULL, NULL, PM_REMOVE)) != 0)
        {
            if (bRet == -1)ErrExit(eMsg);
            TranslateMessage(&bMsg);
            DispatchMessage(&bMsg);
        }
        if (pause)
        {
            if (show_help) continue;
            Draw->BeginDraw();
            Draw->Clear(D2D1::ColorF(D2D1::ColorF::Chocolate));
            if (InactBrush && bigFormat)
                Draw->DrawTextW(L"ПАУЗА", 6, bigFormat, D2D1::RectF(scr_width / 2 - 50.0f, 200.0f,
                    scr_width, scr_height), TextBrush);
            Draw->EndDraw();
            continue;
        }
        /////////////////////////////
        if (Hero)
        {
            ambient_dir = dirs::stop;
            if (Hero->dir == dirs::left)ambient_dir = dirs::right;
            else if (Hero->dir == dirs::right)ambient_dir = dirs::left;

            if (Hero->GetFlag(JUMP_FLAG))
            {
                gamedll::ATOM_CONTAINER conPlatforms((int)(vPlatforms.size()));

                for (int i = 0; i < (int)(vPlatforms.size()); i++)
                {
                    gamedll::ATOMS anAtom(vPlatforms[i].x, vPlatforms[i].y,
                        vPlatforms[i].GetWidth(), vPlatforms[i].GetHeight());
                    conPlatforms.push_back(anAtom);
                }
                Hero->Jumping(conPlatforms);
            }
            else
            {
                Hero->SetFlag(FALL_FLAG);

                gamedll::ATOM_CONTAINER conPlatforms((int)(vPlatforms.size()));
                for (int i = 0; i < vPlatforms.size(); i++)
                {
                    gamedll::ATOMS anAtom(vPlatforms[i].x, vPlatforms[i].y,
                        vPlatforms[i].GetWidth(), vPlatforms[i].GetHeight());
                    conPlatforms.push_back(anAtom);
                }
                Hero->Falling(conPlatforms);
                if (!Hero->GetFlag(FALL_FLAG))
                {
                    if (Hero->dir == dirs::left) Hero->Move((float)(level), true, 0, Hero->y);
                    else if (Hero->dir == dirs::right) Hero->Move((float)(level), true, scr_width, Hero->y);
                }
            }
            
        }
        if (!vFields.empty())
        {
            bool left_needed = false;
            bool right_needed = false;

            for (std::vector<gamedll::FIELD>::iterator field = vFields.begin(); field < vFields.end(); field++)
            {
                if (ambient_dir == dirs::left)
                {
                    field->dir = dirs::left;
                    field->Move((float)(level + 1.0f));
                    if (field->GetFlag(LEFT_FLAG))
                    {
                        vFields.erase(field);
                        right_needed = true;
                        break;
                    }
                }
                else if (ambient_dir == dirs::right)
                {
                    field->dir = dirs::right;
                    field->Move((float)(level + 1.0f));
                    if (field->GetFlag(RIGHT_FLAG))
                    {
                        vFields.erase(field);
                        left_needed = true;
                        break;
                    }
                }
            }

            if (left_needed)
                vFields.insert(vFields.begin(), gamedll::FIELD(static_cast<fields>(Randomizer.generate(0, 2)), 
                    vFields.begin()->x - scr_width));
            if (right_needed)
                vFields.push_back(gamedll::FIELD(static_cast<fields>(Randomizer.generate(0, 2)),
                    vFields.back().ex));
        }
        if (vPlatforms.size() <= 8)
        {
            if (Randomizer.generate(0, 150) == 32)
            {
                bool is_ok = false;
                gamedll::FIELD dummyPlatform(fields::platform, scr_width + (float)(Randomizer.generate(100, 250)));
                while (!is_ok)
                {
                    is_ok = true;
                    if (!vPlatforms.empty())
                    {
                        for (std::vector<gamedll::FIELD>::iterator field = vPlatforms.begin(); field < vPlatforms.end(); field++)
                        {
                            if (field->GetType() == fields::platform &&
                                !(dummyPlatform.x >= field->ex || dummyPlatform.ex <= field->x
                                    || dummyPlatform.y >= field->ey || dummyPlatform.ey <= field->y))
                            {
                                is_ok = false;
                                dummyPlatform.x += (float)(Randomizer.generate(100, 250));
                                dummyPlatform.SetEdges();
                                break;
                            }
                        }
                    }
                }
                vPlatforms.push_back(dummyPlatform);
            }
        }
        
        if (!vPlatforms.empty())
        {
            for (std::vector<gamedll::FIELD>::iterator field = vPlatforms.begin(); field < vPlatforms.end(); field++)
            {
                if (field->GetType() == fields::platform)
                {
                    field->dir = ambient_dir;
                    field->Move((float)(level + 1.0f));
                    if (field->GetFlag(LEFT_FLAG | RIGHT_FLAG))
                    {
                        vPlatforms.erase(field);
                        break;
                    }
                }
            }
        }
        
        //DRAW THINGS ***************
        if (Draw && nrmFormat && TextBrush && HgltBrush && InactBrush && BckgBrush)
        {
            Draw->BeginDraw();
            Draw->FillRectangle(D2D1::RectF(0, 0, scr_width, 50.0f), BckgBrush);
            if (name_set) Draw->DrawText(L"ИМЕ НА ИГРАЧ", 13, nrmFormat, b1Rect, InactBrush);
            else
            {
                if (b1Hglt) Draw->DrawText(L"ИМЕ НА ИГРАЧ", 13, nrmFormat, b1Rect, HgltBrush);
                else Draw->DrawText(L"ИМЕ НА ИГРАЧ", 13, nrmFormat, b1Rect, TextBrush);
            }
            if (b2Hglt) Draw->DrawText(L"ЗВУЦИ ON / OFF", 15, nrmFormat, b2Rect, HgltBrush);
            else Draw->DrawText(L"ЗВУЦИ ON / OFF", 15, nrmFormat, b2Rect, TextBrush);
            if (b3Hglt) Draw->DrawText(L"ПОМОЩ ЗА ИГРАТА", 16, nrmFormat, b3Rect, HgltBrush);
            else Draw->DrawText(L"ПОМОЩ ЗА ИГРАТА", 16, nrmFormat, b3Rect, TextBrush);
        }
        
        if (!vFields.empty())
        {
            for (std::vector<gamedll::FIELD>::iterator field = vFields.begin(); field < vFields.end(); ++field)
            {
                switch (field->GetType())
                {
                case fields::field1:
                    Draw->DrawBitmap(bmpField1[field->GetFrame()], D2D1::RectF(field->x, field->y, field->ex, field->ey));
                    break;

                case fields::field2:
                    Draw->DrawBitmap(bmpField2[field->GetFrame()], D2D1::RectF(field->x, field->y, field->ex, field->ey));
                    break;

                case fields::field3:
                    Draw->DrawBitmap(bmpField3[field->GetFrame()], D2D1::RectF(field->x, field->y, field->ex, field->ey));
                    break;
                }
            }
        }

        if (!vPlatforms.empty())
        {
            for (std::vector<gamedll::FIELD>::iterator field = vPlatforms.begin(); field < vPlatforms.end(); field++)
            {
                switch (field->GetType())
                {
                case fields::platform:
                    Draw->DrawBitmap(bmpPlatform, D2D1::RectF(field->x, field->y, field->ex, field->ey));
                    break;

                case fields::ground_platform:
                    Draw->DrawBitmap(bmpGround, D2D1::RectF(field->x, field->y, field->ex, field->ey));
                    break;
                }
            }
        }

        if (Hero)
        {
            if (hero_attacking)
            {
                switch (Hero->dir)
                {
                case dirs::left:
                    Draw->DrawBitmap(bmpHeroAttL[Hero->GetFrame()], Resizer(bmpHeroAttL[Hero->GetFrame()], Hero->x, Hero->y));
                    break;

                case dirs::right:
                    Draw->DrawBitmap(bmpHeroAttR[Hero->GetFrame()], Resizer(bmpHeroAttR[Hero->GetFrame()], Hero->x, Hero->y));
                    break;

                case dirs::stop:
                    Draw->DrawBitmap(bmpHeroAttR[Hero->GetFrame()], Resizer(bmpHeroAttR[Hero->GetFrame()], Hero->x, Hero->y));
                    break;
                }
            }
            else
            {
                switch (Hero->dir)
                {
                case dirs::left:
                    Draw->DrawBitmap(bmpHeroWalkL[Hero->GetFrame()], Resizer(bmpHeroWalkL[Hero->GetFrame()], Hero->x, Hero->y));
                    break;

                case dirs::right:
                    Draw->DrawBitmap(bmpHeroWalkR[Hero->GetFrame()], Resizer(bmpHeroWalkR[Hero->GetFrame()], Hero->x, Hero->y));
                    break;

                case dirs::stop:
                    Draw->DrawBitmap(bmpHeroWalkR[0], Resizer(bmpHeroWalkR[Hero->GetFrame()], Hero->x, Hero->y));
                    break;
                }
            }
        }

        /////////////////////////////

        Draw->EndDraw();
    }


    std::remove(tmp_file);
    ReleaseResources();

    return (int) bMsg.wParam;
}