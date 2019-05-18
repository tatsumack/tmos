void api_putchar(int c);
void api_end();

void tmos_main(void) {
    char a[100];
    a[10] = 'A';
    api_putchar(a[10]);
    a[102] = 'B';
    api_putchar(a[102]);
    a[140] = 'C';
    api_putchar(a[140]);
    api_end();
}
