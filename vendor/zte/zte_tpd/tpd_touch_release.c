__int64 tpd_touch_release(__int64 result, unsigned __int16 a2, int a3)
{
  unsigned __int16 *v3; // x19
  __int64 v4; // x27
  unsigned int v5; // w20
  __int64 v7; // x26
  __int64 v8; // x28
  __int64 v9; // x25
  __int64 v10; // x8
  int v11; // w8
  int v12; // w9
  int v13; // w9
  __int64 v14; // x10
  __int64 v15; // x22
  unsigned int v16; // w0
  __int64 v17; // x2
  int v18; // w8
  int v19; // w8
  int v20; // w8
  int v21; // w9
  int v22; // w9
  int v23; // w10
  int v24; // w8
  int v25; // w8
  __int64 v26; // x1
  __int64 v27; // x2
  __int64 v28; // x22
  __int64 v29; // x2
  void *v30; // x0
  __int64 v31; // x1
  __int64 v32; // x8
  unsigned __int64 v33; // x22
  unsigned __int16 v34; // w8
  __int64 v35; // x22
  unsigned int v36; // w25
  unsigned int v37; // w26
  unsigned int v38; // w23
  unsigned int v39; // w24
  __int64 v40; // x1
  __int64 v41; // x2

  v5 = a2;
  if ( a2 >= 0xBu )
  {
    __break(0x5512u);
LABEL_83:
    __break(1u);
    goto LABEL_84;
  }
  if ( !result )
    return printk(unk_3A160, "tpd_touch_release", a2);
  a3 = a2;
  if ( a2 == 10 )
    goto LABEL_83;
  v7 = a2;
  v8 = tpd_cdev;
  v9 = 136LL * a2;
  v3 = (unsigned __int16 *)((char *)&point_report_info + v9);
  if ( *((_BYTE *)&point_report_info + v9 + 83) == 1 )
  {
    v15 = result;
    mutex_lock(tpd_cdev + 2776);
    input_event(v15, 3, 47, v5);
    input_mt_report_slot_state(v15, 0, 0);
    v16 = jiffies_to_msecs(jiffies - *((_QWORD *)v3 + 12));
    v17 = *((unsigned int *)v3 + 9);
    *((_QWORD *)v3 + 15) = v16;
    printk(unk_3BD1F, v5, v17);
    if ( is_fake_sleep_mode && (unsigned int)jiffies_to_msecs(jiffies - *((_QWORD *)v3 + 12)) <= 0x95 )
    {
      v18 = *((_DWORD *)v3 + 6) - *((_DWORD *)v3 + 9);
      if ( v18 < 0 )
        v18 = *((_DWORD *)v3 + 9) - *((_DWORD *)v3 + 6);
      if ( v18 <= 49 )
      {
        v19 = *((_DWORD *)v3 + 7) - *((_DWORD *)v3 + 10);
        if ( v19 < 0 )
          v19 = *((_DWORD *)v3 + 10) - *((_DWORD *)v3 + 7);
        if ( v19 <= 49 )
          ufp_report_gesture_uevent("single_tap=true");
      }
    }
    if ( __PAIR64__(is_screen_off_awake_mode, is_fake_sleep_mode)
      && (unsigned int)jiffies_to_msecs(jiffies - *((_QWORD *)v3 + 9)) <= 0x12B
      && (unsigned int)jiffies_to_msecs(jiffies - *((_QWORD *)v3 + 12)) <= 0x95 )
    {
      v20 = *((_DWORD *)v3 + 6);
      v21 = v20 - *((_DWORD *)v3 + 9);
      if ( v21 < 0 )
        v21 = *((_DWORD *)v3 + 9) - v20;
      if ( v21 <= 49 )
      {
        v22 = *((_DWORD *)v3 + 7);
        v23 = v22 - *((_DWORD *)v3 + 10);
        if ( v23 < 0 )
          v23 = *((_DWORD *)v3 + 10) - v22;
        if ( v23 <= 49 )
        {
          v24 = v20 - *((_DWORD *)v3 + 15);
          if ( v24 < 0 )
            v24 = -v24;
          if ( v24 <= 999 )
          {
            v25 = v22 - *((_DWORD *)v3 + 16);
            if ( v25 < 0 )
              v25 = *((_DWORD *)v3 + 16) - v22;
            if ( v25 <= 999 )
              ufp_report_gesture_uevent("double_tap=true");
          }
        }
      }
    }
    result = mutex_unlock(v8 + 2776);
    if ( *((_QWORD *)v3 + 15) < (unsigned __int64)*(unsigned __int8 *)(v8 + 1173) && (v3[45] & 1) == 0 )
    {
      if ( (*(_BYTE *)(v8 + 1160) & 1) == 0 )
      {
        printk(unk_3D033, v26, v27);
        v31 = *(_QWORD *)(v8 + 1200);
        *(_BYTE *)(v8 + 1160) = 1;
        queue_delayed_work_on(32, v31, v8 + 2360, 500);
      }
      v32 = 1169;
      if ( *(int *)(v8 + 1164) > 2 )
        v32 = 1170;
      v33 = *(unsigned __int8 *)(v8 + v32);
      if ( (ghost_check_area((char *)&point_report_info + v9) & 1) != 0 )
      {
        v34 = v3[47] + 1;
        if ( *((_QWORD *)v3 + 15) < v33 )
          ++v3[46];
        v3[47] = v34;
      }
      result = printk(unk_3A7B9, v5, v3[46]);
    }
    --*(_DWORD *)(v8 + 1164);
  }
  if ( *(_BYTE *)(v8 + 37) != 1 )
    goto LABEL_9;
  if ( (v3[43] & 1) != 0 )
    goto LABEL_9;
  v10 = *((_QWORD *)v3 + 14);
  if ( !v10 )
    goto LABEL_9;
  result = jiffies_to_msecs(jiffies - v10);
  if ( (unsigned int)result >= *(unsigned __int16 *)(v8 + 60) )
    goto LABEL_9;
  v28 = tpd_cdev;
  printk(unk_38119, "edge_point_report", v5);
  if ( !*(_QWORD *)(v28 + 1208) )
  {
    v30 = unk_3CAE7;
LABEL_54:
    result = printk(v30, "edge_point_report", v29);
    goto LABEL_9;
  }
  if ( (byte_30EFB & 1) != 0
    || (byte_30F83 & 1) != 0
    || (byte_3100B & 1) != 0
    || (byte_31093 & 1) != 0
    || (byte_3111B & 1) != 0
    || (byte_311A3 & 1) != 0
    || (byte_3122B & 1) != 0
    || (byte_312B3 & 1) != 0
    || (byte_3133B & 1) != 0
    || byte_313C3 == 1 )
  {
    v30 = unk_3B713;
    goto LABEL_54;
  }
  v4 = v28;
  mutex_lock(v28 + 2776);
  a3 = (136 * v7) | 4;
  if ( ((136 * v7) | 4uLL) > 0x550 )
    goto LABEL_83;
  a3 = v9 | 4;
  if ( (v9 | 4uLL) > 0x550 )
    goto LABEL_83;
  v35 = *((_QWORD *)v3 + 16);
  v36 = *v3;
  v37 = v3[2];
  v38 = *((unsigned __int8 *)v3 + 8);
  v39 = *((unsigned __int8 *)v3 + 9);
  input_event(v35, 3, 47, v5);
  input_mt_report_slot_state(v35, 0, 1);
  input_event(v35, 1, 330, 1);
  input_event(v35, 3, 53, v36);
  input_event(v35, 3, 54, v37);
  if ( v39 )
    input_event(v35, 3, 58, v39);
  if ( v38 )
    input_event(v35, 3, 48, v38);
  input_mt_sync_frame(v35);
  input_event(*((_QWORD *)v3 + 16), 0, 0, 0);
  *((_BYTE *)v3 + 84) = 1;
  printk(unk_35994, "edge_point_report", v5);
  mutex_unlock(v4 + 2776);
  a3 = a2;
  if ( a2 <= 4u )
  {
    if ( a2 > 1u )
    {
      switch ( a2 )
      {
        case 2u:
          v40 = *(_QWORD *)(v4 + 1208);
          v41 = v4 + 1424;
          goto LABEL_96;
        case 3u:
          v40 = *(_QWORD *)(v4 + 1208);
          v41 = v4 + 1528;
          goto LABEL_96;
        case 4u:
          v40 = *(_QWORD *)(v4 + 1208);
          v41 = v4 + 1632;
LABEL_96:
          result = queue_delayed_work_on(32, v40, v41, 13);
          goto LABEL_9;
      }
      goto LABEL_97;
    }
LABEL_84:
    if ( !a3 )
    {
      v40 = *(_QWORD *)(v4 + 1208);
      v41 = v4 + 1216;
      goto LABEL_96;
    }
    if ( a3 == 1 )
    {
      v40 = *(_QWORD *)(v4 + 1208);
      v41 = v4 + 1320;
      goto LABEL_96;
    }
    goto LABEL_97;
  }
  if ( a2 <= 6u )
  {
    if ( a2 == 5 )
    {
      v40 = *(_QWORD *)(v4 + 1208);
      v41 = v4 + 1736;
      goto LABEL_96;
    }
    if ( a2 == 6 )
    {
      v40 = *(_QWORD *)(v4 + 1208);
      v41 = v4 + 1840;
      goto LABEL_96;
    }
  }
  else
  {
    switch ( a2 )
    {
      case 7u:
        v40 = *(_QWORD *)(v4 + 1208);
        v41 = v4 + 1944;
        goto LABEL_96;
      case 8u:
        v40 = *(_QWORD *)(v4 + 1208);
        v41 = v4 + 2048;
        goto LABEL_96;
      case 9u:
        v40 = *(_QWORD *)(v4 + 1208);
        v41 = v4 + 2152;
        goto LABEL_96;
    }
  }
LABEL_97:
  result = printk(unk_340BD, "edge_point_report", v5);
LABEL_9:
  v11 = is_screen_off_awake_mode;
  v12 = is_fake_sleep_mode;
  *((_QWORD *)v3 + 14) = 0;
  *((_BYTE *)v3 + 83) = 0;
  v3[40] = 0;
  *(_DWORD *)((char *)v3 + 85) = 0;
  *(unsigned __int16 *)((char *)v3 + 89) = 0;
  if ( v12 | v11 )
  {
    v13 = *((_DWORD *)v3 + 7);
    v14 = *((_QWORD *)v3 + 12);
    *((_DWORD *)v3 + 15) = *((_DWORD *)v3 + 6);
    *((_DWORD *)v3 + 16) = v13;
    *((_QWORD *)v3 + 9) = v14;
  }
  return result;
}
