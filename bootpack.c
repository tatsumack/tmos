void io_hlt(void);

void tmos_main(void)
{
fin:
    io_hlt();
    goto fin;
}
