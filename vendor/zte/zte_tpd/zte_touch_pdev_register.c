__int64 __fastcall zte_touch_pdev_register(__int64 a1, __int64 a2, __int64 a3)
{
  __int64 v3; // x20
  __int64 v4; // x0
  __int64 v5; // x2
  unsigned int v6; // w0
  __int64 result; // x0
  __int64 (__fastcall *v8)(); // x8
  unsigned int v9; // w19
  __int64 v10; // x8
  void (__fastcall *v11)(_QWORD); // x9

  v3 = tpd_cdev;
  printk(unk_37BB9, "zte_touch_pdev_register", a3);
  v4 = platform_device_alloc("zte_touch", 0xFFFFFFFFLL);
  *(_QWORD *)(v3 + 3096) = v4;
  if ( v4 )
  {
    v6 = platform_device_add((struct platform_device *)v4);
    if ( (v6 & 0x80000000) != 0 )
    {
      v9 = v6;
      printk(unk_3AF12, "zte_touch_pdev_register", v6);
      v10 = *(_QWORD *)(v3 + 3096);
      platform_device_put((struct platform_device *)v10);
      result = v9;
      v8 = NULL;
    }
    else
    {
      result = 0;
      v8 = tpd_report_uevent;
    }
  }
  else
  {
    printk(unk_3227D, "zte_touch_pdev_register", v5);
    v8 = NULL;
    result = 4294967284LL;
  }
  *(_QWORD *)(v3 + 3304) = v8;
  return result;
}
