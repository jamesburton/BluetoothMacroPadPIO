#pragma once

#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <lgfx/v1/platforms/esp32s3/Bus_RGB.hpp>
#include <lgfx/v1/platforms/esp32s3/Panel_RGB.hpp>
#include "DisplayConfig.hpp"

class LGFX : public lgfx::LGFX_Device
{
    lgfx::Panel_ST7701 _panel_instance;
    lgfx::Bus_RGB      _bus_instance;
    lgfx::Light_PWM    _light_instance;
    lgfx::Touch_GT911  _touch_instance;

public:
    LGFX(void)
    {
        {
            auto cfg = _bus_instance.config();
            cfg.panel = &_panel_instance;

            // RGB Interface Pins
            cfg.pin_d0  = PIN_D0;
            cfg.pin_d1  = PIN_D1;
            cfg.pin_d2  = PIN_D2;
            cfg.pin_d3  = PIN_D3;
            cfg.pin_d4  = PIN_D4;

            cfg.pin_d5  = PIN_D5;
            cfg.pin_d6  = PIN_D6;
            cfg.pin_d7  = PIN_D7;
            cfg.pin_d8  = PIN_D8;
            cfg.pin_d9  = PIN_D9;
            cfg.pin_d10 = PIN_D10;

            cfg.pin_d11 = PIN_D11;
            cfg.pin_d12 = PIN_D12;
            cfg.pin_d13 = PIN_D13;
            cfg.pin_d14 = PIN_D14;
            cfg.pin_d15 = PIN_D15;

            // Sync / Control
            cfg.pin_henable = PIN_DE;
            cfg.pin_vsync   = PIN_VSYNC;
            cfg.pin_hsync   = PIN_HSYNC;
            cfg.pin_pclk    = PIN_PCLK;

            cfg.freq_write  = WRITE_FREQ_HZ;

            cfg.hsync_polarity = HSYNC_POLARITY;
            cfg.hsync_front_porch = HSYNC_FRONT_PORCH;
            cfg.hsync_pulse_width = HSYNC_PULSE_WIDTH;
            cfg.hsync_back_porch  = HSYNC_BACK_PORCH;

            cfg.vsync_polarity = VSYNC_POLARITY;
            cfg.vsync_front_porch = VSYNC_FRONT_PORCH;
            cfg.vsync_pulse_width = VSYNC_PULSE_WIDTH;
            cfg.vsync_back_porch  = VSYNC_BACK_PORCH;

            cfg.pclk_active_neg   = PCLK_ACTIVE_NEG;
            cfg.de_idle_high      = 0;
            cfg.pclk_idle_high    = 0;

            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }

        {
            auto cfg = _panel_instance.config();
            cfg.memory_width  = 480;
            cfg.memory_height = 480;
            cfg.panel_width   = 480;
            cfg.panel_height  = 480;
            cfg.offset_x      = 0;
            cfg.offset_y      = 0;

            // BGR Color Order
            cfg.rgb_order = true;

            // SPI Init handled manually in main.cpp, no SPI pins config here.

            _panel_instance.config(cfg);
        }

        {
            auto cfg = _touch_instance.config();
            cfg.x_min      = 0;
            cfg.x_max      = 479;
            cfg.y_min      = 0;
            cfg.y_max      = 479;
            cfg.pin_int    = PIN_TOUCH_INT;
            cfg.pin_rst    = PIN_TOUCH_RST;

            cfg.bus_shared = false;
            cfg.pin_sda    = PIN_TOUCH_SDA;
            cfg.pin_scl    = PIN_TOUCH_SCL;

            cfg.i2c_port   = TOUCH_I2C_PORT;
            cfg.i2c_addr   = TOUCH_I2C_ADDR;
            cfg.freq       = TOUCH_I2C_FREQ;

            _touch_instance.config(cfg);
            _panel_instance.setTouch(&_touch_instance);
        }

        {
            auto cfg = _light_instance.config();
            cfg.pin_bl = PIN_BL;
            cfg.invert = false;
            cfg.freq   = 12000;
            cfg.pwm_channel = 7;
            _light_instance.config(cfg);
            _panel_instance.setLight(&_light_instance);
        }

        setPanel(&_panel_instance);
    }

    void setTouchI2CAddress(uint8_t addr)
    {
        auto cfg = _touch_instance.config();
        cfg.i2c_addr = addr;
        _touch_instance.config(cfg);
    }

    void setTouchI2CConfig(uint8_t addr, int8_t port)
    {
        auto cfg = _touch_instance.config();
        cfg.i2c_addr = addr;
        cfg.i2c_port = port;
        _touch_instance.config(cfg);
    }
};
