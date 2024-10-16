#include "440hz.c"
#include "880hz.c"


struct sample_list_s
{
    const unsigned char *data;
    int len;
} ;

struct sample_list_s sample_list[] =
{
    {_ac440hz, sizeof(_ac440hz)},
    {_ac880hz, sizeof(_ac880hz)},
};
