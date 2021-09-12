#include <windows.h> // заголовочный файл, содержащий WINAPI
#include <math.h>
#pragma comment(lib, "Msimg32.lib")


const int update = 10; // частота обновления экрана в миллисекундах
const int speed = 10; // ускорение при нажатии клавиш
const float slowing = 0.985; // снижение скорости за время одного обновления экрана 
const float lossing = 0.8; // снижение скорости при отскоке
const float mouseSpeed = 20; // ускорение объекта при запуске мышкой
const float wheelSpeed = 0.2; // ускорение объекта при прокрутке колеса



// Прототип функции обработки сообщений с пользовательским названием:
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
TCHAR mainMessage[] = L"Какой то-текст!"; // строка с сообщением

// Управляющая функция:
int WINAPI WinMain(HINSTANCE hInst, // дескриптор экземпляра приложения
    HINSTANCE hPrevInst, // не используем
    LPSTR lpCmdLine, // не используем
    int nCmdShow) // режим отображения окошка
{
    TCHAR szClassName[] = L"Мой класс"; // строка с именем класса
    HWND hMainWnd; // создаём дескриптор будущего окошка
    MSG msg; // создём экземпляр структуры MSG для обработки сообщений

    WNDCLASSEX wc; // создаём экземпляр, для обращения к членам класса WNDCLASSEX
    wc.cbSize = sizeof(wc); // размер структуры (в байтах)
    wc.style = CS_HREDRAW | CS_VREDRAW; // стиль класса окошка
    wc.lpfnWndProc = WndProc; // указатель на пользовательскую функцию
    wc.lpszMenuName = NULL; // указатель на имя меню (у нас его нет)
    wc.lpszClassName = szClassName; // указатель на имя класса
    wc.cbWndExtra = NULL; // число освобождаемых байтов в конце структуры
    wc.cbClsExtra = NULL; // число освобождаемых байтов при создании экземпляра приложения
    wc.hIcon = LoadIcon(NULL, IDI_WINLOGO); // декриптор пиктограммы
    wc.hIconSm = LoadIcon(NULL, IDI_WINLOGO); // дескриптор маленькой пиктограммы (в трэе)
    wc.hCursor = LoadCursor(NULL, IDC_ARROW); // дескриптор курсора
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); // дескриптор кисти для закраски фона окна
    wc.hInstance = hInst; // указатель на строку, содержащую имя меню, применяемого для класса
    if (!RegisterClassEx(&wc)) {
        // в случае отсутствия регистрации класса:
        MessageBox(NULL, L"Не получилось зарегистрировать класс!", L"Ошибка", MB_OK);
        return NULL; // возвращаем, следовательно, выходим из WinMain
    }

    // Функция, создающая окошко:
    hMainWnd = CreateWindow(
        szClassName, // имя класса
        L"Полноценная оконная процедура", // имя окошка (то что сверху)
        WS_OVERLAPPEDWINDOW | WS_VSCROLL, // режимы отображения окошка
        CW_USEDEFAULT, // позиция окошка по оси х
        NULL, // позиция окошка по оси у (раз дефолт в х, то писать не нужно)
        CW_USEDEFAULT, // ширина окошка
        NULL, // высота окошка (раз дефолт в ширине, то писать не нужно)
        (HWND)NULL, // дескриптор родительского окна
        NULL, // дескриптор меню
        HINSTANCE(hInst), // дескриптор экземпляра приложения
        NULL); // ничего не передаём из WndProc
    if (!hMainWnd) {
        // в случае некорректного создания окошка (неверные параметры и тп):
        MessageBox(NULL, L"Не получилось создать окно!", L"Ошибка", MB_OK);
        return NULL;
    }

    ShowWindow(hMainWnd, nCmdShow); // отображаем окошко
    ShowScrollBar(hMainWnd, SB_BOTH, FALSE); // скрыть скроллбары
    UpdateWindow(hMainWnd); // обновляем окошко

    while (GetMessage(&msg, NULL, NULL, NULL)) { // извлекаем сообщения из очереди, посылаемые фу-циями, ОС
        TranslateMessage(&msg); // интерпретируем сообщения
        DispatchMessage(&msg); // передаём сообщения обратно ОС
    }
        
    return msg.wParam; // возвращаем код выхода из приложения
}

