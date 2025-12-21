#pragma once

#ifdef __cplusplus
extern "C"
{
#endif
extern void signal_setup();
extern void signal_timer_disable();
extern void signal_timer_enable(int timeout_seconds);

#ifdef __cplusplus
};

#endif
