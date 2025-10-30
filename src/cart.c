#include "../includes/cart.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Cart_Header
{
    uint8_t logo[48];
    char title[16];
    char manufactor_code[4];
    uint8_t cgb_flag;
    uint8_t licensee_code;
    uint8_t sgb_flag;
    uint8_t cartridge_type;
    uint8_t rom_size;
    uint8_t ram_size;
    uint8_t destination_code;
    uint8_t checksum;
    uint16_t global_checksum;
} s_Cart_Header;

static uint8_t *ext_ram = NULL;
static uint32_t ext_ram_size;

static uint8_t *memory = NULL;

static s_Cart_Header sch;

uint8_t calculate_checksum()
{
    uint8_t checksum = 0;
    for (uint16_t address = 0x0134; address <= 0x014C; address++) {
        checksum = checksum - memory[address] - 1;
    }
    return checksum;
}

void load_header() {
    // Copy Nintendo Logo (0x0104-0x0133)
    for (int i = 0; i < 48; i++) {
        sch.logo[i] = memory[0x0104 + i];
    }

    // Load Title (0x0134-0x0143)
    memset(sch.title, 0, sizeof(sch.title));
    for (int i = 0; i < 16; i++) {
        sch.title[i] = memory[0x0134 + i];
    }

    // Load Manufacturer Code (0x013F-0x0142)
    memset(sch.manufactor_code, 0, sizeof(sch.manufactor_code));
    for (int i = 0; i < 4; i++) {
        sch.manufactor_code[i] = memory[0x013F + i];
    }

    // CGB Flag
    sch.cgb_flag = memory[0x0143];

    // New Licensee Code
    if (memory[0x014B] == 0x33) {
        sch.licensee_code = (memory[0x0144] << 8) | memory[0x0145];
    } else {
        sch.licensee_code = memory[0x014B];
    }

    // SGB Flag
    sch.sgb_flag = memory[0x0146];

    // Cartridge Type
    sch.cartridge_type = memory[0x0147];

    // ROM Size
    sch.rom_size = memory[0x0148];

    // RAM Size
    sch.ram_size = memory[0x0149];
    if (sch.ram_size == 0x02)
    {
        ext_ram_size =  8*1024;   
        ext_ram = malloc(8*1024);
    }
    else if (sch.ram_size == 0x03)
    {
        ext_ram_size =  32*1024;  
        ext_ram = malloc(32*1024);
    }
    else if (sch.ram_size == 0x04)
    {
        ext_ram_size =  128*1024;  
        ext_ram = malloc(128*1024);
    }
    else if (sch.ram_size == 0x05)
    {
        ext_ram_size =  32*1024;  
        ext_ram = malloc(64*1024);
    }
    
    // Destination Code
    sch.destination_code = memory[0x014A];

    // ROM Checksum
    sch.checksum = memory[0x014D];

    // Global Checksum
    sch.global_checksum = (memory[0x014E] << 8) | memory[0x014F];
}

int load_rom(const char *path)
{
    FILE *rom = fopen(path, "rb");
    if(!rom)
    {
        perror("Error fopen:");
        return -1;
    }

    fseek(rom, 0, SEEK_END);
    uint32_t size = ftell(rom);
    rewind(rom);

    memory = malloc(size);

    fread(memory, size, 1, rom);
    fclose(rom);

    printf("%02X", memory[0x104]);

    load_header();
    sch.title[15] = '\0'; 

    printf("Cartridge Loaded:\n");
    printf("Title    : %s\n", sch.title);
    printf("Type     : %02X \n", sch.cartridge_type);
    printf("ROM Size : %d KB\n", 32 << size);
    printf("RAM Size : %02X\n", sch.ram_size);
    printf("LIC Code : %02X\n", sch.licensee_code);
    printf("ROM Checksum : %s (%02X)\n", (calculate_checksum() == sch.checksum) ? "OK" : "NOT VALID", sch.checksum);

    // mbc_index = 0x4000;

    if (sch.cartridge_type == 0x03) // For load external ram (ROM+BATTERY)
    {

    }
    return 0;
}

uint8_t rom_read(uint16_t addr)
{
    if (addr < 0x4000) 
    {
        return memory[addr];
    }
    else if ((addr >= 0x4000) && (addr < 0x8000)) 
    {
        return memory[addr - 0x4000 + mbc_index];
    }
    
    return 0xFF;
}

void rom_write(uint16_t addr, uint8_t value)
{
    if (sch.cartridge_type == 0) // ROM ONLY
        return;
    else if (sch.cartridge_type >= 0x19 && sch.cartridge_type <= 0x1E) // MBC5
        mbc_5(addr, value, ext_ram_size);
    else if (sch.cartridge_type >= 0x01 && sch.cartridge_type <= 0x03) // MBC1
        mbc_1(addr, value, ext_ram_size);
}

uint8_t external_ram_read(uint16_t addr)
{
    if (ext_ram_active && ext_ram) 
    {
        return ext_ram[(addr - 0xA000) + ext_ram_index];
    }
    return 0xFF;
}

void external_ram_write(uint16_t addr, uint8_t value)
{
    if (ext_ram_active) 
    {
        uint32_t ram_addr = (addr - 0xA000) + ext_ram_index;
        if (ram_addr < ext_ram_size) 
        {
            ext_ram[ram_addr] = value;
        }
    }
}