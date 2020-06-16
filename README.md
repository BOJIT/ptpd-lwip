# ptpd-lwip
PTPd port for use with lwIP on embedded devices.

This port is based on PTPv2 release **v2rc1**, January 2010. In particular, the code is based on ST Microelectronics' [AN3411](http://www.bdtic.com/download/ST/AN3411.pdf) and the work of [mpthompson](https://github.com/mpthompson/stm32_f4_ptpd).

Since then, the ptpd project structure has changed significantly. The newer versions are much harder to port to a microcontroller, so this project can be considered as a fork from **v2rc1** rather than a direct port of the existing project.
