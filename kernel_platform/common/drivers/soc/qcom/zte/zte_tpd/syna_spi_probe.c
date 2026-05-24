__int64 __fastcall syna_spi_probe(__int64 a1, __int64 a2, __int64 a3)
{
  __int64 v4; // x20
  __int64 property; // x0
  __int64 v6; // x0
  __int64 v7; // x0
  __int64 v8; // x0
  __int64 v9; // x0
  __int64 v10; // x0
  __int64 v11; // x0
  __int64 v12; // x0
  __int64 v13; // x0
  __int64 v14; // x0
  __int64 v15; // x0
  __int64 v16; // x0
  __int64 v17; // x0
  __int64 v18; // x0
  __int64 v19; // x0
  __int64 v20; // x0
  __int64 v21; // x0
  __int64 v22; // x0
  __int64 v23; // x0
  __int64 v24; // x0
  __int64 v25; // x0
  __int64 v26; // x0
  __int64 v27; // x0
  __int64 v28; // x0
  __int64 v29; // x0
  __int64 v30; // x0
  __int64 v31; // x0
  __int64 v32; // x0
  __int64 v33; // x0
  __int64 v34; // x0
  __int64 v35; // x0
  __int64 v36; // x2
  __int64 v37; // x8
  int v38; // w9
  unsigned __int64 v39; // x0
  __int64 v40; // x19
  void *v41; // x0
  unsigned int gpio; // w0
  unsigned int v43; // w19
  void *v44; // x0
  __int64 v45; // x2
  unsigned __int64 v46; // x0
  __int64 v47; // x19
  unsigned int v48; // w0
  __int64 v49; // x2
  __int64 v50; // x19
  unsigned int v51; // w8
  unsigned int v52; // w0
  __int64 v53; // x2
  _DWORD *v54; // x0
  __int64 v55; // x2
  int v56; // w8
  unsigned int v57; // w0
  unsigned int v58; // w0
  unsigned int v59; // w8
  __int64 result; // x0
  unsigned int v61; // w19
  __int64 v62; // x2
  __int64 v63; // x1
  __int64 v64; // x2
  __int64 v65; // x0
  unsigned int v66; // w8
  __int64 v67; // x0
  __int64 v68; // x1
  __int64 v69; // x0
  __int64 v70; // x2
  __int64 v71; // x1
  __int64 v72; // x2
  __int64 v73; // x0
  __int64 v74; // x0
  __int64 v75; // x0
  __int64 v76; // x0
  __int64 v77; // [xsp+10h] [xbp-20h] BYREF
  __int64 v78; // [xsp+18h] [xbp-18h]
  int v79; // [xsp+20h] [xbp-10h]
  __int64 v80; // [xsp+28h] [xbp-8h]

  v80 = *(_QWORD *)(_ReadStatusReg(SP_EL0) + 1808);
  printk(unk_3BFC2, "syna_spi_probe", a3);
  v4 = (__int64)((struct spi_device *)a1)->dev.of_node;
  v79 = 0;
  v77 = 0;
  v78 = 0;
  dword_C88 = -1;
  property = of_find_property(v4, "synaptics,irq-gpio", 0);
  if ( property && *(_DWORD *)(property + 8) )
    dword_C88 = of_get_named_gpio(v4, "synaptics,irq-gpio", 0);
  qword_C90 = 8200;
  v6 = of_find_property(v4, "synaptics,irq-flags", 0);
  if ( v6 && *(_DWORD *)(v6 + 8) )
  {
    of_property_read_variable_u32_array(v4, "synaptics,irq-flags", &v77, 1, 0);
    qword_C90 = (int)v77;
  }
  dword_C8C = 0;
  v7 = of_find_property(v4, "synaptics,irq-on-state", 0);
  if ( v7 && *(_DWORD *)(v7 + 8) )
    of_property_read_variable_u32_array(v4, "synaptics,irq-on-state", &dword_C8C, 1, 0);
  dword_D30 = 1;
  v8 = of_find_property(v4, "synaptics,power-on-state", 0);
  if ( v8 && *(_DWORD *)(v8 + 8) )
    of_property_read_variable_u32_array(v4, "synaptics,power-on-state", &dword_D30, 1, 0);
  dword_D34 = 0;
  v9 = of_find_property(v4, "synaptics,power-delay-ms", 0);
  if ( v9 && *(_DWORD *)(v9 + 8) )
    of_property_read_variable_u32_array(v4, "synaptics,power-delay-ms", &dword_D34, 1, 0);
  dword_CE0 = 0;
  v10 = of_find_property(v4, "synaptics,vdd-control", 0);
  if ( v10 && *(_DWORD *)(v10 + 8) )
    of_property_read_variable_u32_array(v4, "synaptics,vdd-control", &dword_CE0, 1, 0);
  qword_CE8 = 0;
  v11 = of_find_property(v4, "synaptics,vdd-name", 0);
  if ( v11 && *(_DWORD *)(v11 + 8) )
    of_property_read_string(v4, "synaptics,vdd-name", &qword_CE8);
  dword_CF8 = -1;
  v12 = of_find_property(v4, "synaptics,vdd-gpio", 0);
  if ( v12 && *(_DWORD *)(v12 + 8) )
    dword_CF8 = of_get_named_gpio(v4, "synaptics,vdd-gpio", 0);
  dword_D00 = 0;
  v13 = of_find_property(v4, "synaptics,vdd-power-on-delay-ms", 0);
  if ( v13 && *(_DWORD *)(v13 + 8) )
    of_property_read_variable_u32_array(v4, "synaptics,vdd-power-on-delay-ms", &dword_D00, 1, 0);
  dword_D04 = 0;
  v14 = of_find_property(v4, "synaptics,vdd-power-off-delay-ms", 0);
  if ( v14 && *(_DWORD *)(v14 + 8) )
    of_property_read_variable_u32_array(v4, "synaptics,vdd-power-off-delay-ms", &dword_D04, 1, 0);
  dword_D08 = 0;
  v15 = of_find_property(v4, "synaptics,vio-control", 0);
  if ( v15 && *(_DWORD *)(v15 + 8) )
    of_property_read_variable_u32_array(v4, "synaptics,vio-control", &dword_D08, 1, 0);
  qword_D10 = 0;
  v16 = of_find_property(v4, "synaptics,avdd-name", 0);
  if ( v16 && *(_DWORD *)(v16 + 8) )
    of_property_read_string(v4, "synaptics,avdd-name", &qword_D10);
  dword_D20 = -1;
  v17 = of_find_property(v4, "synaptics,vio-gpio", 0);
  if ( v17 && *(_DWORD *)(v17 + 8) )
    dword_D20 = of_get_named_gpio(v4, "synaptics,vio-gpio", 0);
  dword_D28 = 0;
  v18 = of_find_property(v4, "synaptics,vio-power-on-delay-ms", 0);
  if ( v18 && *(_DWORD *)(v18 + 8) )
    of_property_read_variable_u32_array(v4, "synaptics,vio-power-on-delay-ms", &dword_D28, 1, 0);
  dword_D2C = 0;
  v19 = of_find_property(v4, "synaptics,vio-power-off-delay-ms", 0);
  if ( v19 && *(_DWORD *)(v19 + 8) )
    of_property_read_variable_u32_array(v4, "synaptics,vio-power-off-delay-ms", &dword_D2C, 1, 0);
  dword_CD4 = 0;
  v20 = of_find_property(v4, "synaptics,reset-on-state", 0);
  if ( v20 && *(_DWORD *)(v20 + 8) )
    of_property_read_variable_u32_array(v4, "synaptics,reset-on-state", &dword_CD4, 1, 0);
  dword_CD0 = -1;
  v21 = of_find_property(v4, "synaptics,reset-gpio", 0);
  if ( v21 && *(_DWORD *)(v21 + 8) )
    dword_CD0 = of_get_named_gpio(v4, "synaptics,reset-gpio", 0);
  v22 = of_find_property(v4, "synaptics,reset-active-ms", 0);
  if ( v22 && *(_DWORD *)(v22 + 8) )
    of_property_read_variable_u32_array(v4, "synaptics,reset-active-ms", &dword_CDC, 1, 0);
  v23 = of_find_property(v4, "synaptics,reset-delay-ms", 0);
  if ( v23 && *(_DWORD *)(v23 + 8) )
    of_property_read_variable_u32_array(v4, "synaptics,reset-delay-ms", &dword_CD8, 1, 0);
  dword_C80 = -1;
  v24 = of_find_property(v4, "synaptics,io-switch-gpio", 0);
  if ( v24 && *(_DWORD *)(v24 + 8) )
    dword_C80 = of_get_named_gpio(v4, "synaptics,io-switch-gpio", 0);
  v25 = of_find_property(v4, "synaptics,io-switch-state", 0);
  if ( v25 && *(_DWORD *)(v25 + 8) )
    of_property_read_variable_u32_array(v4, "synaptics,io-switch-state", &dword_C84, 1, 0);
  dword_C44 = 0;
  v26 = of_find_property(v4, "synaptics,spi-byte-delay-us", 0);
  if ( v26 && *(_DWORD *)(v26 + 8) )
    of_property_read_variable_u32_array(v4, "synaptics,spi-byte-delay-us", &dword_C44, 1, 0);
  dword_C48 = 0;
  v27 = of_find_property(v4, "synaptics,spi-block-delay-us", 0);
  if ( v27 && *(_DWORD *)(v27 + 8) )
    of_property_read_variable_u32_array(v4, "synaptics,spi-block-delay-us", &dword_C48, 1, 0);
  dword_C40 = 0;
  v28 = of_find_property(v4, "synaptics,spi-mode", 0);
  if ( v28 && *(_DWORD *)(v28 + 8) )
    of_property_read_variable_u32_array(v4, "synaptics,spi-mode", &dword_C40, 1, 0);
  v29 = of_find_property(v4, "synaptics,chunks", 0);
  if ( v29
    && *(_DWORD *)(v29 + 8)
    && (of_property_read_variable_u32_array(v4, "synaptics,chunks", &v77, 2, 0) & 0x80000000) == 0 )
  {
    qword_BF4 = v77;
  }
  v30 = of_find_property(v4, "synaptics,flash-access-delay-us", 0);
  if ( v30
    && *(_DWORD *)(v30 + 8)
    && (of_property_read_variable_u32_array(v4, "synaptics,flash-access-delay-usy", &v77, 3, 0) & 0x80000000) == 0 )
  {
    qword_D50 = v77;
    dword_D58 = v78;
  }
  v31 = of_find_property(v4, "synaptics,command-timeout-ms", 0);
  if ( v31 && *(_DWORD *)(v31 + 8) )
    of_property_read_variable_u32_array(v4, "synaptics,command-timeout-ms", &dword_D38, 1, 0);
  v32 = of_find_property(v4, "synaptics,command-polling-ms", 0);
  if ( v32 && *(_DWORD *)(v32 + 8) )
    of_property_read_variable_u32_array(v4, "synaptics,command-polling-ms", unk_D3C, 1, 0);
  v33 = of_find_property(v4, "synaptics,command-turnaround-us", 0);
  if ( v33
    && *(_DWORD *)(v33 + 8)
    && (of_property_read_variable_u32_array(v4, "synaptics,command-turnaround-us", &v77, 2, 0) & 0x80000000) == 0 )
  {
    qword_D40 = v77;
  }
  v34 = of_find_property(v4, "synaptics,command-retry-ms", 0);
  if ( v34 && *(_DWORD *)(v34 + 8) )
    of_property_read_variable_u32_array(v4, "synaptics,command-retry-ms", &dword_D48, 1, 0);
  v35 = of_find_property(v4, "synaptics,fw-switch-delay-ms", 0);
  if ( v35 && *(_DWORD *)(v35 + 8) )
    of_property_read_variable_u32_array(v4, "synaptics,fw-switch-delay-ms", &dword_D4C, 1, 0);
  printk(unk_36260, "syna_spi_parse_dt", (unsigned int)qword_BF4);
  printk(unk_394A5, "syna_spi_parse_dt", (unsigned int)dword_D38);
  printk(unk_3D259, "syna_spi_parse_dt", (unsigned int)qword_D50);
  syna_ts_check_dt(a1);
  syna_spi_hw_if = a1;
  qword_BE8 = (__int64)&syna_spi_hw_if;
  {
    struct syna_hw_interface_layout *hw = (struct syna_hw_interface_layout *)&syna_spi_hw_if;
    v37 = p_device;
    syna_spi_dev_global = (struct spi_device *)a1;
    syna_board_data_global = &syna_spi_hw_if;
    hw->read = (void *)sub_1DAFC;
    hw->write = (void *)sub_1DDE0;
    hw->enable_irq = (void *)syna_spi_enable_irq;
    hw->power_on = (void *)syna_spi_power_on;
    hw->hw_reset = (void *)syna_spi_hw_reset;
  }
  v38 = dword_D08;
  if ( !dword_D08 )
  {
    v38 = qword_D10;
    if ( qword_D10 )
    {
      v38 = *(unsigned __int8 *)qword_D10;
      if ( *(_BYTE *)qword_D10 )
      {
        v38 = 1;
        dword_D08 = 1;
      }
    }
  }
  if ( !dword_CE0 && qword_CE8 )
    dword_CE0 = *(_BYTE *)qword_CE8 != 0;
  if ( v38 == 1 )
  {
    if ( !qword_D10 || !*(_BYTE *)qword_D10 )
    {
      v41 = unk_33763;
      goto LABEL_161;
    }
    v39 = devm_regulator_get(&((struct platform_device *)p_device)->dev, qword_D10);
    if ( v39 >= 0xFFFFFFFFFFFFF001LL )
    {
      v40 = v39;
      printk(unk_3C5CC, "syna_spi_get_regulator", v36);
      v41 = unk_38379;
      qword_D18 = v40;
LABEL_161:
      printk(v41, "syna_spi_request_power_resources", v36);
      v43 = -6;
      goto LABEL_162;
    }
    qword_D18 = v39;
  }
  else if ( v38 >= 1 && dword_D20 >= 1 )
  {
    gpio = syna_spi_get_gpio((unsigned int)dword_D20, 1);
    if ( (gpio & 0x80000000) != 0 )
    {
      v43 = gpio;
      v44 = unk_3782F;
      v45 = (unsigned int)dword_D20;
      goto LABEL_123;
    }
  }
  if ( dword_CE0 != 1 )
  {
    if ( dword_CE0 < 1 )
      goto LABEL_125;
    if ( dword_CF8 < 1 )
      goto LABEL_125;
    v48 = syna_spi_get_gpio((unsigned int)dword_CF8, 1);
    if ( (v48 & 0x80000000) == 0 )
      goto LABEL_125;
    v43 = v48;
    v44 = unk_34E9A;
    v45 = (unsigned int)dword_CF8;
LABEL_123:
    printk(v44, "syna_spi_request_power_resources", v45);
LABEL_162:
    printk(unk_3AA95, "syna_spi_probe", v49);
LABEL_226:
    result = v43;
    goto LABEL_227;
  }
  if ( !qword_CE8 || !*(_BYTE *)qword_CE8 )
  {
    v41 = unk_38E81;
    goto LABEL_161;
  }
  v46 = devm_regulator_get(&syna_spi_dev_global->dev, qword_CE8);
  if ( v46 >= 0xFFFFFFFFFFFFF001LL )
  {
    v47 = v46;
    printk(unk_3C5CC, "syna_spi_get_regulator", v36);
    v41 = unk_3424B;
    qword_CF0 = v47;
    goto LABEL_161;
  }
  qword_CF0 = v46;
LABEL_125:
  v50 = syna_spi_hw_if;
  if ( !syna_spi_hw_if )
  {
    v43 = -22;
LABEL_214:
    printk(unk_3709F, "syna_spi_probe", v36);
    if ( dword_D08 == 1 )
    {
      if ( qword_D18 )
        devm_regulator_put(qword_D18, v68);
    }
    else if ( dword_D08 >= 1 && dword_D20 >= 1 )
    {
      gpio_free((unsigned int)dword_D20, v68);
    }
    if ( dword_CE0 != 1 )
    {
      if ( dword_CE0 < 1 )
        goto LABEL_226;
      v76 = (unsigned int)dword_CF8;
      if ( dword_CF8 < 1 )
        goto LABEL_226;
      goto LABEL_225;
    }
    if ( !qword_CF0 )
      goto LABEL_226;
    goto LABEL_192;
  }
  _mutex_init(unk_C50, "(struct mutex *)ptr", &syna_pal_mutex_alloc___key_1);
  v51 = dword_C40;
  ((struct spi_device *)v50)->bits_per_word = 8;
  if ( v51 <= 3 )
    ((struct spi_device *)v50)->mode = v51;
  v52 = spi_setup(v50);
  if ( (v52 & 0x80000000) != 0 )
  {
    v43 = v52;
    printk(unk_37122, "syna_spi_request_bus_resources", v53);
    goto LABEL_214;
  }
  v54 = (_DWORD *)_kmalloc_cache_noprof(memset, 3520, 8);
  if ( !v54 )
  {
    printk(unk_3785E, "syna_spi_request_bus_resources", v55);
    v43 = -12;
    goto LABEL_214;
  }
  v56 = dword_C80;
  *v54 = 1022;
  ((struct spi_device *)v50)->controller_data = v54;
  if ( v56 >= 1 )
  {
    v57 = syna_spi_get_gpio((unsigned int)v56, 1);
    if ( (v57 & 0x80000000) != 0 )
    {
      v43 = v57;
      printk(unk_34EE8, "syna_spi_request_bus_resources", (unsigned int)dword_C80);
      goto LABEL_214;
    }
  }
  if ( dword_CD0 >= 1 )
  {
    v58 = syna_spi_get_gpio((unsigned int)dword_CD0, 1);
    if ( (v58 & 0x80000000) != 0 )
    {
      v43 = v58;
      printk(unk_33087, "syna_spi_request_reset_resources", (unsigned int)dword_CD0);
      printk(unk_3AAC8, "syna_spi_probe", v70);
      if ( dword_C80 >= 1 )
        gpio_free((unsigned int)dword_C80, v71);
      result = v43;
      if ( rx_buf )
      {
        if ( syna_spi_dev_global && (v73 = (__int64)&syna_spi_dev_global->dev) != 0 )
          devm_kfree((struct device *)v73, rx_buf);
        else
          printk(unk_3BE43, "syna_pal_mem_free", v72);
        result = v43;
        rx_buf = 0;
      }
      if ( tx_buf )
      {
        if ( syna_spi_dev_global && (v74 = (__int64)&syna_spi_dev_global->dev) != 0 )
          devm_kfree((struct device *)v74, tx_buf);
        else
          printk(unk_3BE43, "syna_pal_mem_free", v72);
        result = v43;
        tx_buf = 0;
      }
      v68 = xfer;
      if ( xfer )
      {
        if ( syna_spi_dev_global && (v75 = (__int64)&syna_spi_dev_global->dev) != 0 )
          devm_kfree((struct device *)v75, xfer);
        else
          printk(unk_3BE43, "syna_pal_mem_free", v72);
        result = v43;
        xfer = 0;
      }
      if ( dword_D08 == 1 )
      {
        if ( !qword_D18 )
        {
LABEL_190:
          if ( dword_CE0 != 1 )
          {
            if ( dword_CE0 >= 1 && dword_CF8 >= 1 )
            {
              v76 = (unsigned int)dword_CF8;
              goto LABEL_225;
            }
            goto LABEL_227;
          }
          if ( !qword_CF0 )
            goto LABEL_227;
LABEL_192:
          devm_regulator_put(qword_CF0, v68);
          goto LABEL_226;
        }
        devm_regulator_put(qword_D18, v68);
      }
      else
      {
        if ( dword_D08 < 1 || dword_D20 < 1 )
          goto LABEL_190;
        gpio_free((unsigned int)dword_D20, v68);
      }
      result = v43;
      goto LABEL_190;
    }
  }
  _mutex_init(unk_CA0, "(struct mutex *)ptr", &syna_pal_mutex_alloc___key_1);
  if ( dword_C88 < 1 )
  {
    result = 0;
  }
  else
  {
    v59 = syna_spi_get_gpio((unsigned int)dword_C88, 0);
    result = 0;
    if ( (v59 & 0x80000000) != 0 )
    {
      v61 = v59;
      printk(unk_38EC4, "syna_spi_request_attn_resources", (unsigned int)dword_C88);
      printk(unk_3D206, "syna_spi_probe", v62);
      if ( dword_CD0 >= 1 )
        gpio_free((unsigned int)dword_CD0, v63);
      if ( dword_C80 >= 1 )
        gpio_free((unsigned int)dword_C80, v63);
      if ( rx_buf )
      {
        if ( syna_spi_dev_global && (v65 = (__int64)&syna_spi_dev_global->dev) != 0 )
          devm_kfree((struct device *)v65, rx_buf);
        else
          printk(unk_3BE43, "syna_pal_mem_free", v64);
        rx_buf = 0;
      }
      v66 = v61;
      if ( tx_buf )
      {
        if ( syna_spi_dev_global && (v67 = (__int64)&syna_spi_dev_global->dev) != 0 )
          devm_kfree((struct device *)v67, tx_buf);
        else
          printk(unk_3BE43, "syna_pal_mem_free", v64);
        v66 = v61;
        tx_buf = 0;
      }
      v68 = xfer;
      if ( xfer )
      {
        if ( syna_spi_dev_global && (v69 = (__int64)&syna_spi_dev_global->dev) != 0 )
          devm_kfree((struct device *)v69, xfer);
        else
          printk(unk_3BE43, "syna_pal_mem_free", v64);
        v66 = v61;
        xfer = 0;
      }
      if ( dword_D08 == 1 )
      {
        if ( !qword_D18 )
        {
LABEL_200:
          if ( dword_CE0 == 1 )
          {
            if ( qword_CF0 )
            {
              v43 = v66;
              devm_regulator_put(qword_CF0, v68);
              goto LABEL_226;
            }
            goto LABEL_206;
          }
          if ( dword_CE0 < 1 || (v76 = (unsigned int)dword_CF8, dword_CF8 < 1) )
          {
LABEL_206:
            result = v66;
            goto LABEL_227;
          }
          v43 = v66;
LABEL_225:
          gpio_free(v76, v68);
          goto LABEL_226;
        }
        devm_regulator_put(qword_D18, v68);
      }
      else
      {
        if ( dword_D08 < 1 || dword_D20 < 1 )
          goto LABEL_200;
        gpio_free((unsigned int)dword_D20, v68);
      }
      v66 = v61;
      goto LABEL_200;
    }
  }
LABEL_227:
  _ReadStatusReg(SP_EL0);
  return result;
}
