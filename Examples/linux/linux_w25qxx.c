#include "w25qxx_Demo.h"

static int GPIO_Init(void);
static int SPI1_Init(void);

int main()
{
    if (GPIO_Init())
        return 1;

    if (SPI1_Init())
        return 1;

    if (w25qxx_Demo(w25qxx_Print, false))
    {
        printf("w25qxx data write/read sequence - no match\n");

        return 1;
    }

    return 0;
}

static int GPIO_Init(void)
{
    printf("GPIO init...\n");

    if (wiringPiSetup())
    {
        printf("GPIO init failed\n");

        return 1;
    }

    pinMode(SPI1_CS0_PIN, OUTPUT);

    return 0;
}

static int SPI1_Init(void)
{
    printf("SPI init...\n");
    int fd = wiringPiSPISetup(SPI_DEV, SPI_SPEED);

    if (fd < 0)
    {
        printf("SPI init failed\n");

        return 1;
    }

    return 0;
}
