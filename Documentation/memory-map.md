Memory Map
==========

DRAM (0x3ffe8000 - 3fffbfff)
-----------------------------

```
0x3ffe8000 - 0x3fff3fff: firmware (48KB)
0x3fff4000 - 0x3fff6fff: firmware heap (12KB)
0x3fff7000 - 0x3fff8fff: Base OS / load_elf in firmware (8KB)
0x3fff9000 - 0x3fffbfff: Base OS (12KB)
```

IRAM (0x40100000 - 0x40107fff)
------------------------------

```
0x40100000 - 0x40107fff: firmware (32KB)
```

SPI Flash
---------

```
0x40201010 - 0x4026b000: firmware (423KB)
0x4026b000 - 0x40286fff: DeviceOS + app (48KB)
0x40287000 - 0x4027B000: SPIFFS (16KB)
```

