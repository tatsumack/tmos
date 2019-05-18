int api_win_open(char* buf, int width, int height, int col_inv, char* title);
void api_win_putstr(int win, int x, int y, int col, int len, char* str);
void api_win_drawrec(int win, int x0, int y0, int x1, int y1, int col);
void api_end();

char buf[150*50];

void tmos_main(void) {
    int win = api_win_open(buf, 150, 50, -1, "hello");
    api_win_drawrec(win, 8, 36, 141, 43, 3);
    api_win_putstr(win, 28, 28, 0, 12, "hello, world");
    api_end();
}
