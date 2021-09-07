set(easyio_srcs
    "src/led.c"
    "src/gpioX.c"
    "src/key.c"
    "src/touch_pad_button.c"
    "src/ledc_pwm.c"
    "src/adc_sampling.c"
    "${IDF_PATH}/components/esp_adc_cal/esp_adc_cal_esp32.c"
    "src/dac_output.c"
    "src/mcpwm_motor.c"
    "src/mcpwm_half_bridge.c"
    "src/mcpwm_servo.c"
    "src/mcpwm_capture.c"
    "src/pulse_cnt.c")

idf_component_register(SRCS "${easyio_srcs}"
                       INCLUDE_DIRS "include"
                       "${IDF_PATH}/components/esp_adc_cal/include")
