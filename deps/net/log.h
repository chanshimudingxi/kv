#ifndef LOG_H_
#define LOG_H_

#define LOG_DEBUG(fmt, args...) printf("[DEBUG]%lu:%s:%d(%s): " fmt "\n",time(NULL), __FILE__, __LINE__, __FUNCTION__ , ##args)
#define LOG_INFO(fmt, args...) printf("[INFO]%lu:%s:%d(%s): " fmt "\n",time(NULL), __FILE__, __LINE__, __FUNCTION__ , ##args)
#define LOG_WARN(fmt, args...) printf("[WARN]%lu:%s:%d(%s): " fmt "\n",time(NULL), __FILE__, __LINE__, __FUNCTION__ , ##args)
#define LOG_ERROR(fmt, args...) printf("[ERROR]%lu:%s:%d(%s): " fmt "\n",time(NULL), __FILE__, __LINE__, __FUNCTION__ , ##args)

#endif