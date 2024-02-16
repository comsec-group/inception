#define IBPB_COST_IOC_HELLO    0x10000
#define IBPB_COST_IOC_MEASSURE 0x20000
#define IBPB_COST_IOC_DRY      0x30000
#define PROC_IBPB_COST         "ibpb_cost"

struct ibpb_cost_ioc_msg {
    unsigned long result;
    // whatever you want
};
