__int64 __fastcall syna_hw_interface_init(__int64 a1, __int64 a2, __int64 a3)
{
  unsigned int v3; // w0
  __int64 v4; // x2
  unsigned int v5; // w19
  void *v6; // x0

  printk(unk_348D8, "syna_hw_interface_init", a3);
  syna_board_data_global = &syna_spi_hw_if;
  v3 = platform_device_register(&syna_spi_device);
  if ( (v3 & 0x80000000) != 0 )
  {
    v5 = v3;
    v6 = unk_336E1;
  }
  else
  {
    p_device = (__int64)&syna_spi_device;
    v5 = _spi_register_driver(&_this_module, &syna_spi_driver);
    if ( (v5 & 0x80000000) == 0 )
    {
      printk(unk_3C561, "syna_hw_interface_init", v4);
      buf_size = 0;
      rx_buf = 0;
      tx_buf = 0;
      return v5;
    }
    v6 = unk_32A1D;
  }
  printk(v6, "syna_hw_interface_init", v4);
  return v5;
}
