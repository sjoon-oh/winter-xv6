// Semaphore for processes
struct semaphore {
  int ori_res; // Original number of resources
  int res; // Current number of resources
  struct spinlock lk; // spinlock protecting semaphore
};

