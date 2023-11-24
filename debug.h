



#ifndef  __DEBUG_H__
#define  __DEBUG_H__




#define LOGE(x...) do { KLOG_ERROR("events", x); } while (0)
#define LOGI(x...) do { KLOG_INFO("events", x); } while (0)
#define LOGV(x...) do { KLOG_DEBUG("events", x); } while (0)



#define BITS_PER_LONG (sizeof(unsigned long) * 8)
#define BITS_TO_LONGS(x) (((x) + BITS_PER_LONG - 1) / BITS_PER_LONG)

#define test_bit(bit, array) \
    ((array)[(bit)/BITS_PER_LONG] & (1 << ((bit) % BITS_PER_LONG)))




extern int debug_msg(const char *format, ...);


#endif
      
     







	
























