#include "esp_system.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern "C" {
#include "tftspi.h"
#include "tft.h"
}

#include <string.h>

extern const char *TAG;

bool HaveDisplay = false;
uint16_t DisplayHeight = 0;
uint16_t DisplayWidth = 0;

esp_err_t InitDisplay()
{
#if CONFIG_EXAMPLE_DISPLAY_TYPE != 0

    esp_err_t err;
    spi_lobo_device_handle_t spi;

    spi_lobo_bus_config_t buscfg;
    memset((void *)&buscfg, 0, sizeof(buscfg));
    buscfg.miso_io_num=PIN_NUM_MISO;              // set SPI MISO pin
    buscfg.mosi_io_num=PIN_NUM_MOSI;              // set SPI MOSI pin
    buscfg.sclk_io_num=PIN_NUM_CLK;               // set SPI CLK pin
    buscfg.quadwp_io_num=-1;
    buscfg.quadhd_io_num=-1;
    buscfg.max_transfer_sz = 6*1024;

    spi_lobo_device_interface_config_t devcfg;
    memset((void *)&devcfg, 0, sizeof(devcfg));
    devcfg.clock_speed_hz=8000000;                // Initial clock out at 8 MHz
    devcfg.mode=0;                                // SPI mode 0
    devcfg.spics_io_num=-1;                       // we will use external CS pin
    devcfg.spics_ext_io_num=PIN_NUM_CS;           // external CS pin
    devcfg.flags=LB_SPI_DEVICE_HALFDUPLEX;        // ALWAYS SET  to HALF DUPLEX MODE!! for display spi

    tft_disp_type = DISP_TYPE_ILI9341;
    max_rdclock = 8000000;

    // Initialize all pins used by display driver.
    TFT_PinsInit();

    // Initialize SPI bus and add a device for the display.
    err = spi_lobo_bus_add_device(TFT_HSPI_HOST, &buscfg, &devcfg, &spi);
    if (err != ESP_OK)
        return err;

    // Configure the display to use the new SPI device.
    disp_spi = spi;

    err = spi_lobo_device_select(spi, 1);
    if (err != ESP_OK)
        return err;
    err = spi_lobo_device_deselect(spi);
    if (err != ESP_OK)
        return err;

    // Initialize the display driver.
    TFT_display_init();

    // ---- Detect maximum read speed ----
    max_rdclock = find_rd_speed();

    // Set the SPI clock speed.
    spi_lobo_set_speed(spi, DEFAULT_SPI_CLOCK);

    TFT_setGammaCurve(0);
    TFT_setRotation(LANDSCAPE);
    TFT_setFont(DEJAVU24_FONT, NULL);
    TFT_resetclipwin();

    HaveDisplay = true;
    DisplayWidth = (uint16_t)(1 + dispWin.x2 - dispWin.x1);
    DisplayHeight = (uint16_t)(1 + dispWin.y2 - dispWin.y1);

    ESP_LOGE(TAG, "Display initialized (height %u, width %u)", DisplayHeight, DisplayWidth);

    return err;

#else // CONFIG_EXAMPLE_DISPLAY_TYPE

    return ESP_ERR_NOT_SUPPORTED;

#endif // CONFIG_EXAMPLE_DISPLAY_TYPE
}
