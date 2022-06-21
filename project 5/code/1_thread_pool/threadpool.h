// function prototypes
void execute(void (*somefunction)(void *p), void *p);
int pool_submit(void (*somefunction)(void *p), void *p);
void *worker(void *param);
void pool_init(int num);
void pool_shutdown(void);
