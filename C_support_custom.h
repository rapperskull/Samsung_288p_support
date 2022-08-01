#define F_SEEK_UP 4
#define F_SEEK_DOWN -4

void LOG(const char *fmt, ...);
#define clog(...) LOG("["LIB_NAME"] " __VA_ARGS__)
#define log_found(fname, addr) if(fname && addr) \
  clog("Found %s at: 0x%08x\n", fname, addr);

void* get_dlsym_addr(void *h, char *fn_name);
void *check_is_LDR_RD(unsigned int *addr);
int is_function_start(unsigned int *addr);
unsigned int * find_function_start(unsigned int *addr);
unsigned int * find_next_function_start(unsigned int *addr);
unsigned int* find_function_end(unsigned int*addr);
unsigned int calculate_branch(unsigned int from, unsigned int to, unsigned int b_type);
unsigned int calculate_branch_addr(unsigned int *from);
unsigned int* find_func_by_string(void *h, char *fName, char *f_string, int seek_step, int offset);
unsigned int* find_nth_func(unsigned int *seek_start, int funcNumber);
unsigned int* find_nth_func_by_string(void *h, char *fName, char *f_string, int seek_step, int offset, int funcNumber);
unsigned int* find_nth_func_from_export(void *h, char *fName, int funcNumber);
