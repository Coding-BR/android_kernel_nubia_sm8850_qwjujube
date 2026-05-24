__int64 __fastcall syna_dev_remove(__int64 a1, __int64 a2, __int64 a3)
{
  __int64 v3; // x19
  __int64 v4; // x0
  __int64 v5; // x20
  __int64 v6; // x0
  __int64 v7; // x2
  __int64 v8; // x2
  __int64 v9; // x20
  __int64 v10; // x0
  __int64 v11; // x2
  __int64 v12; // x0

  v3 = (__int64)syna_tcm_handle_global;
  if ( !v3 )
    printk(unk_38D7D, "syna_dev_remove", a3);
  printk(unk_34878, "syna_dev_remove", a3);
  cancel_work_sync(v3 + 1320);
  _flush_workqueue(*(_QWORD *)(v3 + 1352));
  destroy_workqueue(*(_QWORD *)(v3 + 1352));
  if ( active_panel && *(_QWORD *)(v3 + 1360) )
    panel_event_notifier_unregister();
  syna_sysfs_remove_dir(v3);
  syna_cdev_remove(v3);
  v4 = syna_dev_disconnect(v3, 0, 0);
  v5 = *(_QWORD *)(v3 + 1448);
  if ( v5 )
  {
    v6 = syna_request_managed_device(v4);
    if ( !v6 )
    {
      v4 = printk(unk_3BE43, "syna_pal_mem_free", v7);
      v8 = *(unsigned __int8 *)(v3 + 744);
      if ( !*(_BYTE *)(v3 + 744) )
        goto LABEL_10;
      goto LABEL_16;
    }
    v4 = devm_kfree(v6, v5);
  }
  v8 = *(unsigned __int8 *)(v3 + 744);
  if ( !*(_BYTE *)(v3 + 744) )
    goto LABEL_10;
LABEL_16:
  v4 = printk(unk_34845, "syna_tcm_buf_release", v8);
LABEL_10:
  v9 = *(_QWORD *)(v3 + 680);
  v10 = syna_request_managed_device(v4);
  if ( v10 )
  {
    if ( v9 )
      devm_kfree(v10, v9);
  }
  else
  {
    printk(unk_3BE43, "syna_pal_mem_free", v11);
  }
  v12 = *(_QWORD *)(v3 + 1120);
  *(_QWORD *)(v3 + 688) = 0;
  *(_BYTE *)(v3 + 744) = 0;
  kfree(v12);
  return syna_tcm_remove_device(*(_QWORD *)v3);
}