bool arrow[4] = {}; // массив для сохранения ужатых стрелок
bool lbutton = false; // ужата ли ЛКМ
POINT d = { 0, 0 }; // разница координат курсора и объекта во время ужатия ЛКМ
POINT cursor0 = { 0, 0 }; // положение курсора при ужатии ЛКМ
POINT cursor1 = { 0, 0 }; // положение курсора при отжатии ЛКМ
SYSTEMTIME time0; // время ужатия ЛКМ
SYSTEMTIME time1; // время отжатия ЛКМ
int wheelDeltaY = 0; // прокрутка колесика
int wheelDeltaX = 0; // прокрутка колесика с ужатой shift

class Figure {
    public:
        POINT pos = { 100, 100 };
        int width = 180;
        int height = 108;
        HBITMAP hBitmap;

        POINT dir = {0, 0}; // вектор движения объекта

        Figure(LPCWSTR path) {
            hBitmap = (HBITMAP)LoadImage(NULL, path, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
        }

        void Move(HWND hWnd) {
            RECT rect;
            GetClientRect(hWnd, &rect);
            int wndWidth = rect.right - rect.left;
            int wndHeight = rect.bottom - rect.top;

            pos.x += dir.x;
            pos.y += dir.y;

            // обработка отскоков
            int dx, dy;

            if ((pos.x + width >= wndWidth) ||
                (pos.y + height >= wndHeight) ||
                (pos.x <= 0) || (pos.y <= 0)) { // если объект попал за границы окна
                dir.x *= lossing;
                dir.y *= lossing; // снижаем скорость объекта, т.к. он отскакивает
            }

            if (pos.x + width >= wndWidth) { // выход по правой границе
                dx = pos.x + width - wndWidth;
                dx = (int)round(dx * lossing);
                pos.x = wndWidth - dx - width;
                dir.x *= -1; // при отскоке направление движения по x меняется 
            }
            if (pos.y + height >= wndHeight) { // выход по нижней границе
                dy = pos.y + height - wndHeight;
                dy = (int)round(dy * lossing);
                pos.y = wndHeight - dy - height;
                dir.y *= -1; // при отскоке направление движения по y меняется 
            }
            if (pos.x <= 0) { // выход по левой границе
                dx = -pos.x;
                dx = (int)round(dx * lossing);
                pos.x = dx;
                dir.x *= -1; // при отскоке направление движения по x меняется 
            }
            if (pos.y <= 0) { // выход по верхней границе
                dy = -pos.y;
                dy = (int)round(dy * lossing);
                pos.y = dy;
                dir.y *= -1; // при отскоке направление движения по y меняется 
            }
        }

        void Draw(HDC hDC, HWND hWnd) {
            HDC memDC = CreateCompatibleDC(hDC);

            RECT rcClientRect;
            GetClientRect(hWnd, &rcClientRect);

            HBITMAP bmp = CreateCompatibleBitmap(hDC,
                rcClientRect.right - rcClientRect.left,
                rcClientRect.bottom - rcClientRect.top);

            SelectObject(memDC, bmp);

            HDC hCompatibleDC = CreateCompatibleDC(memDC);
            SelectObject(hCompatibleDC, hBitmap);
            FillRect(memDC, &rcClientRect, (HBRUSH)GetStockObject(WHITE_BRUSH));
            TransparentBlt(memDC, pos.x, pos.y, width, height, hCompatibleDC, 0, 0, 900, 540, RGB(255, 0, 0));
            DeleteDC(hCompatibleDC);

            BitBlt(hDC, 0, 0, rcClientRect.right - rcClientRect.left,
                rcClientRect.bottom - rcClientRect.top, memDC, 0, 0, SRCCOPY);

           //DeleteObject(bmp); // delete bitmap since it is no longer required
            DeleteDC(memDC);   // delete memory DC since it is no longer required
        }

        bool IsCursorInside(HWND hWnd)
        {
            POINT cursor;
            RECT rect;
            GetCursorPos(&cursor);
            GetClientRect(hWnd, &rect);
            cursor.x -= rect.left;
            cursor.y -= rect.top;
            if ((cursor.x > pos.x) &&
                (cursor.x < pos.x + width) &&
                (cursor.y > pos.y) &&
                (cursor.y < pos.y + height)) return true;
            return false;
        }
};

Figure figure = *(new Figure(L"test.bmp"));

LRESULT CALLBACK WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
    HDC hDC, hCompatibleDC;
    PAINTSTRUCT PaintStruct;
    HANDLE hBitmap, hOldBitmap;
    RECT Rect;
    BITMAP Bitmap;
    
