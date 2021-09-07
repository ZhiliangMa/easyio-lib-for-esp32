set(easyio_srcs
    "src/led.c"
    "src/gpioX.c"
    "src/key.c"
    "src/touch_pad_button.c")

idf_component_register(SRCS "${easyio_srcs}"
                       INCLUDE_DIRS "include")
