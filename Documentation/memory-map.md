Memory Map
==========

DRAM (0x3ffe8000 - 0x3fffbfff)
-----------------------------

```
0x3ffe8000 - 0x3fff3fff: firmware (48KB)
0x3fff4000 - 0x3fff6fff: firmware heap (12KB)
0x3fff7000 - 0x3fff8fff: OS kmalloc chunk (8KB)
    0x3fff7000 - 0x3fff73ff: .ELF header (1KB)
    0x3fff7400 - 0x3fff93ff: app image temporary buffer (4KB)
0x3fff9000 - 0x3fffbfff: OS (12KB)
```

IRAM (0x40100000 - 0x40107fff)
------------------------------

```
0x40100000 - 0x40105fff: firmware (24KB)
0x40106000 - 0x40107fff: OS (8KB)
```

SPI Flash
---------

```
0x40201010 - 0x4026b000: firmware (423KB)
0x4026b000 - 0x40286fff: OS + app (48KB)
0x40287000 - 0x4027b000: SPIFFS (16KB)
```

