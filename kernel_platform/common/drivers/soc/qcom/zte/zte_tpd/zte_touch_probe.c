__int64 __fastcall zte_touch_probe(__int64 a1)
{
  struct platform_device *pdev = (struct platform_device *)a1;
  __int64 v2; // x0
  __int64 v3; // x1
  __int64 v4; // x2
  __int64 v5; // x19
  __int64 v6; // x20
  __int64 property; // x0
  __int64 v8; // x1
  __int64 v9; // x2
  __int64 v10; // x0
  __int64 v11; // x1
  __int64 v12; // x2
  __int64 v13; // x0
  __int64 v14; // x1
  __int64 v15; // x2
  char v16; // w8
  char v17; // w8
  char v18; // w8
  char v19; // w8
  char v20; // w8
  int v21; // w8
  int v22; // w8
  int v23; // w8
  int v24; // w0
  int v25; // w8
  __int64 v26; // x1
  __int64 v27; // x2
  __int64 v28; // x2
  __int64 v29; // x2
  __int64 v30; // x2
  __int64 v31; // x2
  __int64 v32; // x2
  __int64 v33; // x2
  __int64 v34; // x2
  __int64 v35; // x2
  __int64 v36; // x2
  __int64 v37; // x0
  __int64 v38; // x1
  __int64 v39; // x2
  __int64 v40; // x0
  __int64 v41; // x2
  __int64 v42; // x1
  __int64 v43; // x2
  __int64 v44; // x1
  __int64 v45; // x2
  __int64 v46; // x1
  __int64 v47; // x2
  __int64 v48; // x1
  __int64 v49; // x2
  __int64 v50; // x1
  __int64 v51; // x2
  __int64 v52; // x1
  __int64 v53; // x2
  __int64 v54; // x1
  __int64 v55; // x2
  __int64 v56; // x1
  __int64 v57; // x2
  __int64 v58; // x1
  __int64 v59; // x2
  __int64 v60; // x1
  __int64 v61; // x2
  __int64 v62; // x1
  __int64 v63; // x2
  __int64 v64; // x1
  __int64 v65; // x2
  __int64 v66; // x1
  __int64 v67; // x2
  __int64 v68; // x1
  __int64 v69; // x2
  __int64 v70; // x1
  __int64 v71; // x2
  __int64 v72; // x1
  __int64 v73; // x2
  __int64 v74; // x1
  __int64 v75; // x2
  __int64 v76; // x1
  __int64 v77; // x2
  __int64 v78; // x1
  __int64 v79; // x2
  __int64 v80; // x1
  __int64 v81; // x2
  __int64 v82; // x1
  __int64 v83; // x2
  __int64 v84; // x1
  __int64 v85; // x2
  __int64 v86; // x1
  __int64 v87; // x2
  __int64 v88; // x1
  __int64 v89; // x2
  __int64 v90; // x1
  __int64 v91; // x2
  __int64 v92; // x1
  __int64 v93; // x2
  __int64 v94; // x1
  __int64 v95; // x2
  __int64 v96; // x1
  __int64 v97; // x2
  __int64 v98; // x1
  __int64 v99; // x2
  __int64 v100; // x1
  __int64 v101; // x2
  __int64 v102; // x1
  __int64 v103; // x2
  __int64 v104; // x1
  __int64 v105; // x2
  __int64 v106; // x20
  __int64 v107; // x8
  __int64 v108; // x0
  __int64 v109; // x1
  __int64 v110; // x2
  unsigned int bin_file; // w0
  __int64 v112; // x2
  __int64 v113; // x0
  __int64 v114; // x0
  __int64 v115; // x0
  __int64 v116; // x1
  __int64 v117; // x2
  __int64 result; // x0
  __int64 v119; // x1
  __int64 v120; // x2
  __int64 v121; // x1
  __int64 v122; // x1
  __int64 v123; // x2
  __int64 v124; // x2
  __int64 v125; // x2
  __int64 v126; // x1
  __int64 v127; // x1
  __int64 v128; // x1
  int v129; // [xsp+4h] [xbp-Ch] BYREF
  __int64 v130; // [xsp+8h] [xbp-8h]

  v130 = *(_QWORD *)(_ReadStatusReg(SP_EL0) + 1808);
  if (pdev) {
    if (pdev->name) {
      printk("zte_touch_probe: probing device named '%s'\n", pdev->name);
      if (strcmp(pdev->name, "zte_touch") == 0) {
        printk("zte_touch_probe: ignoring virtual device 'zte_touch' to prevent recursion\n");
        return 0;
      }
    } else {
      printk("zte_touch_probe: pdev->name is NULL\n");
    }
  } else {
    printk("zte_touch_probe: pdev is NULL\n");
  }
  printk(unk_322D0, "zte_touch_probe", 2858);
  v2 = devm_kmalloc(&((struct platform_device *)a1)->dev, 3664, 3520);
  if ( v2 )
  {
    v5 = v2;
    *(_QWORD *)(v2 + 3088) = a1;
    tpd_cdev = v2;
    zte_touch_dev_global = (void *)v2;
    zte_touch_pdev_register(v2, v3, v4);
    v6 = (__int64)((struct platform_device *)a1)->dev.of_node;
    v129 = 0;
    property = of_find_property(v6, "zte,tp_algo", 0);
    *(_BYTE *)(v5 + 27) = property != 0;
    if ( property )
      printk(unk_3339E, v8, v9);
    v10 = of_find_property(v6, "zte,tp_long_press", 0);
    *(_BYTE *)(v5 + 37) = v10 != 0;
    if ( v10 )
    {
      printk(unk_3A0BE, v11, v12);
      if ( (of_property_read_variable_u32_array(v6, "zte,tp_long_press_timer", &v129, 1, 0) & 0x80000000) == 0 )
        *(_WORD *)(v5 + 60) = v129;
      if ( (of_property_read_variable_u32_array(v6, "zte,tp_long_press_left_v", &v129, 1, 0) & 0x80000000) == 0 )
        *(_BYTE *)(v5 + 52) = v129;
      if ( (of_property_read_variable_u32_array(v6, "zte,tp_long_press_right_v", &v129, 1, 0) & 0x80000000) == 0 )
        *(_BYTE *)(v5 + 53) = v129;
      if ( (of_property_read_variable_u32_array(v6, "zte,tp_long_press_left_h", &v129, 1, 0) & 0x80000000) == 0 )
        *(_BYTE *)(v5 + 54) = v129;
      if ( (of_property_read_variable_u32_array(v6, "zte,tp_long_press_right_h", &v129, 1, 0) & 0x80000000) == 0 )
        *(_BYTE *)(v5 + 55) = v129;
    }
    v13 = of_find_property(v6, "zte,ghost_check_config", 0);
    *(_BYTE *)(v5 + 1168) = v13 != 0;
    if ( v13 )
    {
      printk(unk_36E60, v14, v15);
      if ( (int)of_property_read_variable_u32_array(v6, "zte,ghost_check_single_time", &v129, 1, 0) < 0 )
        v16 = 25;
      else
        v16 = v129;
      *(_BYTE *)(v5 + 1169) = v16;
      if ( (int)of_property_read_variable_u32_array(v6, "zte,ghost_check_multi_time", &v129, 1, 0) < 0 )
        v17 = 20;
      else
        v17 = v129;
      *(_BYTE *)(v5 + 1170) = v17;
      if ( (int)of_property_read_variable_u32_array(v6, "zte,ghost_check_single_count", &v129, 1, 0) < 0 )
        v18 = 5;
      else
        v18 = v129;
      *(_BYTE *)(v5 + 1171) = v18;
      if ( (int)of_property_read_variable_u32_array(v6, "zte,ghost_check_multi_count", &v129, 1, 0) < 0 )
        v19 = 8;
      else
        v19 = v129;
      *(_BYTE *)(v5 + 1172) = v19;
      if ( (int)of_property_read_variable_u32_array(v6, "zte,ghost_check_start_time", &v129, 1, 0) < 0 )
        v20 = 35;
      else
        v20 = v129;
      *(_BYTE *)(v5 + 1173) = v20;
      if ( (int)of_property_read_variable_u32_array(v6, "zte,ghost_check_ignore_id", &v129, 1, 0) < 0 )
        v21 = -1;
      else
        v21 = v129;
      *(_DWORD *)(v5 + 1176) = v21;
      if ( (int)of_property_read_variable_u32_array(v6, "zte,ghost_check_ignore_edge_area", &v129, 1, 0) < 0 )
        v22 = 5121;
      else
        v22 = v129;
      *(_DWORD *)(v5 + 1180) = v22;
      if ( (int)of_property_read_variable_u32_array(v6, "zte,ghost_check_ignore_corner_x", &v129, 1, 0) < 0 )
        v23 = 801;
      else
        v23 = v129;
      *(_DWORD *)(v5 + 1184) = v23;
      v24 = of_property_read_variable_u32_array(v6, "zte,ghost_check_ignore_corner_y", &v129, 1, 0);
      v25 = v129;
      v26 = *(unsigned __int8 *)(v5 + 1169);
      if ( v24 < 0 )
        v25 = 801;
    }
    else
    {
      v26 = 25;
      *(_BYTE *)(v5 + 1173) = 35;
      *(_DWORD *)(v5 + 1169) = 134550553;
      *(_QWORD *)(v5 + 1176) = 0xC81FFFFFFFFLL;
      v25 = 801;
      *(_DWORD *)(v5 + 1184) = 801;
    }
    *(_DWORD *)(v5 + 1188) = v25;
    printk(unk_333B9, v26, v15);
    printk(unk_35E38, *(unsigned __int8 *)(v5 + 1170), v27);
    printk(unk_333DE, *(unsigned __int8 *)(v5 + 1171), v28);
    printk(unk_33A77, *(unsigned __int8 *)(v5 + 1172), v29);
    printk(unk_396F5, *(unsigned __int8 *)(v5 + 1173), v30);
    printk(unk_38C31, *(unsigned int *)(v5 + 1176), v31);
    printk(unk_364EF, *(unsigned int *)(v5 + 1180), v32);
    printk(unk_31D70, *(unsigned int *)(v5 + 1184), v33);
    printk(unk_33A9C, *(unsigned int *)(v5 + 1188), v34);
    if ( (of_property_read_variable_u32_array(v6, "zte,tp_jitter_check", &v129, 1, 0) & 0x80000000) == 0 )
    {
      v119 = (unsigned __int8)v129;
      *(_BYTE *)(v5 + 36) = v129;
      printk(unk_3C972, v119, v35);
      if ( *(_BYTE *)(v5 + 36) )
      {
        if ( (of_property_read_variable_u32_array(v6, "zte,tp_jitter_timer", &v129, 1, 0) & 0x80000000) == 0 )
        {
          v121 = (unsigned __int16)v129;
          *(_WORD *)(v5 + 62) = v129;
          printk(unk_35E5C, v121, v120);
        }
      }
    }
    if ( (of_property_read_variable_u32_array(v6, "zte,tp_edge_click_suppression_pixel", &v129, 1, 0) & 0x80000000) == 0 )
    {
      v122 = (unsigned __int8)v129;
      *(_BYTE *)(v5 + 38) = v129;
      printk(unk_36E82, v122, v36);
      *(_DWORD *)(v5 + 39) = 16843009 * *(unsigned __int8 *)(v5 + 38);
    }
    v37 = of_find_property(v6, "zte,ufp_enable", 0);
    *(_BYTE *)(v5 + 28) = v37 != 0;
    if ( v37 )
    {
      printk(unk_38669, v38, v39);
      if ( (of_property_read_variable_u32_array(v6, "zte,ufp_circle_center_x", &v129, 1, 0) & 0x80000000) == 0 )
      {
        v127 = (unsigned __int16)v129;
        *(_WORD *)(v5 + 30) = v129;
        printk(unk_3749F, v127, v123);
      }
      if ( (of_property_read_variable_u32_array(v6, "zte,ufp_circle_center_y", &v129, 1, 0) & 0x80000000) == 0 )
      {
        v128 = (unsigned __int16)v129;
        *(_WORD *)(v5 + 32) = v129;
        printk(unk_32DBE, v128, v124);
      }
      if ( (of_property_read_variable_u32_array(v6, "zte,ufp_circle_radius", &v129, 1, 0) & 0x80000000) == 0 )
      {
        v126 = (unsigned __int16)v129;
        *(_WORD *)(v5 + 34) = v129;
        printk(unk_32889, v126, v125);
      }
    }
    _mutex_init(v5 + 2728, "&ztp_dev->cmd_mutex", &zte_touch_probe___key);
    _mutex_init(v5 + 2776, "&ztp_dev->report_mutex", &zte_touch_probe___key_92);
    _mutex_init(v5 + 2824, "&ztp_dev->tp_resume_mutex", &zte_touch_probe___key_94);
    v40 = proc_mkdir("touchscreen", 0);
    tpd_proc_dir = v40;
    if ( v40 )
    {
      if ( !proc_create("ts_information", 436, v40, &proc_ops_tp_module_Info) )
        printk(unk_3652D, v42, v43);
      if ( !proc_create("wake_gesture", 436, tpd_proc_dir, &proc_ops_wake_gesture) )
        printk(unk_34C27, v44, v45);
      if ( !proc_create("smart_cover", 436, tpd_proc_dir, &proc_ops_smart_cover) )
        printk(unk_380C2, v46, v47);
      if ( !proc_create("glove_mode", 436, tpd_proc_dir, &proc_ops_glove) )
        printk(unk_3C3D0, v48, v49);
      if ( !proc_create("FW_upgrade", 436, tpd_proc_dir, &proc_ops_tpfwupgrade) )
        printk(unk_32358, v50, v51);
      if ( !proc_create("suspend", 436, tpd_proc_dir, &proc_ops_suspend) )
        printk(unk_391A8, v52, v53);
      if ( !proc_create("headset_state", 436, tpd_proc_dir, &proc_ops_headset_state) )
        printk(unk_391C7, v54, v55);
      if ( !proc_create("rotation_limit_level", 436, tpd_proc_dir, &proc_ops_rotation_limit_level) )
        printk(unk_31825, v56, v57);
      if ( !proc_create("mRotation", 436, tpd_proc_dir, &proc_ops_mrotation) )
        printk(unk_3B64C, v58, v59);
      if ( !proc_create("single_tap", 436, tpd_proc_dir, &proc_ops_single_tap) )
        printk(unk_3AF41, v60, v61);
      if ( !proc_create("single_aod", 436, tpd_proc_dir, &proc_ops_single_aod) )
        printk(unk_369C3, v62, v63);
      if ( !proc_create("single_game", 436, tpd_proc_dir, &proc_ops_single_game) )
        printk(unk_38683, v64, v65);
      if ( !proc_create("get_noise", 436, tpd_proc_dir, &proc_ops_get_noise) )
        printk(unk_374D6, v66, v67);
      if ( !proc_create("edge_report_limit", 436, tpd_proc_dir, &proc_ops_edge_report_limit) )
        printk(unk_39719, v68, v69);
      if ( !proc_create("one_key", 436, tpd_proc_dir, &proc_ops_onekey) )
        printk(unk_34042, v70, v71);
      if ( !proc_create("play_game", 436, tpd_proc_dir, &proc_ops_playgame) )
        printk(unk_352C1, v72, v73);
      if ( !proc_create("tp_report_rate", 436, tpd_proc_dir, &proc_ops_tp_report_rate) )
        printk(unk_35E79, v74, v75);
      if ( !proc_create("follow_hand_level", 436, tpd_proc_dir, &proc_ops_follow_hand_level) )
        printk(unk_3C9A7, v76, v77);
      if ( !proc_create("stability_level", 436, tpd_proc_dir, &proc_ops_stability_level) )
        printk(unk_38C54, v78, v79);
      if ( !proc_create("sensibility", 436, tpd_proc_dir, &proc_ops_sensibility_level) )
        printk(unk_358DB, v80, v81);
      if ( !proc_create("game_partition", 436, tpd_proc_dir, &proc_ops_game_partition) )
        printk(unk_3C3F2, v82, v83);
      if ( !proc_create("pen_only", 436, tpd_proc_dir, &proc_ops_pen_only) )
        printk(unk_3CEFC, v84, v85);
      if ( !proc_create("finger_lock_flag", 436, tpd_proc_dir, &proc_ops_finger_lock_flag) )
        printk(unk_37BE2, v86, v87);
      if ( !proc_create("tp_self_test", 436, tpd_proc_dir, &proc_ops_tp_self_test) )
        printk(unk_37C0A, v88, v89);
      if ( !proc_create("tp_palm_mode", 436, tpd_proc_dir, &proc_ops_palm_mode) )
        printk(unk_34C4B, v90, v91);
      if ( !proc_create("fold_state", 436, tpd_proc_dir, &proc_ops_fold_state) )
        printk(unk_33AE9, v92, v93);
      if ( !proc_create("fake_sleep", 436, tpd_proc_dir, &proc_ops_fake_sleep) )
        printk(unk_38C7B, v94, v95);
      if ( !proc_create("screen_off_awake", 436, tpd_proc_dir, &proc_ops_screen_off_awake) )
        printk(unk_386A6, v96, v97);
      if ( !proc_create("ghost_debug", 436, tpd_proc_dir, &proc_ops_ghost_debug) )
        printk(unk_3A104, v98, v99);
      if ( !proc_create("BBAT_test", 436, tpd_proc_dir, &proc_ops_BBAT_test) )
        printk(unk_36EAF, v100, v101);
      if ( !proc_create("tp_test", 436, tpd_proc_dir, &proc_ops_tp_test) )
        printk(unk_36553, v102, v103);
      if ( !proc_create("frame_data", 436, tpd_proc_dir, &proc_ops_frame_data) )
        printk(unk_33B0B, v104, v105);
    }
    else
    {
      printk(unk_3A0E3, "create_tpd_proc_entry", v41);
    }
    v106 = tpd_cdev;
    v107 = *(_QWORD *)(tpd_cdev + 3096);
    if ( v107 )
    {
      v108 = kobject_create_and_add("fwupdate", &((struct platform_device *)v107)->dev.kobj);
      *(_QWORD *)(v106 + 2712) = v108;
      if ( v108 )
      {
        bin_file = sysfs_create_bin_file(v108, &fwimage_attr);
        if ( bin_file )
        {
          printk(unk_3537D, bin_file, v112);
          kobject_put(*(_QWORD *)(v106 + 2712));
        }
      }
      else
      {
        printk(unk_334BC, v109, v110);
      }
    }
    else
    {
      printk(unk_36F15, v104, v105);
    }
    v113 = tpd_clean_all_event();
    v114 = ghost_check_reset(v113);
    v115 = ufp_mac_init(v114);
    if ( !(unsigned int)tpd_workqueue_init(v115, v116, v117) )
    {
      queue_delayed_work_on(32, *(_QWORD *)(v5 + 1200), v5 + 2256, 375);
      *(_BYTE *)(v5 + 29) = 0;
      *(_BYTE *)(v5 + 2632) = -1;
      *(_DWORD *)(v5 + 1192) = 0;
      *(_DWORD *)(v5 + 3112) = 0;
      _init_swait_queue_head(v5 + 3120, "&x->wait", &init_completion___key);
      *(_BYTE *)(v5 + 3145) = 0;
      printk(unk_38656, "zte_touch_probe", 2897);
      result = 0;
      goto LABEL_119;
    }
  }
  else
  {
    printk(unk_3C945, v3, v4);
  }
  result = 4294967284LL;
LABEL_119:
  _ReadStatusReg(SP_EL0);
  return result;
}
