insmod ~/Driver/canvasLedDriver/canvasLedDriver.ko led_row_n=32 led_col_n=64
tail -f /var/log/messages > lastCrash.txt &
