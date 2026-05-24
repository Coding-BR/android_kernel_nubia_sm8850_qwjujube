__int64 __fastcall syna_dev_probe(__int64 a1, __int64 a2, __int64 a3)
{
  __int64 v4; // x0
  __int64 v5; // x2
  __int64 v6; // x21
  __int64 result; // x0
  __int64 v8; // x0
  __int64 v9; // x2
  __int64 v10; // x2
  __int64 v11; // x0
  __int64 v12; // x19
  __int64 v13; // x2
  unsigned int device; // w8
  int v15; // w9
  int v16; // w10
  int v17; // w9
  int v18; // w8
  int v19; // w10
  int v20; // w9
  int v21; // w8
  unsigned int v22; // w0
  __int64 v23; // x2
  unsigned int v24; // w21
  void *v25; // x0
  unsigned int v26; // w19
  unsigned int v27; // w0
  unsigned int v28; // w0
  __int64 v29; // x2
  unsigned int v30; // w0
  __int64 v31; // x2
  __int64 v32; // x2
  __int64 v33; // x0
  __int64 v34; // x20
  __int64 v35; // x2
  __int64 v36; // x2
  __int64 v37; // x8
  unsigned int boot_mode; // w0
  __int64 v39; // x8
  char v40; // w8
  _QWORD v41[2]; // [xsp+8h] [xbp-38h] BYREF
  int v42; // [xsp+18h] [xbp-28h]
  int v43; // [xsp+1Ch] [xbp-24h]
  int v44; // [xsp+20h] [xbp-20h]
  int v45; // [xsp+24h] [xbp-1Ch]
  int v46; // [xsp+28h] [xbp-18h]
  int v47; // [xsp+2Ch] [xbp-14h]
  int v48; // [xsp+30h] [xbp-10h]
  int v49; // [xsp+34h] [xbp-Ch]
  __int64 v50; // [xsp+38h] [xbp-8h]

  v50 = *(_QWORD *)(_ReadStatusReg(SP_EL0) + 1808);
  v41[0] = 0;
  v4 = printk(unk_365DA, "syna_dev_probe", a3);
  v6 = (__int64)syna_board_data_global;
  if ( !v6 || !*(_QWORD *)(v6 + 40) || !*(_QWORD *)(v6 + 48) )
  {
    printk(unk_36FB5, "syna_dev_probe: board_data or read/write operations not ready, deferring probe\n");
    result = 4294966779LL;
    goto LABEL_7;
  }
  v8 = syna_request_managed_device(v4);
  if ( !v8 )
  {
    printk(unk_3BE43, "syna_pal_mem_alloc", v9);
LABEL_6:
    printk(unk_3BE0E, "syna_dev_probe", v10);
    result = 4294967284LL;
    goto LABEL_7;
  }
  v11 = devm_kmalloc(v8, 1744, 3520);
  if ( !v11 )
    goto LABEL_6;
  v12 = v11;
  *(_DWORD *)(v11 + 1416) = 0;
  _init_swait_queue_head(v11 + 1424, "&x->wait", &init_completion___key_1);
  device = syna_tcm_allocate_device(v41, v6 + 8, v12);
  if ( (device & 0x80000000) != 0 || !v41[0] )
  {
    v25 = unk_35F48;
    v26 = device;
LABEL_13:
    printk(v25, "syna_dev_probe", v13);
    result = v26;
    goto LABEL_7;
  }
  *(_QWORD *)v12 = v41[0];
  *(_QWORD *)(v12 + 8) = a1;
  *(_QWORD *)(v12 + 624) = v6;
  v15 = *(_DWORD *)(v6 + 352);
  v16 = *(_DWORD *)(v6 + 368);
  v41[1] = *(_QWORD *)(v6 + 344);
  v42 = v15;
  v17 = *(_DWORD *)(v6 + 360);
  v18 = *(_DWORD *)(v6 + 356);
  v45 = v16;
  v19 = *(_DWORD *)(v6 + 248);
  v43 = v18;
  v44 = v17;
  v20 = *(_DWORD *)(v6 + 364);
  v46 = *(_DWORD *)(v6 + 372);
  v21 = *(_DWORD *)(v6 + 376);
  v49 = v19;
  v47 = v21;
  v48 = v20;
  v22 = syna_tcm_config_timings();
  if ( (v22 & 0x80000000) != 0 )
  {
    v24 = v22;
    printk(unk_318DC, "syna_dev_probe", v23);
LABEL_29:
    syna_tcm_remove_device(v41[0]);
    result = v24;
    goto LABEL_7;
  }
  *(_BYTE *)(v12 + 744) = 0;
  *(_QWORD *)(v12 + 680) = 0;
  *(_QWORD *)(v12 + 688) = 0;
  _mutex_init(v12 + 696, "(struct mutex *)ptr", &syna_pal_mutex_alloc___key);
  _mutex_init(v12 + 632, "(struct mutex *)ptr", &syna_pal_mutex_alloc___key);
  *(_BYTE *)(v12 + 756) = 0;
  *(_WORD *)(v12 + 1409) = 1;
  *(_QWORD *)(v12 + 1704) = syna_dev_connect;
  *(_QWORD *)(v12 + 1712) = syna_dev_disconnect;
  *(_QWORD *)(v12 + 1720) = syna_dev_set_up_app_fw;
  *(_QWORD *)(v12 + 1728) = syna_dev_resume;
  *(_DWORD *)(v12 + 1404) = 0;
  *(_QWORD *)(v12 + 1736) = syna_dev_suspend;
  *(_QWORD *)(v12 + 1120) = _kmalloc_large_noprof(204800, 3264);
  _mutex_init(v12 + 1136, "&tcm->frame_mutex", &syna_dev_probe___key);
  _init_waitqueue_head(v12 + 1096, "&(tcm->frame_mmap_list.wait)", &syna_dev_probe___key_53);
  *(_DWORD *)(v12 + 1128) = 0;
  *(_DWORD *)(v12 + 1184) = 255;
  *(_QWORD *)(v12 + 1192) = "input_agent";
  *(_QWORD *)(v12 + 1200) = &zte_fops;
  *(_QWORD *)(v12 + 1224) = 0;
  v27 = misc_register(v12 + 1184);
  if ( v27 )
  {
    v26 = v27;
    v25 = unk_38836;
    goto LABEL_13;
  }
  *(_QWORD *)(v12 + 1448) = 0;
  syna_tcm_handle_global = (void *)v12;
  device_set_wakeup_capable(&((struct platform_device *)a1)->dev, 1);
  device_wakeup_enable(&((struct platform_device *)a1)->dev);
  v28 = syna_dev_connect(v12, 0, 0);
  if ( (v28 & 0x80000000) != 0 )
  {
    v24 = v28;
    printk(unk_38208, "syna_dev_probe", v29);
LABEL_23:
    kfree(*(_QWORD *)(v12 + 1120));
    v33 = 0;
    if ( *(_BYTE *)(v12 + 744) )
      v33 = printk(unk_34845, "syna_tcm_buf_release", *(unsigned __int8 *)(v12 + 744));
    v34 = *(_QWORD *)(v12 + 680);
    if ( syna_request_managed_device(v33) )
    {
      if ( v34 )
        devm_kfree(syna_request_managed_device(v33), (void *)v34);
    }
    else
    {
      printk(unk_3BE43, "syna_pal_mem_free", v35);
    }
    *(_QWORD *)(v12 + 688) = 0;
    *(_BYTE *)(v12 + 744) = 0;
    goto LABEL_29;
  }
  v30 = syna_cdev_create(v12, a1);
  if ( (v30 & 0x80000000) != 0 )
  {
    v24 = v30;
    printk(unk_31E7C, "syna_dev_probe", v31);
LABEL_22:
    syna_dev_disconnect(v12, 0, 0);
    goto LABEL_23;
  }
  if ( (syna_sysfs_create_dir(v12, a1) & 0x80000000) != 0 )
  {
    printk(unk_31EAB, "syna_dev_probe", v32);
    syna_cdev_remove(v12);
    v24 = -20;
    goto LABEL_22;
  }
  syna_ts_register_for_panel_events(v12);
  printk(unk_393AC, "tpd_resume_suspend_register", v36);
  v37 = tpd_cdev;
  *(_QWORD *)(tpd_cdev + 3152) = tpd_goodix_ts_resume;
  *(_QWORD *)(v37 + 3104) = v12;
  *(_QWORD *)(v37 + 3160) = tpd_goodix_ts_suspend;
  *(_DWORD *)(v12 + 1368) = 0;
  _init_swait_queue_head(v12 + 1376, "&x->wait", &init_completion___key_1);
  *(_BYTE *)(v12 + 1400) = 0;
  *(_DWORD *)(v12 + 1312) = 0;
  *(_QWORD *)(v12 + 1352) = alloc_workqueue(unk_364AC, 393226, 1, "synaptics_tcm_helper");
  *(_QWORD *)(v12 + 1320) = 0xFFFFFFFE00000LL;
  *(_QWORD *)(v12 + 1328) = v12 + 1328;
  *(_QWORD *)(v12 + 1336) = v12 + 1328;
  *(_QWORD *)(v12 + 1344) = syna_dev_helper_work;
  *(_QWORD *)(v12 + 1536) = 0x200000002LL;
  *(_DWORD *)(v12 + 1548) = 2;
  *(_QWORD *)(v12 + 1456) = 0;
  *(_DWORD *)(v12 + 1464) = 0;
  *(_DWORD *)(v12 + 1476) = 0;
  *(_DWORD *)(v12 + 1492) = 0;
  *(_DWORD *)(v12 + 1500) = 0;
  *(_QWORD *)(v12 + 1528) = 0x200000001LL;
  *(_QWORD *)(v12 + 1512) = 0;
  *(_QWORD *)(v12 + 1520) = 0;
  boot_mode = zte_get_boot_mode();
  *(_DWORD *)(v12 + 1560) = boot_mode;
  printk(unk_36FE8, "syna_dev_probe", boot_mode);
  printk(unk_31906, "syna_dev_probe", "synaptics_tcm");
  v39 = tpd_cdev;
  *(_BYTE *)(v12 + 1411) = 1;
  *(_BYTE *)(v39 + 29) = 1;
  *(_BYTE *)(v39 + 1094) = 1;
  v40 = completion_done(v12 + 1416);
  result = 0;
  if ( (v40 & 1) == 0 )
  {
    complete(v12 + 1416);
    result = 0;
  }
LABEL_7:
  _ReadStatusReg(SP_EL0);
  return result;
}
