
#include "types.h"
#include "param.h"
#include "riscv.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "defs.h"


struct
{
  struct spinlock lock;
  uint64 seed;
} psdurandom;

struct 
{
  struct spinlock lock;
  uint64 written;
} psdnullstat;


int psddevicewrite(short minor, int user_src, uint64 src, int n)
{
  switch (minor)
  {
  case PSD_NULL:
    return n;

  case PSD_ZERO:
    return -1;

  case PSD_URANDOM:
    uint64 new_seed;

    if (n != sizeof(psdurandom.seed)) return -1;
    if(either_copyin(&new_seed, user_src, src, n) == -1) return -1;

    acquire(&psdurandom.lock);
    psdurandom.seed = new_seed;
    release(&psdurandom.lock);

    return n;

  case PSD_NULLSTAT:
    acquire(&psdnullstat.lock);
    psdnullstat.written += n;
    release(&psdnullstat.lock);

    return n;
  
  default:
    return -1;
  }
}

int psddeviceread(short minor, int user_dst, uint64 dst, int n)
{
  uint8 buf[64];
  int written = 0;
  switch (minor)
  {
    case PSD_NULL:
      return 0;

    case PSD_ZERO:
      memset(buf, 0, sizeof(buf));
      while (n - written > 0)
      {
        if (n - written <= sizeof(buf))
        {
          if(either_copyout(user_dst, dst + written, buf, n - written) == -1) return -1;
          written = n;
        }
        else
        {
          if(either_copyout(user_dst, dst + written, buf, sizeof(buf)) == -1) return -1;
          written += sizeof(buf);
        }
        
      }
      
      return n;

    case PSD_URANDOM:
      uint64 a = 2862933555777941757;
      uint64 c = 3037000493;

      acquire(&psdurandom.lock);
      uint64 new_seed = psdurandom.seed;

      int ind = 0;

      for (int i = 0; i < n; ++i)
      {
        new_seed = new_seed * a + c;
        uint8 byte = new_seed & 0xFF;
        if (ind < sizeof(buf)) 
        {
          buf[ind] = byte;
          ++ind;
        }
        else
        {
          psdurandom.seed = new_seed;
          release(&psdurandom.lock);
          
          if(either_copyout(user_dst, dst + written, buf, sizeof(buf)) == -1) return -1;
          written += sizeof(buf);
          buf[0] = byte;
          ind = 1;

          acquire(&psdurandom.lock);
        }
        
      }
      psdurandom.seed = new_seed;
      release(&psdurandom.lock);

      if (written < n)
      {
        if(either_copyout(user_dst, dst + written, buf, ind) == -1) return -1;
      }

      
      return n;

    case PSD_NULLSTAT:
      if (n != sizeof(psdnullstat.written)) return -1;

      acquire(&psdnullstat.lock);
      uint64 cnt = psdnullstat.written;
      release(&psdnullstat.lock);

      if(either_copyout(user_dst, dst, &cnt, sizeof(cnt)) == -1) return -1;      

      return sizeof(cnt);

    default:
      return -1;
  }
}

void psddeviceinit(void)
{
  initlock(&psdurandom.lock, "psdurandom");
  initlock(&psdnullstat.lock, "psdnullstat");

  psdurandom.seed = 23;
  psdnullstat.written = 0;

  devsw[PSD_DEVICE].read = psddeviceread;
  devsw[PSD_DEVICE].write = psddevicewrite;
}
