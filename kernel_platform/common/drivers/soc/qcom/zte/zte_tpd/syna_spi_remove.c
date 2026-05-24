__int64 __fastcall syna_spi_remove(__int64 a1, __int64 a2, __int64 a3)
{
  __int64 result; // x0
  __int64 v4; // x0
  __int64 v5; // x0
  __int64 v6; // x1
  __int64 v7; // x0

  if ( dword_C88 >= 1 )
    gpio_free((unsigned int)dword_C88, a2);
  if ( dword_CD0 >= 1 )
    gpio_free((unsigned int)dword_CD0, a2);
  result = (unsigned int)dword_C80;
  if ( dword_C80 >= 1 )
    result = gpio_free((unsigned int)dword_C80, a2);
  if ( rx_buf )
  {
    if ( p_device && (v4 = syna_request_managed_device()) != 0 )
      result = devm_kfree(v4, rx_buf);
    else
      result = printk(unk_3BE43, "syna_pal_mem_free", a3);
    rx_buf = 0;
  }
  if ( tx_buf )
  {
    if ( p_device && (v5 = syna_request_managed_device()) != 0 )
      result = devm_kfree(v5, tx_buf);
    else
      result = printk(unk_3BE43, "syna_pal_mem_free", a3);
    tx_buf = 0;
  }
  v6 = xfer;
  if ( xfer )
  {
    if ( p_device && (v7 = syna_request_managed_device()) != 0 )
      result = devm_kfree(v7, xfer);
    else
      result = printk(unk_3BE43, "syna_pal_mem_free", a3);
    xfer = 0;
  }
  if ( dword_D08 == 1 )
  {
    result = qword_D18;
    if ( qword_D18 )
      result = devm_regulator_put(qword_D18, v6);
  }
  else if ( dword_D08 >= 1 )
  {
    result = (unsigned int)dword_D20;
    if ( dword_D20 >= 1 )
      result = gpio_free((unsigned int)dword_D20, v6);
  }
  if ( dword_CE0 == 1 )
  {
    result = qword_CF0;
    if ( qword_CF0 )
      return devm_regulator_put(qword_CF0, v6);
  }
  else if ( dword_CE0 >= 1 )
  {
    result = (unsigned int)dword_CF8;
    if ( dword_CF8 >= 1 )
      return gpio_free((unsigned int)dword_CF8, v6);
  }
  return result;
}
