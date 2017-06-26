#include "ml.h"

_ML_C_HEAD_

typedef int uc_arch;
typedef int uc_err;
typedef int uc_mode;
typedef void* uc_engine;
typedef void* uc_hook;
typedef UINT64 uint64_t;
typedef UINT32 uint32_t;

unsigned int uc_version(unsigned int *major, unsigned int *minor)
{
    return 0;
}

bool uc_arch_supported(uc_arch arch)
{
    return 0;
}

uc_err uc_open(uc_arch arch, uc_mode mode, uc_engine **uc)
{
    return 0;
}

uc_err uc_close(uc_engine *uc)
{
    return 0;
}

uc_err uc_errno(uc_engine *uc)
{
    return 0;
}

const char* uc_strerror(uc_err code)
{
    return 0;
}

uc_err uc_reg_write(uc_engine *uc, int regid, const void *value)
{
    return 0;
}

uc_err uc_reg_read(uc_engine *uc, int regid, void *value)
{
    return 0;
}

uc_err __cdecl uc_reg_read_batch(uc_engine *uc, int *regs, void **vals, int count)
{
    return 0;
}

uc_err __cdecl uc_reg_write_batch(uc_engine *uc, int *regs, void *const *vals, int count)
{
    return 0;
}

uc_err uc_mem_write(uc_engine *uc, uint64_t address, const void *bytes, size_t size)
{
    return 0;
}

uc_err uc_mem_read(uc_engine *uc, uint64_t address, void *bytes, size_t size)
{
    return 0;
}

uc_err uc_emu_start(uc_engine *uc, uint64_t begin, uint64_t until, uint64_t timeout, size_t count)
{
    return 0;
}

uc_err uc_emu_stop(uc_engine *uc)
{
    return 0;
}

uc_err uc_hook_add(uc_engine *uc, uc_hook *hh, int type, void *callback, void *user_data, ...)
{
    return 0;
}

uc_err uc_hook_del(uc_engine *uc, uc_hook hh)
{
    return 0;
}

uc_err uc_mem_map(uc_engine *uc, uint64_t address, size_t size, uint32_t perms)
{
    return 0;
}

uc_err __cdecl uc_mem_map_ptr(uc_engine *uc, uint64_t address, size_t size, uint32_t perms, void *ptr)
{
    return 0;
}

uc_err uc_mem_unmap(uc_engine *uc, uint64_t address, size_t size)
{
    return 0;
}

uc_err uc_mem_protect(uc_engine *uc, uint64_t address, size_t size, uint32_t perms)
{
    return 0;
}

_ML_C_TAIL_
