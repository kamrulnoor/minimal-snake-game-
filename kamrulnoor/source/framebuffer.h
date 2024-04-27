extern unsigned int width, height, pitch;
extern unsigned char* framebuffer;
void init_framebuffer();
void drawPixel(int x, int y, unsigned int color);
void fillScreen(unsigned int color);
void drawRect(int x1, int y1, int x2, int y2, unsigned int color, int fill);

void drawImage(const unsigned char *img, int width, int height, int offx, int offy);

