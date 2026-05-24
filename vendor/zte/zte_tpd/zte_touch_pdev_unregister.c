void zte_touch_pdev_unregister()
{
  __int64 v0;

  v0 = tpd_cdev;
  if ( !*(_QWORD *)(tpd_cdev + 3096) )
  {
    gpio_free(16);
    platform_device_unregister(*(_QWORD *)(v0 + 3096));
  }
}
