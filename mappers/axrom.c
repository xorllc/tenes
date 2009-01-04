/* INES Mapper #7 - AxROM */

int mapper7_bank;

int mapper7_init (void)
{
    nes.rom.mirror_mode = MIRROR_ONESCREEN;
    //mapper7_bank = (nes.rom.prg_size / 0x8000) - 1;
    mapper7_bank = 0;
    return 1;
}

void mapper7_shutdown (void)
{
}

void mapper7_write (register word Addr,register byte Value)
{
    int bank = (Value & 7) % (nes.rom.prg_size / 0x8000);
    //printf("axrom: wrote %02X. switching to %i.\n", (int)Value, bank);
    mapper7_bank = bank;
}

byte mapper7_read(register word Addr)
{

    return nes.rom.prg[mapper7_bank * 0x8000 + (Addr & 0x7FFF)];
}

struct mapper_methods mapper_axrom = 
{
    mapper7_init,
    mapper7_shutdown,
    mapper7_write,
    mapper7_read,
    mapper0_scanline
};
