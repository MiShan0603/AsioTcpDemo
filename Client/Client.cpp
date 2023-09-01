// PlayerNode.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "Client.h"
#include "Utils/Tool.h"
#define MAX_LOADSTRING 100

#include <iostream>
#include "Net/TcpClient.h"

using namespace std;

TcpClientPtr g_clientPtr = nullptr;

HINSTANCE g_hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
HWND g_hWnd = NULL;

bool g_buttonDown = false;

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

#define NET_ERROR WM_USER + 0x01

void OnEvent(TcpClientPtr clientPtr, TCP_CLIENT_EVENT ev, string msg)
{
    if (ev == TCP_EVENT_CONNECTED)
    {
        X_PRINT("connect success");
        return;
    }
    else
    {
        cout << "Event callback:" << msg << endl;
        OutputDebugStringA(msg.c_str());

        ::PostMessage(g_hWnd, NET_ERROR, 0, 0);
    }
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR lpCmdLine,  _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此处放置代码。

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CLIENT, szWindowClass, MAX_LOADSTRING);
    
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CLIENT));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
    RegisterClassExW(&wcex);

    g_hInst = hInstance;
    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, 
                    0, 0, 640, 360, nullptr, nullptr, hInstance, nullptr);
    if (!hWnd)
    {
        return FALSE;
    }

    Utils::Tool::Instance()->CenterWindow(hWnd);
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    g_hWnd = hWnd;

    g_clientPtr = boost::make_shared<TcpClient>();
    g_clientPtr->SetEventCallBack(boost::bind(OnEvent, g_clientPtr, _1, _2));
    g_clientPtr->AsyncConnect("127.0.0.1", 19000);

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CLIENT));

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择:
            switch (wmId)
            {
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case NET_ERROR:
    {
        g_clientPtr = nullptr;
        g_clientPtr = boost::make_shared<TcpClient>();
        g_clientPtr->SetEventCallBack(boost::bind(OnEvent, g_clientPtr, _1, _2));
        g_clientPtr->AsyncConnect("127.0.0.1", 19000);
    }
        
        break;
    case WM_LBUTTONDOWN:
        g_buttonDown = true;
        break;
    case WM_MOUSEMOVE:
        if (g_buttonDown && g_clientPtr)
        {
            struct MouseInfo
            {
                Cmd_Header header;
                Cmd_MouseInfo info;
            };

            MouseInfo mouseInfo;
            mouseInfo.header.cmdLen = sizeof(Cmd_MouseInfo);
            mouseInfo.header.cmdType = CmdType_MouseInfo;

            mouseInfo.info.x = LOWORD(lParam);
            mouseInfo.info.y = HIWORD(lParam);

            g_clientPtr->write(&mouseInfo, sizeof(MouseInfo));
        }
        break;
    case WM_LBUTTONUP:
        g_buttonDown = false;
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}