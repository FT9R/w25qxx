#include "w25qxx_Demo.h"

#define DEMO_TARGET_PAGE 31

/* Private variables */
static w25qxx_HandleTypeDef w25qxx_Handle;
static const uint8_t bufferWrite[] = "Hello World!";
static uint8_t bufferRead[sizeof(bufferWrite)] = {0};
static uint8_t erasedTemplate[sizeof(bufferWrite)];

static struct DemoFlags_s {
    uint8_t success : 1;
    uint8_t error : 1;
} demoFlags;

/**
 * @brief Error handler for the W25QXX demo
 * @param fpPrint Function pointer for printing messages
 */
static void w25qxx_DemoErrorHandler(void (*fpPrint)(const char *message));

uint8_t w25qxx_Demo(void (*fpPrint)(const char *message), bool forceChipErase)
{
    /* Check the flags */
    if (demoFlags.success)
        return 0;
    if (demoFlags.error)
        return 1;

    fpPrint("\nInterface link\n");
    w25qxx_Link(&w25qxx_Handle, &hspi1, w25qxx_SPI_Receive, w25qxx_SPI_Transmit, w25qxx_SPI1_CS0_Set);

    fpPrint("Device initialization\n");
    w25qxx_Init(&w25qxx_Handle);

    fpPrint("Manufacturer: ");
    switch (w25qxx_Handle.ID[0])
    {
    case W25QXX_MANUFACTURER_ID:
        fpPrint("Winbond\n");
        break;

    default:
        fpPrint("undefined\n");
        w25qxx_DemoErrorHandler(fpPrint);

        return 1;
        break;
    }

    fpPrint("Device: ");
    switch (w25qxx_Handle.ID[1])
    {
    case W25Q80:
        fpPrint("W25Q80");
        break;

    case W25Q16:
        fpPrint("W25Q16");
        break;

    case W25Q32:
        fpPrint("W25Q32");
        break;

    case W25Q64:
        fpPrint("W25Q64");
        break;

    case W25Q128:
        fpPrint("W25Q128");
        break;

    default:
        fpPrint("undefined\n");
        w25qxx_DemoErrorHandler(fpPrint);

        return 1;
        break;
    }

    /* Memory array capacity */
    char capacityString[30];
    snprintf(capacityString, sizeof(capacityString), " (%uMbit in %u pages)\n",
             (w25qxx_Handle.numberOfPages * W25QXX_PAGE_SIZE * 8 / 1024 / 1024), w25qxx_Handle.numberOfPages);
    fpPrint(capacityString);

    fpPrint("Forcing status registers to its default state\n");
    w25qxx_Handle.statusRegister = 0x00;
    w25qxx_WriteStatus(&w25qxx_Handle, 1u, W25QXX_SR_VOLATILE);
    w25qxx_WriteStatus(&w25qxx_Handle, 2u, W25QXX_SR_VOLATILE);
    w25qxx_WriteStatus(&w25qxx_Handle, 3u, W25QXX_SR_VOLATILE);

    if (forceChipErase)
    {
        fpPrint("Chip erase...\n");
        w25qxx_Erase(&w25qxx_Handle, W25QXX_CHIP_ERASE, 0, W25QXX_WAIT_BUSY);
    }

    fpPrint("First approach to read\n");
    w25qxx_Read(&w25qxx_Handle, bufferRead, sizeof(bufferRead), W25QXX_PAGE_TO_ADDRESS(DEMO_TARGET_PAGE), W25QXX_CRC,
                W25QXX_FASTREAD_NO);
    switch (w25qxx_Handle.error)
    {
    case W25QXX_ERROR_NONE:
        if (memcmp(bufferRead, bufferWrite, sizeof(bufferRead)) == 0)
        {
            fpPrint("Data already exists at target page boundaries\n");
            demoFlags.success = 1u;

            return 0;
        }
        break;

    case W25QXX_ERROR_CHECKSUM:
        memset(erasedTemplate, 0xff, sizeof(erasedTemplate));
        if (memcmp(w25qxx_Handle.frameBuf, erasedTemplate, sizeof(bufferRead)) == 0)
            fpPrint("Target page is probably erased\n");
        else
            fpPrint("Target page contains corrupted data\n");

        fpPrint("Checksum error reset\n");
        w25qxx_ResetError(&w25qxx_Handle);

        fpPrint("Sector erase...\n");
        w25qxx_Erase(&w25qxx_Handle, W25QXX_SECTOR_ERASE_4KB,
                     W25QXX_SECTOR_TO_ADDRESS(W25QXX_PAGE_TO_SECTOR(DEMO_TARGET_PAGE)),
                     W25QXX_WAIT_BUSY); // Minimal erase operation

        fpPrint("Target page programming\n");
        w25qxx_Write(&w25qxx_Handle, bufferWrite, sizeof(bufferWrite), W25QXX_PAGE_TO_ADDRESS(DEMO_TARGET_PAGE),
                     W25QXX_CRC, W25QXX_WAIT_BUSY);

        fpPrint("Second approach to read\n");
        w25qxx_Read(&w25qxx_Handle, bufferRead, sizeof(bufferRead), W25QXX_PAGE_TO_ADDRESS(DEMO_TARGET_PAGE),
                    W25QXX_CRC, W25QXX_FASTREAD_NO);
        if (memcmp(bufferRead, bufferWrite, sizeof(bufferRead)) == 0)
        {
            fpPrint("Writing process success\n");
            demoFlags.success = 1u;

            return 0;
        }
        else
        {
            fpPrint("Writing process failure\n");
            w25qxx_DemoErrorHandler(fpPrint);

            return 1;
        }
        break;

    default:
        w25qxx_DemoErrorHandler(fpPrint);

        return 1;
        break;
    }

    return 1;
}