    switch (Msg)
    {
    case WM_CREATE:
        SetTimer(hWnd, 0, update, NULL);
        return 0;
    case WM_TIMER:
        figure.dir.x *= slowing;
        figure.dir.y *= slowing; // объект постепенно тормозит

        if (arrow[0]) figure.dir.y -= speed; // если ужата клавиша вверх
        if (arrow[1]) figure.dir.x += speed; // если ужата клавиша вправо
        if (arrow[2]) figure.dir.y += speed; // если ужата клавиша вниз
        if (arrow[3]) figure.dir.x -= speed; // если ужата клавиша влево

        if (lbutton)
        {
            POINT cursor;
            RECT rect;
            GetCursorPos(&cursor);
            GetClientRect(hWnd, &rect);

            if ((cursor.x <= rect.left) || // выход за пределы окна равноценно отпусканию мыши
                (cursor.x >= rect.right) ||
                (cursor.y <= rect.top) ||
                (cursor.y >= rect.bottom)) PostMessage(hWnd, WM_LBUTTONUP, NULL, NULL);

            //figure.pos.x = (cursor.x - rect.left - 8) - figure.width / 2;
            //figure.pos.y = (cursor.y - rect.top - 32) - figure.height / 2;
        }
        else 
        if (!lbutton) {
            float dt = (time1.wMinute - time0.wMinute) * 60 * 1000 +
                      (time1.wSecond - time0.wSecond) * 1000 +
                      (time1.wMilliseconds - time0.wMilliseconds);
            if (dt != 0) {
                figure.dir.x += (int)round((cursor1.x - cursor0.x) * mouseSpeed / dt);
                figure.dir.y += (int)round((cursor1.y - cursor0.y) * mouseSpeed / dt);
            }
            time0 = time1;
        }

        figure.dir.y -= wheelDeltaY * wheelSpeed;
        figure.dir.x -= wheelDeltaX * wheelSpeed;
        wheelDeltaY = 0;
        wheelDeltaX = 0;

        if ((figure.dir.x != 0) || (figure.dir.y != 0)) {
            figure.Move(hWnd); // двигаем объект на вектор передвижения
            InvalidateRect(hWnd, NULL, FALSE);
            UpdateWindow(hWnd);
        }
        return 0;
    case WM_PAINT:
        hDC = BeginPaint(hWnd, &PaintStruct);
        figure.Draw(hDC, hWnd);
        EndPaint(hWnd, &PaintStruct);
        return 0;
    case WM_KEYDOWN:
        switch (wParam)
        {
            case VK_UP: arrow[0] = true; break;
            case VK_RIGHT: arrow[1] = true; break;
            case VK_DOWN: arrow[2] = true; break;
            case VK_LEFT: arrow[3] = true; break;
        }
        return 0;
    case WM_KEYUP:
        switch (wParam)
        {
            case VK_UP: arrow[0] = false; break;
            case VK_RIGHT: arrow[1] = false; break;
            case VK_DOWN: arrow[2] = false; break;
            case VK_LEFT: arrow[3] = false; break;
        }
        return 0;
    case WM_LBUTTONDOWN:
        lbutton = true;
        GetSystemTime(&time0);
        GetCursorPos(&cursor0);
        return 0;
    case WM_LBUTTONUP:
        lbutton = false;
        GetSystemTime(&time1);
        GetCursorPos(&cursor1);
        return 0;
    case WM_MOUSEWHEEL:
        if (GET_KEYSTATE_WPARAM(wParam) == MK_SHIFT) 
            wheelDeltaX += GET_WHEEL_DELTA_WPARAM(wParam); else
            wheelDeltaY += GET_WHEEL_DELTA_WPARAM(wParam);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, Msg, wParam, lParam);
}