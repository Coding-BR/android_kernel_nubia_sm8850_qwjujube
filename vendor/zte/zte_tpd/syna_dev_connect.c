__int64 __fastcall syna_dev_connect(__int64 *a1, __int64 a2, __int64 a3)
{
  __int64 v3; // x20
  __int64 v5; // x21
  __int64 (__fastcall *v6)(__int64, __int64); // x8
  __int64 v7; // x0
  void *v9; // x0
  __int64 v10; // x0
  void (__fastcall *v11)(_QWORD); // x8
  unsigned int v12; // w0
  __int64 v13; // x2
  void *v14; // x8
  unsigned int v15; // w19
  __int64 v16; // x2
  __int64 v17; // x2
  __int64 v18; // x2
  __int64 v19; // x0
  __int64 v20; // x2
  __int64 v21; // x21
  __int64 v22; // x0
  __int64 v23; // x2
  unsigned int v24; // w20
  void *v25; // x0
  unsigned int v26; // w8
  __int64 v27; // x20
  __int64 v28; // x0
  unsigned int v29; // w0
  __int64 v30; // x4
  unsigned int v31; // w0
  __int64 v32; // x2
  __int64 v33; // x8
  const char *v34; // x2

  v3 = *a1;
  if ( !*a1 )
  {
    printk(unk_32430, "syna_dev_connect", a3);
    return 4294967274LL;
  }
  if ( *((_BYTE *)a1 + 1410) == 1 )
  {
    v9 = unk_3A284;
    goto LABEL_11;
  }
  v5 = a1[78];
  v6 = *(__int64 (__fastcall **)(__int64, __int64))(v5 + 384);
  if ( v6 )
  {
    v7 = a1[78];
    /* CFI check removed */
    if ( (v6(v7, 1) & 0x80000000) != 0 )
      return 4294967277LL;
    v10 = *(unsigned int *)(v5 + 340);
    if ( (int)v10 >= 1 )
      msleep(v10);
  }
  v11 = *(void (__fastcall **)(_QWORD))(v5 + 392);
  if ( v11 )
  {
    /* CFI check removed */
    v11(v5);
  }
  v12 = syna_tcm_detect_device(*a1, 1, 0);
  if ( (v12 & 0x80000000) != 0 )
  {
    v14 = unk_340F1;
LABEL_20:
    v15 = v12;
    printk(v14, "syna_dev_connect", v13);
    return v15;
  }
  v16 = *(unsigned __int8 *)(v3 + 9);
  if ( (_DWORD)v16 == 1 )
  {
    if ( (syna_dev_set_up_app_fw(a1) & 0x80000000) != 0 )
    {
      printk(unk_392F3, "syna_dev_connect", v17);
      printk(unk_3CB1F, "syna_dev_connect", v18);
      syna_tcm_switch_fw_mode(v3, 11, *(unsigned int *)(v3 + 488));
    }
    else
    {
      v12 = syna_dev_set_up_input_device(a1);
      if ( (v12 & 0x80000000) != 0 )
      {
        v14 = unk_3A2AF;
        goto LABEL_20;
      }
    }
  }
  else
  {
    printk(unk_32EDC, "syna_dev_connect", v16);
    if ( *(_BYTE *)(v3 + 9) == 11 && (syna_tcm_get_boot_info(v3, 0, 20) & 0x80000000) == 0 )
      printk(unk_39879, "syna_dev_connect", *(unsigned __int8 *)(v3 + 225));
  }
  v19 = syna_tcm_set_report_dispatcher(v3, 16, syna_dev_process_unexpected_reset, a1);
  if ( (v19 & 0x80000000) != 0 )
    v19 = printk(unk_3244F, "syna_dev_connect", v20);
  v21 = a1[78];
  v22 = syna_request_managed_device(v19);
  if ( v22 )
  {
    v26 = *(_DWORD *)(v21 + 168);
    if ( (v26 & 0x80000000) != 0 )
    {
      v24 = -22;
      v25 = unk_398A0;
      goto LABEL_37;
    }
    v27 = v22;
    v28 = gpio_to_desc(v26);
    v29 = gpiod_to_irq(v28);
    v30 = *(_QWORD *)(v21 + 176);
    *(_DWORD *)(v21 + 184) = v29;
    v31 = devm_request_threaded_irq(v27, v29, 0, syna_dev_isr, v30, "synaptics_tcm", a1);
    if ( (v31 & 0x80000000) != 0 )
    {
      v24 = v31;
      v25 = unk_3B7BA;
      goto LABEL_37;
    }
    *(_BYTE *)(v21 + 188) = 1;
    printk(unk_3601F, "syna_dev_request_irq", v23);
    a1[134] = alloc_workqueue(unk_364AC, 393226, 1, "syna_reflash");
    a1[121] = 0xFFFFFFFE00000LL;
    a1[122] = (__int64)(a1 + 122);
    a1[123] = (__int64)(a1 + 122);
    a1[124] = (__int64)syna_dev_reflash_startup_work;
    init_timer_key(a1 + 125, &delayed_work_timer_fn, 0x200000, 0, 0);
    queue_delayed_work_on(32, a1[134], a1 + 121, 50);
    v33 = *a1;
    *((_DWORD *)a1 + 351) = 1;
    *((_BYTE *)a1 + 1410) = 1;
    printk(unk_39326, "syna_dev_show_info", *(unsigned int *)(v33 + 56));
    printk(unk_34D1D, "syna_dev_show_info", "yes");
    if ( *((_BYTE *)a1 + 1409) )
      v34 = "yes";
    else
      v34 = (const char *)unk_398C0;
    printk(unk_3A30A, "syna_dev_show_info", v34);
    v9 = unk_39D58;
LABEL_11:
    printk(v9, "syna_dev_connect", "synaptics_tcm");
    return 0;
  }
  v24 = -517; // -EPROBE_DEFER
  v25 = unk_3BE43;
LABEL_37:
  printk(v25, "syna_dev_request_irq", v23);
  printk(unk_3A8CE, "syna_dev_connect", v32);
  if ( a1[118] )
  {
    input_unregister_device((struct input_dev *)a1[118]);
    a1[118] = 0;
  }
  return v24;
}
