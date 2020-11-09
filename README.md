# ptpd-lwip
PTPd port for use with lwIP on embedded devices.

This port is based on PTPv2 release **v2rc1**, January 2010. In particular, the code is based on ST Microelectronics' [AN3411](http://www.bdtic.com/download/ST/AN3411.pdf) and the work of [mpthompson](https://github.com/mpthompson/stm32_f4_ptpd).

Since then, the ptpd project structure has changed significantly. The newer versions are much harder to port to a microcontroller, so this project can be considered as a fork from **v2rc1** rather than a direct port of the existing project.

Note that the port interface layer is designed to be rollover-safe, but PTPd itself is not yet... ...something to worry about in 2038!

Prerequisites:

- lwIP is installed and has an architecture port that supports RTOS features such as tasks, mailboxes, queues, etc.

- When PTP is used, the ```lwipopts.h``` file should contain the following entry:

```c
#if LWIP_PTP
    #define LWIP_PBUF_CUSTOM_DATA \
                    u32_t tv_sec; \
                    u32_t tv_nsec;
#endif /* LWIP_PTP */
```

  This adds timestamp fields to the basic lwIP pbuf structure.

# TODO
- relocate startup functions
- write and test network functions with netconn API + NO FREERTOS DEPENDENCIES
- check the validity of the lwip timers + check for memory allocation issues here!

- management message handling doesn't appear to be implemented in AN3411. Look into this.

- If hardware timestamping is used, ptpClock->Outbound/Inbound Latency = 0, but events must wait for a callback
