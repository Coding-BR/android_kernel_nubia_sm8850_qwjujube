__int64 __fastcall tpd_report_uevent(unsigned __int8 a1, __int64 a2, __int64 a3)
{
  __int64 v3; // x19
  const char *v4; // x8
  __int64 result; // x0
  _QWORD v6[3]; // [xsp+8h] [xbp-18h] BYREF

  v6[2] = *(_QWORD *)(_ReadStatusReg(SP_EL0) + 1808);
  v3 = tpd_cdev;
  v6[0] = 0;
  v6[1] = 0;
  if ( a1 == 2 )
  {
    printk(unk_380AB, "tpd_report_uevent", a3);
    v4 = "pen_capacity_low=true";
  }
  else if ( a1 == 1 )
  {
    printk(unk_38BFD, "tpd_report_uevent", a3);
    v4 = "double_tap=true";
  }
  else
  {
    if ( a1 )
    {
      result = printk(unk_396D3, "tpd_report_uevent", a1);
      goto LABEL_9;
    }
    printk(unk_3912B, "tpd_report_uevent", a3);
    v4 = "single_tap=true";
  }
  v6[0] = v4;
  struct platform_device *pdev = (struct platform_device *)*(_QWORD *)(v3 + 3096);
  result = kobject_uevent_env(&pdev->dev.kobj, 2, v6);
LABEL_9:
  _ReadStatusReg(SP_EL0);
  return result;
}
