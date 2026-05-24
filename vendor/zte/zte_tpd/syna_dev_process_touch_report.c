__int64 __fastcall syna_dev_process_touch_report(unsigned __int8 a1, const void *a2, __int64 a3, __int64 a4)
{
  unsigned int v5; // w22
  const void *v6; // x20
  size_t v7; // x21
  unsigned int v8; // w0
  __int64 v9; // x2
  __int64 v10; // x20
  __int64 v11; // x25
  __int64 v12; // x1
  __int64 v13; // x2
  int v14; // w8
  __int64 v15; // x0
  __int64 v16; // x21
  int v17; // w27
  __int64 v18; // x23
  _DWORD *v19; // x22
  char v20; // w8
  int v21; // w9
  unsigned int v22; // w25
  signed int v23; // w28
  signed int v24; // w24
  unsigned int v25; // w26
  int v26; // w8
  unsigned int v27; // w4
  __int64 v28; // x3
  unsigned int v29; // w25
  __int64 v30; // x1
  __int64 v31; // x2
  __int64 result; // x0
  unsigned int v33; // w19
  __int64 v34; // x2
  __int64 v35; // [xsp+8h] [xbp-28h]
  __int64 v36; // [xsp+10h] [xbp-20h]
  _QWORD v37[3]; // [xsp+18h] [xbp-18h] BYREF

  v37[2] = *(_QWORD *)(_ReadStatusReg(SP_EL0) + 1808);
  if ( !a4 )
  {
    printk(unk_3411A, "syna_dev_process_touch_report", a3);
    result = 4294967274LL;
    goto LABEL_60;
  }
  if ( a1 != 17 )
  {
    printk(unk_3660C, "syna_dev_process_touch_report", a1);
    result = 4294967274LL;
    goto LABEL_60;
  }
  if ( *(int *)(a4 + 900) >= 1 && *(_BYTE *)(a4 + 1412) != 1 )
    goto LABEL_59;
  if ( *(_DWORD *)(a4 + 1504) )
  {
    v5 = a3;
    v6 = a2;
    if ( (unsigned int)a3 >= 0x32000 )
      v7 = 204800;
    else
      v7 = (unsigned int)a3;
    memcpy(*(void **)(a4 + 1120), a2, v7);
    *(_DWORD *)(a4 + 1080) = v7;
    *(_DWORD *)(a4 + 1128) = 1;
    _wake_up(a4 + 1096, 1, 1, 0);
    a2 = v6;
    a3 = v5;
  }
  v8 = syna_tcm_parse_touch_report(*(_QWORD *)a4, a2, a3, a4 + 16);
  if ( (v8 & 0x80000000) != 0 )
  {
    v33 = v8;
    printk(unk_3A914, "syna_dev_process_touch_report", v9);
    result = v33;
    goto LABEL_60;
  }
  v10 = *(_QWORD *)(a4 + 944);
  if ( !v10 )
    goto LABEL_59;
  v11 = *(unsigned int *)(*(_QWORD *)a4 + 24LL);
  v36 = tpd_cdev;
  mutex_lock(a4 + 632);
  if ( *(_DWORD *)(a4 + 1404) == 2 && *(_BYTE *)(a4 + 756) == 1 )
  {
    printk(unk_35FD5, "syna_dev_report_input_events", *(unsigned int *)(a4 + 544));
    v34 = *(unsigned int *)(a4 + 544);
    if ( (_DWORD)v34 == 1 )
    {
      printk(unk_35FFC, "syna_dev_report_input_events", v34);
      ufp_report_gesture_uevent("double_tap=true");
    }
    else if ( (_DWORD)v34 == 16 )
    {
      printk(unk_3354D, "syna_dev_report_input_events", v34);
      ufp_report_gesture_uevent("single_tap=true");
    }
    else
    {
      printk(unk_37625, "syna_dev_report_input_events", v34);
    }
  }
  v14 = *(_DWORD *)(a4 + 544);
  if ( v14 == 128 )
  {
    v15 = 1;
    goto LABEL_18;
  }
  if ( v14 == 129 )
  {
    v15 = 0;
LABEL_18:
    report_ufp_uevent(v15);
  }
  if ( *(_DWORD *)(a4 + 1404) == 2 )
    goto LABEL_58;
  if ( !(_DWORD)v11 )
    goto LABEL_56;
  v16 = 0;
  v17 = 0;
  v18 = a4 + 612;
  v19 = (_DWORD *)(a4 + 40);
  v35 = v11;
  do
  {
    if ( *((_BYTE *)v19 - 16) == 6 && *(_DWORD *)(a4 + 1508) )
    {
      if ( ufp_tp_ops.pdev )
      {
        v21 = large_area_ignore_count;
        if ( large_area_ignore_count < 0 )
        {
          if ( large_area_uevent_count <= 2 )
          {
            v37[0] = "large_area=true";
            v37[1] = 0;
            kobject_uevent_env(&ufp_tp_ops.pdev->dev.kobj, 2, v37);
            printk(unk_32F20, v30, v31);
            ++large_area_uevent_count;
          }
        }
        else
        {
          --large_area_ignore_count;
          printk(unk_3475B, (unsigned int)(v21 - 1), v13);
        }
      }
      else
      {
        printk(unk_33C0B, v12, v13);
      }
    }
    if ( v16 == 10 )
      __break(0x5512u);
    if ( *(_BYTE *)(v18 + v16) || *((_BYTE *)v19 - 16) )
    {
      if ( (unsigned int)*((unsigned __int8 *)v19 - 16) - 1 < 2 )
      {
        v22 = *(v19 - 3);
        if ( *(_DWORD *)(a4 + 1560) == 4 )
        {
          v23 = *(v19 - 1);
          v24 = *v19;
          v22 /= 0xAu;
          v25 = *(v19 - 2) / 0xAu;
          goto LABEL_45;
        }
        v24 = *v19;
        v25 = *(v19 - 2);
        v23 = *(v19 - 1);
        if ( *(_BYTE *)(v36 + 27) == 1 )
        {
          if ( v23 <= v24 )
            v27 = *v19;
          else
            v27 = *(v19 - 1);
          tpd_touch_press(v10, v22, v25, v16, v27, 0);
          ((void (__fastcall *)(__int64, _QWORD, _QWORD, _QWORD))one_key_report)(1, v22, v25, (unsigned int)v16);
        }
        else
        {
LABEL_45:
          input_event(v10, 3, 47, (unsigned int)v16);
          input_mt_report_slot_state(v10, 0, 1);
          input_event(v10, 1, 330, 1);
          input_event(v10, 1, 325, 1);
          input_event(v10, 3, 53, v22);
          input_event(v10, 3, 54, v25);
          if ( v23 <= v24 )
            v28 = (unsigned int)v24;
          else
            v28 = (unsigned int)v23;
          if ( v23 >= v24 )
            v29 = v24;
          else
            v29 = v23;
          input_event(v10, 3, 48, v28);
          input_event(v10, 3, 49, v29);
        }
        v11 = v35;
        ++v17;
        goto LABEL_23;
      }
      if ( !*((_BYTE *)v19 - 16) )
      {
        if ( *(_BYTE *)(v36 + 27) != 1 || (v26 = *(_DWORD *)(a4 + 1560), v26 == 4) )
        {
          input_event(v10, 3, 47, (unsigned int)v16);
          input_mt_report_slot_state(v10, 0, 0);
        }
        else
        {
          tpd_touch_release(v10, v16, v26);
          ((void (__fastcall *)(_QWORD, __int64, __int64, _QWORD))one_key_report)(
            0,
            0xFFFFFFFFLL,
            0xFFFFFFFFLL,
            (unsigned int)v16);
        }
      }
    }
LABEL_23:
    v20 = *((_BYTE *)v19 - 16);
    v19 += 13;
    *(_BYTE *)(v18 + v16++) = v20;
  }
  while ( v11 != v16 );
  if ( !v17 )
  {
LABEL_56:
    input_event(v10, 1, 330, 0);
    input_event(v10, 1, 325, 0);
  }
  input_mt_sync_frame(v10);
  input_event(v10, 0, 0, 0);
LABEL_58:
  mutex_unlock(a4 + 632);
LABEL_59:
  result = 0;
LABEL_60:
  _ReadStatusReg(SP_EL0);
  return result;
}
