void api_putchar(int c);
void api_end();

void tmos_main(void) {
    for (;;) {
        api_putchar('a');
    }
    api_end();
}
