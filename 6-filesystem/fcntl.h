#define O_RDONLY  0x000
#define O_WRONLY  0x001
#define O_RDWR    0x002
#define O_CREATE  0x200

// Exercise 8.
// Added.
#define O_TRUNC   0x010
#define O_APPEND  0x020

#define SEEK_SET  0x000
#define SEEK_CUR  0x001
#define SEEK_END  0x002