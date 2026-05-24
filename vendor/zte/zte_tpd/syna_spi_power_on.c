__int64 __fastcall syna_spi_power_on(__int64 a1, char a2)
{
  const char *v4; // x2
  __int64 v5; // x2
  int v6; // w8
  unsigned int v7; // w20
  int v8; // w8
  __int64 v9; // x0
  unsigned int v10; // w20
  __int64 v11; // x0
  __int64 v12; // x0
  int v13; // w8
  void *v14; // x0
  __int64 v15; // x0
  _BOOL4 v16; // w20
  __int64 v17; // x0
  __int64 v18; // x0
  int v19; // w8
  __int64 v20; // x0
  _BOOL4 v21; // w20
  __int64 v22; // x0
  __int64 v23; // x0
  __int64 v24; // x0
  __int64 v25; // x2
  void *v26; // x2
  __int64 v27; // x0
  unsigned int v28; // w21
  __int64 v29; // x0
  __int64 v30; // x0

  if ( (a2 & 1) != 0 )
    v4 = "enable";
  else
    v4 = "disable";
  printk(unk_33D51, "syna_spi_power_on", v4);
  if ( (a2 & 1) == 0 )
  {
    v8 = *(_DWORD *)(a1 + 256);
    if ( v8 >= 1 )
    {
      if ( v8 == 1 )
      {
        regulator_disable(*(_QWORD *)(a1 + 272));
      }
      else
      {
        v15 = *(unsigned int *)(a1 + 280);
        if ( (int)v15 >= 1 )
        {
          v16 = *(_DWORD *)(a1 + 336) == 0;
          v17 = gpio_to_desc(v15);
          gpiod_set_raw_value(v17, v16);
        }
      }
      v18 = *(unsigned int *)(a1 + 292);
      if ( (int)v18 >= 1 )
        msleep(v18);
    }
    v19 = *(_DWORD *)(a1 + 296);
    if ( v19 >= 1 )
    {
      if ( v19 == 1 )
      {
        regulator_disable(*(_QWORD *)(a1 + 312));
      }
      else
      {
        v20 = *(unsigned int *)(a1 + 320);
        if ( (int)v20 >= 1 )
        {
          v21 = *(_DWORD *)(a1 + 336) == 0;
          v22 = gpio_to_desc(v20);
          gpiod_set_raw_value(v22, v21);
        }
      }
      v23 = *(unsigned int *)(a1 + 332);
      if ( (int)v23 >= 1 )
        msleep(v23);
    }
    v24 = gpio_to_desc(*(unsigned int *)(a1 + 240));
    gpiod_direction_output_raw(v24, 0);
    usleep_range_state(10000, 11000, 2);
    printk(unk_39E34, "syna_spi_power_on", v25);
    v7 = 0;
    v26 = unk_38E7D;
    goto LABEL_44;
  }
  v6 = *(_DWORD *)(a1 + 296);
  if ( v6 < 1 )
  {
    v7 = 0;
LABEL_18:
    v13 = *(_DWORD *)(a1 + 256);
    if ( v13 >= 1 )
    {
      if ( v13 == 1 )
      {
        if ( *(_QWORD *)(a1 + 272) >= 0xFFFFFFFFFFFFF001LL )
        {
          v14 = unk_3A480;
LABEL_40:
          printk(v14, "syna_spi_power_on", v5);
          return v7;
        }
        v7 = regulator_enable(*(_QWORD *)(a1 + 272));
        if ( (v7 & 0x80000000) != 0 )
        {
          v14 = unk_399EE;
          goto LABEL_40;
        }
      }
      else
      {
        v27 = *(unsigned int *)(a1 + 280);
        if ( (int)v27 >= 1 )
        {
          v28 = *(_DWORD *)(a1 + 336);
          v29 = gpio_to_desc(v27);
          gpiod_set_raw_value(v29, v28);
        }
      }
      v30 = *(unsigned int *)(a1 + 288);
      if ( (int)v30 >= 1 )
        msleep(v30);
    }
    v26 = unk_3711F;
LABEL_44:
    printk(unk_362D2, "syna_spi_power_on", v26);
    return v7;
  }
  if ( v6 != 1 )
  {
    v9 = *(unsigned int *)(a1 + 320);
    if ( (int)v9 >= 1 )
    {
      v10 = *(_DWORD *)(a1 + 336);
      v11 = gpio_to_desc(v9);
      gpiod_set_raw_value(v11, v10);
    }
    v7 = 0;
LABEL_16:
    v12 = *(unsigned int *)(a1 + 328);
    if ( (int)v12 >= 1 )
      msleep(v12);
    goto LABEL_18;
  }
  if ( *(_QWORD *)(a1 + 312) < 0xFFFFFFFFFFFFF001LL )
  {
    v7 = regulator_enable(*(_QWORD *)(a1 + 312));
    if ( (v7 & 0x80000000) != 0 )
    {
      v14 = unk_325D6;
      goto LABEL_40;
    }
    goto LABEL_16;
  }
  printk(unk_38350, "syna_spi_power_on", v5);
  return 0;
}
