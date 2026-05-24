__int64 __fastcall syna_dev_set_up_input_device(__int64 a1)
{
  __int64 v1; // x2
  __int64 v3; // x8
  int v4; // w9
  __int64 v5; // x2
  __int64 v6; // x0
  unsigned int *v7; // x21
  __int64 v8; // x2
  __int64 device; // x0
  __int64 v10; // x2
  __int64 v11; // x20
  __int64 v12; // x8
  unsigned int v13; // w0
  __int64 v14; // x2
  unsigned int *v15; // x0
  __int64 v16; // x2
  __int64 v18; // x2
  unsigned int v19; // w21
  unsigned __int64 v26; // x9
  unsigned __int64 v29; // x9
  unsigned __int64 v32; // x9
  unsigned __int64 v35; // x9
  unsigned __int64 v38; // x9
  unsigned __int64 v41; // x9
  unsigned __int64 v44; // x9
  struct device *managed_dev;
  struct input_dev *input_dev;
  struct platform_device *pdev;
  struct device *parent;

  v1 = *(unsigned __int8 *)(*(_QWORD *)a1 + 9LL);
  if ( (_DWORD)v1 != 1 )
  {
    printk(unk_32EDC, "syna_dev_set_up_input_device", v1);
    return 0;
  }
  syna_dev_free_input_events(a1);
  v3 = *(_QWORD *)a1;
  v4 = *(_DWORD *)(*(_QWORD *)a1 + 16LL);
  if ( !v4 && !*(_DWORD *)(v3 + 20) )
    return 0;
  if ( *(_DWORD *)(a1 + 952) == v4 && *(_DWORD *)(a1 + 956) == *(_DWORD *)(v3 + 20) )
  {
    v5 = *(unsigned int *)(a1 + 960);
    if ( (_DWORD)v5 == *(_DWORD *)(v3 + 24) )
    {
      if ( (unsigned int)v5 < 0xB )
        printk(unk_3CB57, "syna_dev_check_input_params", v5);
      else
        printk(unk_3BE7D, "syna_dev_check_input_params", v5);
      return 0;
    }
  }
  mutex_lock(a1 + 632);
  v6 = *(_QWORD *)(a1 + 944);
  if ( v6 )
  {
    input_unregister_device((struct input_dev *)v6);
    *(_QWORD *)(a1 + 944) = 0;
  }
  v7 = *(unsigned int **)a1;
  managed_dev = (struct device *)syna_request_managed_device(v6);
  if ( managed_dev )
  {
    input_dev = devm_input_allocate_device(managed_dev);
    if ( input_dev )
    {
      v11 = (__int64)input_dev;
      pdev = *(struct platform_device **)(a1 + 8);
      parent = pdev ? &pdev->dev : managed_dev;

      input_dev->name = "synaptics_tcm_touch";
      input_dev->phys = "synaptics_tcm/touch_input";
      input_dev->id.bustype = 1;
      input_dev->id.vendor = 1;
      input_dev->dev.parent = parent;
      input_set_drvdata(input_dev, (void *)a1);

      __set_bit(INPUT_PROP_DIRECT, input_dev->propbit);
      __set_bit(EV_SYN, input_dev->evbit);
      __set_bit(EV_KEY, input_dev->evbit);
      __set_bit(EV_ABS, input_dev->evbit);
      __set_bit(KEY_WAKEUP, input_dev->keybit);
      __set_bit(BTN_TOUCH, input_dev->keybit);
      __set_bit(BTN_TOOL_FINGER, input_dev->keybit);
      input_set_capability(input_dev, EV_KEY, KEY_WAKEUP);
      input_set_abs_params(input_dev, ABS_MT_POSITION_X, 0, v7[4], 0, 0);
      input_set_abs_params(input_dev, ABS_MT_POSITION_Y, 0, v7[5], 0, 0);
      input_mt_init_slots(input_dev, v7[6], INPUT_MT_DIRECT);
      input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
      input_set_abs_params(input_dev, ABS_MT_TOUCH_MINOR, 0, 255, 0, 0);
      *(_DWORD *)(a1 + 952) = v7[4];
      *(_DWORD *)(a1 + 956) = v7[5];
      *(_DWORD *)(a1 + 960) = v7[6];
      v13 = input_register_device(v11);
      if ( (v13 & 0x80000000) == 0 )
      {
        v15 = *(unsigned int **)a1;
        *(_QWORD *)(a1 + 944) = v11;
        if ( (syna_tcm_set_report_dispatcher(v15, 17, syna_dev_process_touch_report, a1) & 0x80000000) != 0 )
          printk(unk_35F92, "syna_dev_set_up_input_device", v16);
        mutex_unlock(a1 + 632);
        return 0;
      }
      v19 = v13;
      printk(unk_375F8, "syna_dev_create_input_device", v14);
      input_free_device(v11);
    }
    else
    {
      printk(unk_375CB, "syna_dev_create_input_device", v10);
      v19 = -19;
    }
  }
  else
  {
    printk(unk_3BE43, "syna_dev_create_input_device", v8);
    v19 = -22;
  }
  printk(unk_3C485, "syna_dev_set_up_input_device", v18);
  mutex_unlock(a1 + 632);
  return v19;
}
