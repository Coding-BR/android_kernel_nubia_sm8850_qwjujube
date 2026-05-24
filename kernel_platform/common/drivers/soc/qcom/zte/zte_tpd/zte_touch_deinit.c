__int64 __fastcall zte_touch_deinit(__int64 a1, __int64 a2, __int64 a3)
{
  __int64 v3; // x22
  __int64 result; // x0
  __int64 v5; // x0
  __int64 v6; // x1
  __int64 v7; // x2
  __int64 v8; // x23
  __int64 v9; // x2
  __int64 v10; // x0
  __int64 v11; // x0
  __int64 v12; // x24
  __int64 v13; // x2
  __int64 v14; // x19
  void (__fastcall *v15)(__int64); // x8

  v3 = tpd_cdev;
  if ( !tpd_cdev || zte_touch_deinit_ztp_release == 1 )
    return printk(unk_323A0, a2, a3);
  v5 = ufp_mac_exit();
  tpd_proc_deinit(v5, v6, v7);
  v8 = tpd_cdev;
  v10 = printk(unk_322AA, "tpd_workqueue_deinit", v9);
  v11 = tpd_report_work_deinit(v10);
  tpd_resume_work_deinit(v11);
  v12 = tpd_cdev;
  printk(unk_322AA, "tpd_probe_work_deinit", v13);
  cancel_delayed_work_sync(v12 + 2256);
  cancel_delayed_work_sync(v8 + 2360);
  if ( !*(_QWORD *)(v3 + 1200) )
    destroy_workqueue(0);
  v14 = tpd_cdev;
  result = *(_QWORD *)(tpd_cdev + 2712);
  if ( result )
  {
    sysfs_remove_bin_file(result, &fwimage_attr);
    kobject_put(*(_QWORD *)(v14 + 2712));
    v14 = tpd_cdev;
  }
  if ( !*(_QWORD *)(v14 + 3096) )
  {
    gpio_free(16);
    platform_device_unregister(*(_QWORD *)(v14 + 3096));
  }
  zte_touch_deinit_ztp_release = 1;
  return 0;
}
