void api_putchar(int c);
void api_end();

void tmos_main(void) {
    api_putchar('A');
    api_end();
}
