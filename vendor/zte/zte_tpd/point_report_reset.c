#include "defs.h"

extern __int64 tpd_touch_report(__int64 result, __int64 a2, __int64 a3, __int64 a4, __int64 a5, __int64 a6);

__int64 __fastcall point_report_reset(__int64 result, __int64 a2, __int64 a3, __int64 a4, __int64 a5, __int64 a6)
{
  char *v6; // x20
  unsigned int v7; // w19
  __int64 v8; // x21

  if ( (unsigned int)result > 9 )
  {
    __break(1u);
    return tpd_touch_report(result, a2, a3, a4, a5, a6);
  }
  else
  {
    v6 = (char *)&point_report_info + 136 * (unsigned int)result;
    if ( v6[84] == 1 )
    {
      v7 = result;
      v8 = tpd_cdev;
      printk(unk_36A65, "point_report_reset", (unsigned int)result);
      v6[84] = 0;
      mutex_lock(v8 + 2776);
      input_event(*((_QWORD *)v6 + 16), 3, 47, v7);
      input_mt_report_slot_state(*((_QWORD *)v6 + 16), 0, 0);
      input_mt_sync_frame(*((_QWORD *)v6 + 16));
      input_event(*((_QWORD *)v6 + 16), 0, 0, 0);
      mutex_unlock(v8 + 2776);
      usleep_range_state(1000, 1100, 2);
      return 0;
    }
  }
  return result;
}
