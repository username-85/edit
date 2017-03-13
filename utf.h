/* some macros that i saw in sandy text editor */

#ifndef UTF_H 
#define UTF_H 

#define UTF_BUF_SIZE 7

#define UTF8LEN(ch)   ((unsigned char)ch>=0xFC ? 6 : \
                      ((unsigned char)ch>=0xF8 ? 5 : \
                      ((unsigned char)ch>=0xF0 ? 4 : \
                      ((unsigned char)ch>=0xE0 ? 3 : \
                      ((unsigned char)ch>=0xC0 ? 2 : 1)))))

#define ISASCII(ch)   ((unsigned char)ch < 0x80)

#define ISFILL(ch)    (!ISASCII(ch) && (unsigned char)ch<=0xBF)

#endif /* UTF_H */