/**
 * @section Private Functions
 */
static void w25qxx_DemoErrorHandler(void (*fpPrint)(const char *message))
{
    demoFlags.error = 1u;

    fpPrint("An error occured: ");
    switch (w25qxx_Handle.error)
    {
    case W25QXX_ERROR_NONE:
        break;

    case W25QXX_ERROR_STATUS:
        fpPrint("status match\n");
        break;

    case W25QXX_ERROR_INITIALIZATION:
        fpPrint("initialization\n");
        break;

    case W25QXX_ERROR_ARGUMENT:
        fpPrint("argument\n");
        break;

    case W25QXX_ERROR_ADDRESS:
        fpPrint("address\n");
        break;

    case W25QXX_ERROR_SPI:
        fpPrint("spi\n");
        break;

    case W25QXX_ERROR_TIMEOUT:
        fpPrint("timeout\n");
        break;

    case W25QXX_ERROR_CHECKSUM:
        fpPrint("checksum\n");
        break;

    case W25QXX_ERROR_INSTRUCTION:
        fpPrint("instruction\n");
        break;
    }

    fpPrint("Last detected status: ");
    switch (w25qxx_Handle.status)
    {
    case W25QXX_STATUS_NOLINK:
        fpPrint("no link\n");
        break;

    case W25QXX_STATUS_LINK:
        fpPrint("link\n");
        break;

    case W25QXX_STATUS_INIT:
        fpPrint("busy init\n");
        break;

    case W25QXX_STATUS_WRITE:
        fpPrint("busy write\n");
        break;

    case W25QXX_STATUS_READ:
        fpPrint("busy read\n");
        break;

    case W25QXX_STATUS_ERASE:
        fpPrint("busy erase\n");
        break;

    case W25QXX_STATUS_WRITE_SR:
        fpPrint("busy write status register\n");
        break;

    case W25QXX_STATUS_READ_SR:
        fpPrint("busy read status register\n");
        break;

    case W25QXX_STATUS_RESET:
        fpPrint("reset\n");
        break;

    case W25QXX_STATUS_BUSY:
        fpPrint("busy\n");
        break;

    case W25QXX_STATUS_READY:
        fpPrint("ready\n");
        break;

    case W25QXX_STATUS_UNDEFINED:
        fpPrint("undefined\n");
        break;
    }
}