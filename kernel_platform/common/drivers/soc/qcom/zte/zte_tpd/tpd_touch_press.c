__int64 __fastcall tpd_touch_press(
        __int64 a1,
        unsigned int a2,
        unsigned int a3,
        unsigned int a4,
        unsigned int a5,
        unsigned int a6)
{
  int v6; // w8
  __int64 v7; // x27
  int v8; // w10
  int v9; // w9
  unsigned int v10; // w19
  unsigned int v11; // w21
  __int64 v16; // x26
  unsigned int *v17; // x28
  int v18; // w8
  unsigned int v19; // w8
  signed int v20; // w9
  unsigned int v21; // w10
  unsigned int v22; // w9
  __int64 result; // x0
  int v24; // w10
  unsigned int v25; // w8
  int v26; // w10
  unsigned int v27; // w8
  int v28; // w8
  int v29; // w8
  int v30; // w8
  unsigned __int64 v31; // x20
  __int64 v32; // x1
  __int64 v33; // x2
  __int64 v34; // x0
  int v35; // w9
  int v36; // w8
  int v37; // w9
  __int64 v38; // x9
  __int64 v39; // x9
  __int64 v40; // x2
  void *v41; // x0
  __int64 v42; // x8
  __int64 v43; // x1
  __int64 v44; // x8
  int v45; // w20
  int v46; // w8
  unsigned int v47; // w8
  int v48; // w9
  unsigned int v49; // [xsp+Ch] [xbp-4h]

  v6 = (unsigned __int16)a2;
  v7 = tpd_cdev;
  v8 = (unsigned __int16)a3;
  if ( (a2 & 0xFFFE) == 0 )
    v6 = 1;
  v9 = *(_DWORD *)(tpd_cdev + 16);
  if ( (a3 & 0xFFFE) == 0 )
    v8 = 1;
  if ( v9 )
    v10 = v8;
  else
    v10 = a3;
  if ( v9 )
    v11 = v6;
  else
    v11 = a2;
  if ( !a1 || (unsigned __int16)a4 >= 0xAu )
    return printk(unk_3A160, "tpd_touch_press", (unsigned __int16)a4);
  v49 = (unsigned __int16)a4;
  v16 = 136LL * (unsigned __int16)a4;
  v17 = (unsigned int *)((char *)&point_report_info + v16);
  v18 = *((unsigned __int8 *)&point_report_info + v16 + 80);
  *(_QWORD *)((char *)&point_report_info + v16 + 128) = a1;
  if ( v18 == 1 )
  {
    if ( !*(_BYTE *)(v7 + 37) || (*((_BYTE *)v17 + 89) & 1) != 0 || (*(_DWORD *)(v7 + 16) | 2) == 3 )
      goto LABEL_89;
    v19 = *(unsigned __int16 *)(v7 + 1090);
    v20 = v19 >> 2;
    if ( v19 >> 2 < (unsigned __int16)v11 && (unsigned __int16)v11 < (3 * v19) >> 2 )
    {
      *((_BYTE *)v17 + 89) = 1;
      v19 = *(unsigned __int16 *)(v7 + 1090);
      v20 = v19 >> 2;
    }
    v21 = v17[6];
    if ( (int)v21 >= v20 && v21 <= (3 * v19) >> 2 )
      goto LABEL_89;
    if ( !*(_BYTE *)(v7 + 37) )
      goto LABEL_124;
    if ( (*(_DWORD *)(v7 + 16) | 2) == 3 )
    {
      if ( *(unsigned __int8 *)(v7 + 52) > (unsigned int)(unsigned __int16)v11
        || (int)(v19 - *(unsigned __int8 *)(v7 + 53)) < (unsigned __int16)v11
        || *(unsigned __int8 *)(v7 + 54) > (unsigned int)(unsigned __int16)v10
        || *(unsigned __int16 *)(v7 + 1092) - *(unsigned __int8 *)(v7 + 55) < (unsigned __int16)v10 )
      {
        goto LABEL_125;
      }
      if ( !*(_BYTE *)(v7 + 56) || *(unsigned __int16 *)(v7 + 46) >= (unsigned int)(unsigned __int16)v10 )
        goto LABEL_124;
      v22 = *(unsigned __int16 *)(v7 + 44);
      if ( v22 > (unsigned __int16)v11 )
      {
LABEL_125:
        if ( *((_BYTE *)v17 + 88) == 1 )
        {
          v44 = *((_QWORD *)v17 + 13);
          v45 = (unsigned __int16)v11;
        }
        else
        {
          v45 = (unsigned __int16)v11;
          v17[12] = (unsigned __int16)v11;
          v17[13] = (unsigned __int16)v10;
          *((_BYTE *)v17 + 88) = 1;
          v44 = jiffies;
          *((_QWORD *)v17 + 13) = jiffies;
        }
        if ( (unsigned int)jiffies_to_msecs(jiffies - v44) >= 0x321 )
        {
          v46 = v17[12] - v45;
          if ( v46 < 0 )
            v46 = v45 - v17[12];
          if ( v46 < 21 )
          {
            v47 = (unsigned __int16)v10;
            v48 = v17[13] - (unsigned __int16)v10;
            if ( v48 < 0 )
              v48 = (unsigned __int16)v10 - v17[13];
            if ( v48 < 21 )
            {
              result = tpd_touch_release(*((_QWORD *)v17 + 16), a4, 0);
              *((_BYTE *)v17 + 88) = 0;
              return result;
            }
          }
          else
          {
            v47 = (unsigned __int16)v10;
          }
          v17[12] = v45;
          v17[13] = v47;
          *((_QWORD *)v17 + 13) = jiffies;
        }
LABEL_89:
        v30 = *((unsigned __int8 *)v17 + 82);
        *((_BYTE *)v17 + 80) = 1;
        if ( v30 == 1 )
        {
          v31 = v16 | 4;
          LODWORD(v16) = a6;
          if ( (*((_BYTE *)v17 + 83) & 1) != 0 )
          {
            if ( v31 <= 0x550 )
              goto LABEL_92;
          }
          else
          {
            *((_BYTE *)v17 + 83) = 1;
            v38 = jiffies;
            *((_BYTE *)v17 + 90) = 1;
            *((_QWORD *)v17 + 12) = v38;
            ++*(_DWORD *)(v7 + 1164);
            v17[6] = *v17;
            if ( v31 <= 0x550 )
            {
              v17[7] = v17[1];
              point_report_reset(v49);
              printk(unk_37C4E, v49, *v17);
LABEL_92:
              tpd_touch_report(a1, *v17, v17[1], a4, a5, a6);
              input_mt_sync_frame(a1);
              input_event(a1, 0, 0, 0);
              usleep_range_state(1000, 1500, 2);
              if ( (*((_BYTE *)v17 + 83) & 1) != 0 )
              {
LABEL_93:
                v32 = v17[3];
                v33 = v17[4];
                v34 = a1;
LABEL_109:
                result = tpd_touch_report(v34, v32, v33, a4, a5, (unsigned int)v16);
LABEL_110:
                *((_BYTE *)v17 + 82) = 0;
                v17[9] = (unsigned __int16)v11;
                v17[10] = (unsigned __int16)v10;
                return result;
              }
LABEL_115:
              *((_BYTE *)v17 + 83) = 1;
              v39 = jiffies;
              *((_BYTE *)v17 + 90) = 1;
              *((_QWORD *)v17 + 12) = v39;
              ++*(_DWORD *)(v7 + 1164);
              LODWORD(v39) = v17[1];
              v17[6] = *v17;
              v17[7] = v39;
              point_report_reset(v49);
              printk(unk_37C4E, v49, v17[3]);
              goto LABEL_93;
            }
          }
LABEL_114:
          __break(1u);
          goto LABEL_115;
        }
        LODWORD(v16) = a6;
        if ( *(_BYTE *)(v7 + 36) )
        {
          if ( (*((_BYTE *)v17 + 83) & 1) != 0 )
          {
            if ( *((_BYTE *)v17 + 87) == 1 )
            {
              result = jiffies_to_msecs(jiffies - *((_QWORD *)v17 + 12));
              if ( (int)(v17[6] - (unsigned __int16)v11) >= 0 )
                v35 = v17[6] - (unsigned __int16)v11;
              else
                v35 = (unsigned __int16)v11 - v17[6];
              v36 = *(unsigned __int8 *)(v7 + 36);
              if ( (unsigned int)result < 0x65 )
                v36 *= 3;
              if ( v35 > v36 )
                goto LABEL_106;
              v37 = v17[7] - (unsigned __int16)v10;
              if ( v37 < 0 )
                v37 = (unsigned __int16)v10 - v17[7];
              if ( v37 > v36 )
              {
LABEL_106:
                result = tpd_touch_report(a1, v11, v10, a4, a5, a6);
                *((_BYTE *)v17 + 87) = 0;
              }
              goto LABEL_110;
            }
            goto LABEL_108;
          }
          *((_BYTE *)v17 + 83) = 1;
          ++*(_DWORD *)(v7 + 1164);
          point_report_reset(v49);
          v40 = (unsigned __int16)v11;
          v17[6] = (unsigned __int16)v11;
          v17[7] = (unsigned __int16)v10;
          v41 = unk_3C42D;
          v42 = jiffies;
          *((_BYTE *)v17 + 87) = 1;
          v43 = v49;
        }
        else
        {
          if ( (*((_BYTE *)v17 + 83) & 1) != 0 )
          {
LABEL_108:
            v34 = a1;
            v32 = v11;
            v33 = v10;
            goto LABEL_109;
          }
          *((_BYTE *)v17 + 83) = 1;
          ++*(_DWORD *)(v7 + 1164);
          v17[6] = (unsigned __int16)v11;
          v17[7] = (unsigned __int16)v10;
          point_report_reset(v49);
          v41 = unk_3A78C;
          v42 = jiffies;
          v43 = v49;
          v40 = (unsigned __int16)v11;
          LODWORD(v16) = a6;
        }
        *((_QWORD *)v17 + 12) = v42;
        printk(v41, v43, v40);
        goto LABEL_108;
      }
    }
    else
    {
      if ( *(unsigned __int8 *)(v7 + 52) > (unsigned int)(unsigned __int16)v11 )
        goto LABEL_125;
      v22 = *(unsigned __int8 *)(v7 + 53);
    }
    if ( (int)(v19 - v22) < (unsigned __int16)v11 )
      goto LABEL_125;
LABEL_124:
    *((_BYTE *)v17 + 88) = 0;
    goto LABEL_89;
  }
  if ( (point_is_in_limit_area(v11, v10) & 1) == 0 )
  {
    if ( *(_BYTE *)(tpd_cdev + 37) )
    {
      if ( (*(_DWORD *)(tpd_cdev + 16) | 2) == 3 )
      {
        if ( (unsigned __int16)v11 < (unsigned int)*(unsigned __int8 *)(tpd_cdev + 52) )
          goto LABEL_33;
        v26 = *(unsigned __int16 *)(tpd_cdev + 1090);
        if ( v26 - *(unsigned __int8 *)(tpd_cdev + 53) < (unsigned __int16)v11 )
          goto LABEL_33;
        if ( *(unsigned __int8 *)(tpd_cdev + 54) > (unsigned int)(unsigned __int16)v10 )
          goto LABEL_33;
        if ( *(unsigned __int16 *)(tpd_cdev + 1092) - *(unsigned __int8 *)(tpd_cdev + 55) < (unsigned __int16)v10 )
          goto LABEL_33;
        if ( *(_BYTE *)(tpd_cdev + 56) )
        {
          if ( *(unsigned __int16 *)(tpd_cdev + 46) < (unsigned int)(unsigned __int16)v10 )
          {
            v27 = *(unsigned __int16 *)(tpd_cdev + 44);
            if ( v27 > (unsigned __int16)v11 || (int)(v26 - v27) < (unsigned __int16)v11 )
              goto LABEL_33;
          }
        }
      }
      else if ( *(unsigned __int8 *)(tpd_cdev + 52) > (unsigned int)(unsigned __int16)v11
             || *(unsigned __int16 *)(tpd_cdev + 1090) - *(unsigned __int8 *)(tpd_cdev + 53) < (unsigned __int16)v11 )
      {
        goto LABEL_33;
      }
    }
LABEL_86:
    if ( (*((_BYTE *)v17 + 81) & 1) != 0 )
    {
      *((_BYTE *)v17 + 20) = a5;
      v17[3] = (unsigned __int16)v11;
      v17[4] = (unsigned __int16)v10;
      *((_BYTE *)v17 + 21) = a6;
      *((_BYTE *)v17 + 82) = 1;
    }
    else
    {
      *((_BYTE *)v17 + 82) = 0;
    }
    goto LABEL_89;
  }
LABEL_33:
  result = point_is_in_limit_area(v11, v10);
  if ( (result & 1) != 0 || !*(_BYTE *)(tpd_cdev + 37) )
    goto LABEL_77;
  if ( (*(_DWORD *)(tpd_cdev + 16) | 2) == 3 )
  {
    if ( (unsigned __int16)v11 < (unsigned int)*(unsigned __int8 *)(tpd_cdev + 52) )
      goto LABEL_62;
    v24 = *(unsigned __int16 *)(tpd_cdev + 1090);
    if ( v24 - *(unsigned __int8 *)(tpd_cdev + 53) < (unsigned __int16)v11 )
      goto LABEL_62;
    if ( *(unsigned __int8 *)(tpd_cdev + 54) > (unsigned int)(unsigned __int16)v10 )
      goto LABEL_62;
    if ( *(unsigned __int16 *)(tpd_cdev + 1092) - *(unsigned __int8 *)(tpd_cdev + 55) < (unsigned __int16)v10 )
      goto LABEL_62;
    if ( *(_BYTE *)(tpd_cdev + 56) )
    {
      if ( *(unsigned __int16 *)(tpd_cdev + 46) < (unsigned int)(unsigned __int16)v10 )
      {
        v25 = *(unsigned __int16 *)(tpd_cdev + 44);
        if ( v25 > (unsigned __int16)v11 || (int)(v24 - v25) < (unsigned __int16)v11 )
          goto LABEL_62;
      }
    }
LABEL_77:
    if ( (*((_BYTE *)v17 + 85) & 1) == 0 )
    {
      *((_BYTE *)v17 + 85) = 1;
      result = printk(unk_36572, v49, (unsigned __int16)v11);
      if ( (*((_BYTE *)v17 + 81) & 1) != 0 )
        goto LABEL_79;
      goto LABEL_75;
    }
LABEL_78:
    if ( (*((_BYTE *)v17 + 81) & 1) != 0 )
      goto LABEL_79;
LABEL_75:
    *v17 = (unsigned __int16)v11;
    if ( (v16 | 4uLL) <= 0x550 )
    {
      *((_BYTE *)v17 + 8) = a5;
      v17[1] = (unsigned __int16)v10;
      *((_BYTE *)v17 + 9) = a6;
      *((_BYTE *)v17 + 81) = 1;
      return result;
    }
    goto LABEL_114;
  }
  if ( *(unsigned __int8 *)(tpd_cdev + 52) <= (unsigned int)(unsigned __int16)v11
    && *(unsigned __int16 *)(tpd_cdev + 1090) - *(unsigned __int8 *)(tpd_cdev + 53) >= (unsigned __int16)v11 )
  {
    goto LABEL_77;
  }
LABEL_62:
  if ( (*((_BYTE *)v17 + 85) & 1) == 0 )
  {
    *((_BYTE *)v17 + 85) = 1;
    *((_QWORD *)v17 + 14) = jiffies;
    result = printk(unk_3A806, v49, (unsigned __int16)v11);
  }
  if ( (byte_30EFB & 1) == 0
    && (byte_30F83 & 1) == 0
    && (byte_3100B & 1) == 0
    && (byte_31093 & 1) == 0
    && (byte_3111B & 1) == 0
    && (byte_311A3 & 1) == 0
    && (byte_3122B & 1) == 0
    && (byte_312B3 & 1) == 0
    && (byte_3133B & 1) == 0
    && byte_313C3 != 1 )
  {
    goto LABEL_78;
  }
  *((_BYTE *)v17 + 86) = 1;
  if ( (*((_BYTE *)v17 + 81) & 1) == 0 )
    goto LABEL_75;
LABEL_79:
  v28 = *v17 - (unsigned __int16)v11;
  if ( v28 < 0 )
    v28 = (unsigned __int16)v11 - *v17;
  if ( v28 > 100 )
    goto LABEL_86;
  if ( (v16 | 4uLL) > 0x550 )
    goto LABEL_114;
  v29 = v17[1] - (unsigned __int16)v10;
  if ( v29 < 0 )
    v29 = (unsigned __int16)v10 - v17[1];
  if ( v29 >= 101 )
    goto LABEL_86;
  return result;
}
