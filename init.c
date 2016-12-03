

// _init is required by the linker (ld) but not required when linked using cc (gcc),
// it is called by __libc_init_array. I'm not sure what this function should do, but
// it only seems to be required for C++ code (constructors/destructors).

#if 1
void _init(void)
{
}
#endif
