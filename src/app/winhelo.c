int api_openwin(char* buf, int width, int height, int col_inv, char* title);
void api_end();

char buf[150*50];

void tmos_main(void) {
    int win = api_openwin(buf, 150, 50, -1, "hello");
    api_end();
}
