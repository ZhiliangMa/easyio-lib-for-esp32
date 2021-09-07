set(easyio_srcs
    "src/led.c"
    "src/gpioX.c")

idf_component_register(SRCS "${easyio_srcs}"
                       INCLUDE_DIRS "include")
