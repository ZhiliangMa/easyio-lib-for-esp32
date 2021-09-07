set(led_srcs "src/led.c")

idf_component_register(SRCS "${led_srcs}"
                       INCLUDE_DIRS "include")

