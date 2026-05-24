__int64 __fastcall syna_dev_free_input_events(__int64 result)
{
  __int64 v1; // x20
  __int64 v2; // x19
  unsigned int i; // w21
  int v4; // w8
  __int64 v5; // x2

  v1 = *(_QWORD *)(result + 944);
  if ( v1 )
  {
    v2 = result;
    mutex_lock(result + 632);
    for ( i = 0; i != 10; ++i )
    {
      input_event(v1, 3, 47, i);
      input_mt_report_slot_state(v1, 0, 0);
      tpd_touch_release(v1, i, v4);
      ((void (__fastcall *)(_QWORD, __int64, __int64, _QWORD))one_key_report)(0, 0xFFFFFFFFLL, 0xFFFFFFFFLL, i);
    }
    input_event(v1, 1, 330, 0);
    input_event(v1, 1, 325, 0);
    if ( HIBYTE(word_314C0) | (unsigned __int8)word_314C0 )
    {
      byte_314C2 = 1;
      printk(unk_3A18D, "syna_dev_free_input_events", v5);
    }
    report_ufp_uevent(0);
    input_mt_sync_frame(v1);
    input_event(v1, 0, 0, 0);
    return mutex_unlock(v2 + 632);
  }
  return result;
}
