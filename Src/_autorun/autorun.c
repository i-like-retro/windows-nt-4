#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <assert.h>

#define CP_ACP 0

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_IMPLEMENTATION
#define NK_GDI_IMPLEMENTATION
#include "nuklear.h"
#include "nuklgdi.h"

typedef struct Command {
    struct Command* next;
    const char* command;
} Command;

typedef struct Item {
    struct Item* next;
    const char* name;
    Command* firstCommand;
    Command* lastCommand;
} Item;

static struct nk_context* ctx;
static HWND hWnd;
static char prefixPath[MAX_PATH];
static char fileName[MAX_PATH];
static int windowWidth;
static int windowHeight;
static Item* firstItem;
static Item* lastItem;

static void AddCommand(Item* item, const char* cmd)
{
    Command* command = (Command*)malloc(sizeof(Command));

    command->next = NULL;
    command->command = (cmd ? strdup(cmd) : NULL);
    if (cmd && !command->command)
        abort();

    if (!item->firstCommand)
        item->firstCommand = command;
    else
        item->lastCommand->next = command;
    item->lastCommand = command;
}

static Item* AddItem(const char* name)
{
    Item* item = (Item*)malloc(sizeof(Item));

    item->next = NULL;
    item->firstCommand = NULL;
    item->lastCommand = NULL;
    item->name = (name ? strdup(name) : NULL);
    if (name && !item->name)
        abort();

    if (!firstItem)
        firstItem = item;
    else
        lastItem->next = item;
    lastItem = item;

    return item;
}

#define SEPARATOR_HEIGHT 10
#define BUTTON_HEIGHT 30

static void GUI(void)
{
    if (!nk_begin(ctx, "Autorun", nk_rect(0, 0, windowWidth, windowHeight), 0)) {
        nk_end(ctx);
        return;
    }

    for (const Item* item = firstItem; item; item = item->next) {
        if (!item->name) {
            nk_layout_row_static(ctx, SEPARATOR_HEIGHT, windowWidth - 20, 1);
            nk_label(ctx, "", NK_TEXT_ALIGN_CENTERED);
        } else {
            nk_layout_row_static(ctx, BUTTON_HEIGHT, windowWidth - 20, 1);
            if (nk_button_label(ctx, item->name)) {
                ShowWindow(hWnd, SW_HIDE);

                for (const Command* cmd = item->firstCommand; cmd; ) {
                    if (!cmd->command)
                        ExitProcess(0);

                    snprintf(fileName, sizeof(fileName), cmd->command, prefixPath);
                    int err = system(fileName);
                    if (err != 0) {
                        char buf[1024];
                        snprintf(buf, sizeof(buf), "Command exited with code %d:\n%s", err, fileName);
                        int r = MessageBox(NULL, buf, "Error", MB_ICONERROR | MB_ABORTRETRYIGNORE | MB_SETFOREGROUND);
                        if (r == IDABORT) {
                            ShowWindow(hWnd, SW_SHOW);
                            return;
                        } else if (r == IDRETRY)
                            continue;
                    }
                    cmd = cmd->next;
                }

                ShowWindow(hWnd, SW_SHOW);
            }
        }
    }

    nk_layout_row_static(ctx, SEPARATOR_HEIGHT, windowWidth - 20, 1);
    nk_label(ctx, "", NK_TEXT_ALIGN_CENTERED);

    nk_end(ctx);
}

static LRESULT CALLBACK WndProc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    if (nk_gdi_handle_event(wnd, msg, wparam, lparam))
        return 0;

    return DefWindowProc(wnd, msg, wparam, lparam);
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    GdiFont* font;

    WNDCLASS wc;
    ATOM atom;
    RECT rect = { 0, 0, 300, 400 };
    DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    DWORD dwExStyle = WS_EX_APPWINDOW;
    HDC hDC;
    BOOL running = TRUE;
    BOOL needsRefresh = TRUE;
    int w, h;

    (void)hInstance;
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nShowCmd;

    GetModuleFileName(NULL, prefixPath, sizeof(prefixPath));
    char* p = strrchr(prefixPath, '\\');
    if (p)
        p[1] = 0;
    else
        prefixPath[0] = 0;

    strcpy(fileName, prefixPath);
    strcat(fileName, "AUTORUN.DAT");
    FILE* f = fopen(fileName, "r");
    if (!f) {
        MessageBox(NULL, "Can't open AUTORUN.DAT.", "Error", MB_ICONERROR | MB_OK | MB_SETFOREGROUND);
        return 1;
    }

    char buf[256];
    Item* item = NULL;
    int wantedH = SEPARATOR_HEIGHT + 10;
    while (fgets(buf, sizeof(buf), f)) {
        size_t len = strlen(buf);
        if (len > 0 && buf[len - 1] == '\n')
            buf[len - 1] = 0;

        if (buf[0] == 0)
            continue;
        else if (buf[0] == '>' && buf[1] == '>' && buf[2] == ' ') {
            item = AddItem(&buf[3]);
            wantedH += BUTTON_HEIGHT + 4;
        } else if (buf[0] == '-') {
            AddItem(NULL);
            item = NULL;
            wantedH += SEPARATOR_HEIGHT + 8;
        } else if (buf[0] == '!')
            AddCommand(item, NULL);
        else
            AddCommand(item, buf);
    }

    fclose(f);

    memset(&wc, 0, sizeof(wc));
    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = TEXT("NuklearWindowClass");
    atom = RegisterClass(&wc);

    rect.bottom = rect.top + wantedH;
    AdjustWindowRectEx(&rect, dwStyle, FALSE, dwExStyle);

    int screenH = GetSystemMetrics(SM_CYSCREEN);
    if (rect.bottom - rect.top > screenH - 20)
        rect.bottom = rect.top + screenH - 20;

    w = rect.right - rect.left;
    h = rect.bottom - rect.top;
    rect.left = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
    rect.top  = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;

    hWnd = CreateWindowEx(dwExStyle, wc.lpszClassName, TEXT("Autorun"),
        dwStyle | WS_VISIBLE, rect.left, rect.top, w, h, NULL, NULL, wc.hInstance, NULL);
    hDC = GetDC(hWnd);

    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    windowWidth = clientRect.right - clientRect.left;
    windowHeight = clientRect.bottom - clientRect.top;

    font = nk_gdifont_create("Arial", 16);
    ctx = nk_gdi_init(font, hDC, windowWidth, windowHeight);

    while (running) {
        MSG msg;

        nk_input_begin(ctx);

        if (needsRefresh)
            needsRefresh = FALSE;
        else {
            if (GetMessage(&msg, NULL, 0, 0) <= 0)
                running = FALSE;
            else {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            needsRefresh = TRUE;
        }

        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT)
                running = FALSE;
            else {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
                needsRefresh = TRUE;
            }
        }

        nk_input_end(ctx);

        GUI();

        nk_gdi_render(nk_rgb(30, 30, 30));
    }

    nk_gdifont_del(font);
    ReleaseDC(hWnd, hDC);
    UnregisterClass(wc.lpszClassName, wc.hInstance);

    return 0;
}
