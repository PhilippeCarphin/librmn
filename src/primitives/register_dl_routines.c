#include <dlfcn.h>

void DlRegister(void *open, void *sym, void *error, void *close);

void register_dl_routines()
{
 DlRegister(dlopen,dlsym,dlerror,dlclose);
}

void register_dl_routines_(){register_dl_routines();}
