static unsigned int syna_spi_alloc_mem_xfer_count = 0;

__int64 __fastcall syna_spi_alloc_mem(unsigned int a1, unsigned int a2, __int64 a3)
{
  __int64 v5; // x0
  __int64 v6; // x8
  __int64 v7; // x0
  __int64 v8; // x2
  __int64 v9; // x0
  __int64 v10; // x0
  __int64 v11; // x0
  __int64 v12; // x2
  __int64 v13; // x0
  void *v15; // x0
  void *v16; // x0
  void *v17; // x0

  if ( syna_spi_alloc_mem_xfer_count >= a1 )
  {
    memset((void *)xfer, 0, 136 * a1);
LABEL_12:
    if ( buf_size >= a2 )
      return 0;
    if ( rx_buf )
    {
      if ( p_device && (v9 = syna_request_managed_device()) != 0 )
        devm_kfree(v9, rx_buf);
      else
        printk(unk_3BE43, "syna_pal_mem_free", v8);
      rx_buf = 0;
    }
    if ( tx_buf )
    {
      if ( p_device && (v10 = syna_request_managed_device()) != 0 )
        devm_kfree(v10, tx_buf);
      else
        printk(unk_3BE43, "syna_pal_mem_free", v8);
      tx_buf = 0;
    }
    if ( p_device && (v11 = syna_request_managed_device()) != 0 )
    {
      if ( (int)a2 > 0 )
      {
        rx_buf = devm_kmalloc(v11, a2, 3520);
        if ( rx_buf )
        {
          if ( p_device && (v13 = syna_request_managed_device()) != 0 )
          {
            tx_buf = devm_kmalloc(v13, a2, 3520);
            if ( tx_buf )
            {
              buf_size = a2;
              return 0;
            }
          }
          else
          {
            printk(unk_3BE43, "syna_pal_mem_alloc", v12);
            tx_buf = 0;
          }
          v17 = unk_33731;
          goto LABEL_45;
        }
LABEL_44:
        v17 = unk_39E02;
LABEL_45:
        printk(v17, "syna_spi_alloc_mem", v12);
        buf_size = 0;
        return 4294967284LL;
      }
      v16 = unk_38286;
    }
    else
    {
      v16 = unk_3BE43;
    }
    printk(v16, "syna_pal_mem_alloc", v8);
    rx_buf = 0;
    goto LABEL_44;
  }
  if ( p_device && (v5 = syna_request_managed_device()) != 0 )
  {
    if ( xfer )
      devm_kfree(v5, xfer);
    v6 = p_device;
    if ( !p_device )
      goto LABEL_33;
  }
  else
  {
    printk(unk_3BE43, "syna_pal_mem_free", a3);
    v6 = p_device;
    if ( !p_device )
      goto LABEL_33;
  }
  v7 = syna_request_managed_device();
  if ( !v7 )
  {
LABEL_33:
    v15 = unk_3BE43;
LABEL_40:
    printk(v15, "syna_pal_mem_alloc", a3);
    xfer = 0;
    goto LABEL_41;
  }
  if ( (int)(136 * a1) <= 0 )
  {
    v15 = unk_38286;
    goto LABEL_40;
  }
  xfer = devm_kmalloc(v7, 136LL * a1, 3520);
  if ( xfer )
  {
    syna_spi_alloc_mem_xfer_count = a1;
    goto LABEL_12;
  }
LABEL_41:
  printk(unk_3490A, "syna_spi_alloc_mem", v8);
  syna_spi_alloc_mem_xfer_count = 0;
  return 4294967284LL;
}
