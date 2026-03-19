#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/procinfo.h"
#include "user/user.h"

int main()
{
  int tests = 0;
  int passed = 0;

  int proc_count = -1;
  int result = -1;


  tests++;
  proc_count = ps_listinfo(0, 0);
  if(proc_count >= 0)
  {
    passed++;
    printf("count_only: ok (returned %d)\n", proc_count);
  } 
  else 
  {
    fprintf(2, "count_only: fail (returned %d)\n", proc_count);
  }

  tests++;
  result = ps_listinfo(0, -1);
  if(result < 0)
  {
    passed++;
    printf("negative_limit: ok (returned %d)\n", result);
  }
  else
  {
    fprintf(2, "negative_limit: fail (returned %d)\n", result);
  }

  if(proc_count >= 0)
  {
    tests++;
    int lim_ok = proc_count + 4;
    struct procinfo *buf_ok = malloc(lim_ok * sizeof(struct procinfo));

    if(buf_ok == 0)
    {
      fprintf(2, "normal_buffer: fail (malloc failed)\n");
    } 
    else 
    {
      result = ps_listinfo(buf_ok, lim_ok);

      if(result >= 0 && result <= lim_ok)
      {
        passed++;
        printf("normal_buffer: ok (lim=%d, returned %d)\n", lim_ok, result);
      } 
      else 
      {
        fprintf(2, "normal_buffer: fail (lim=%d, returned %d)\n", lim_ok, result);
      }

      free(buf_ok);
    }


    tests++;
    int lim_small = (proc_count > 1) ? (proc_count - 1) : 1;
    struct procinfo *buf_small = malloc(lim_small * sizeof(struct procinfo));

    if(buf_small == 0)
    {
      fprintf(2, "small_buffer: fail (malloc failed)\n");
    } 
    else 
    {
      result = ps_listinfo(buf_small, lim_small);

      if(result > lim_small)
      {
        passed++;
        printf("small_buffer: ok (lim=%d, returned %d)\n", lim_small, result);
      } 
      else 
      {
        fprintf(2, "small_buffer: fail (lim=%d, returned %d)\n", lim_small, result);
      }

      free(buf_small);
    }


    tests++;
    struct procinfo *buf_zero = malloc(sizeof(struct procinfo));
    if(buf_zero == 0)
    {
      fprintf(2, "zero_limit_with_buffer: fail (malloc failed)\n");
    }
    else
    {
      result = ps_listinfo(buf_zero, 0);

      if(proc_count == 0)
      {
        if(result == 0)
        {
          passed++;
          printf("zero_limit_with_buffer: ok (returned %d)\n", result);
        }
        else
        {
          fprintf(2, "zero_limit_with_buffer: fail (returned %d)\n", result);
        }
      }
      else
      {
        if(result > 0)
        {
          passed++;
          printf("zero_limit_with_buffer: ok (returned %d)\n", result);
        }
        else
        {
          fprintf(2, "zero_limit_with_buffer: fail (returned %d)\n", result);
        }
      }

      free(buf_zero);
    }


    tests++;
    result = ps_listinfo((struct procinfo *)0xffffffffffffULL, 4);

    if(result < 0)
    {
      passed++;
      printf("bad_address: ok (returned %d)\n", result);
    } 
    else 
    {
      fprintf(2, "bad_address: fail (returned %d)\n", result);
    }
  }

  printf("passed %d/%d tests\n", passed, tests);
  exit(0);
}
