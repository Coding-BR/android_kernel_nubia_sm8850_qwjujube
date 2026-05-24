#include <linux/platform_device.h>
__int64 syna_request_managed_device()
{
  if ( p_device )
    return (__int64)&((struct platform_device *)p_device)->dev;
  else
    return 0;
}
