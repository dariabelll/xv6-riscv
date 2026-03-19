#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/procinfo.h"
#include "user/user.h"

int main()
{
  int lim = ps_listinfo(0, 0);
  int count;
  struct procinfo *buffer;

  if (lim < 0)
  {
    fprintf(2, "ps_listinfo failed\n");
    exit(1);
  }


  while (1) 
  {
    buffer = malloc(lim * sizeof(struct procinfo));
    if (buffer == 0)
    {
      fprintf(2, "malloc failed\n");
      exit(1);
    }

    count = ps_listinfo(buffer, lim);
    if (count < 0)
    {
      fprintf(2, "ps_listinfo failed\n");
      free(buffer);
      exit(1);
    }

    if (count > lim)
    {
      free(buffer);
      lim = count;
      continue;
    }

    break;
  }

  printf("pid\tname\t\tstate\t\tparent_pid\tparent_name\n");

  for (int i = 0; i < count; ++i)
  {
    char *state = "UNKNOWN";
    char *parent_name = "-";

    switch (buffer[i].state)
    {
        case 1:
            state = "USED";
            break;
        case 2:
            state = "SLEEPING";
            break;
        case 3:
            state = "RUNNABLE";
            break;
        case 4:
            state = "RUNNING";
            break;
        case 5:
            state = "ZOMBIE";
            break;
    }

    if (buffer[i].parent_id != 0)
    {
      for (int j = 0; j < count; ++j) 
      {
        if (buffer[j].pid == buffer[i].parent_id)
        {
          parent_name = buffer[j].name;
          break;
        }
      }
    }

    printf("%d\t%s\t\t%s\t\t%d\t%s\n", buffer[i].pid, buffer[i].name, state, buffer[i].parent_id, parent_name);
  }

  free(buffer);
  exit(0);
}