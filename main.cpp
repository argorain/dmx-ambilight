#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xutil.h>
#include <cstdio>
#include <ftdi.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <list>

#define AVGS 6 //How many samples from history is going to be used

using namespace std;

int main()
{
    Display *display = XOpenDisplay(NULL);
    Window root = DefaultRootWindow(display);

    XWindowAttributes gwa;

    XGetWindowAttributes(display, root, &gwa);
    int width = gwa.width;
    int height = gwa.height;

    unsigned char c = 0;
    struct ftdi_context ftdic;
    unsigned char buffer[513];
    
    // Create history lists
    list<unsigned char> redLongA;
    list<unsigned char> greenLongA;
    list<unsigned char> blueLongA;
    

    memset(buffer, 0, 513);

    while(1) {

        // Initialize context for subsequent function calls
        ftdi_init(&ftdic);

        // Open FTDI device based on FT232R vendor & product IDs 
        if(ftdi_usb_open(&ftdic, 0x0403, 0x6001) < 0) {
            fprintf(stderr, "Can't open device.\n");
            exit(-1);
        }

        // Read out FTDIChip-ID of R type chips
        if (ftdic.type == TYPE_R)
        {
            unsigned int chipid;
            printf("ftdi_read_chipid: %d\n", ftdi_read_chipid(&ftdic, &chipid));
            printf("FTDI chipid: %X\n", chipid);
        }

        // Setup FTDI for DMX
        int st = ftdi_set_line_property(&ftdic, BITS_8, STOP_BIT_2, NONE);
        st += ftdi_setflowctrl(&ftdic, SIO_DISABLE_FLOW_CTRL);
        if(st != 0) {
            fprintf(stderr, "Can't setup device.\n");
            exit(-2);
        }


        buffer[1] = 2; //DMX space starts at byte #1 and ends at #512
        //My light is on address 1 and require set first byte to 2

        // Check screen and send it to light forever.
        while(1) {

            XImage *image = XGetImage(display,root, 0,0 , width,height,AllPlanes, ZPixmap);

            unsigned long red_mask = image->red_mask;
            unsigned long green_mask = image->green_mask;
            unsigned long blue_mask = image->blue_mask;

            unsigned long redA=0, greenA=0, blueA=0;

            for (int x = 0; x < width; x++)
                for (int y = 0; y < height ; y++)
                {
                    unsigned long pixel = XGetPixel(image,x,y);

                    unsigned char blue = pixel & blue_mask;
                    unsigned char green = (pixel & green_mask) >> 8;
                    unsigned char red = (pixel & red_mask) >> 16;

                    redA += red;
                    greenA += green;
                    blueA += blue;
                }

            XDestroyImage(image);

            // Image average
            redA = redA/(width*height);
            blueA = blueA/(width*height);
            greenA = greenA/(width*height);

            // Store those values for averaging to avoid flickering
            redLongA.push_front(redA);
            blueLongA.push_front(blueA);
            greenLongA.push_front(greenA);
            redA = blueA = greenA = 0;  

            // Sum history
            for (list<unsigned char>::iterator it=redLongA.begin(); it != redLongA.end(); ++it)
                redA += *it;
            
            for (list<unsigned char>::iterator it=greenLongA.begin(); it != greenLongA.end(); ++it)
                greenA += *it;
            
            for (list<unsigned char>::iterator it=blueLongA.begin(); it != blueLongA.end(); ++it)
                blueA += *it;
           
            // Average history
            redA = redA/redLongA.size();
            greenA = greenA/greenLongA.size();
            blueA = blueA/blueLongA.size();

            // Remove oldests
            if(redLongA.size() >= AVGS) {
                redLongA.pop_back();
                greenLongA.pop_back();
                blueLongA.pop_back();
            }

            // My light have RGB channels on address 4,5,6.
            // There is cubic function with normalization to make colors more vibrant
            // Formula is this: result = (average^N)/(255^(N-1))
            buffer[4] = (redA*redA*redA)/(255*255);
            buffer[5] = (greenA*greenA*greenA)/(255*255);
            buffer[6] = (blueA*blueA*greenA)/(255*255);

            // If color is black, turn off light (mine is not turned off when RGB=0)
            if(redA+greenA+blueA == 0)
                buffer[1] = 0; //Turn off light completely
            else
                buffer[1] = 2; //Turn of again

            // DMX
            //Send break
            st += ftdi_set_baudrate(&ftdic, 120000); 
            st += ftdi_write_data_set_chunksize(&ftdic, 1);
            ftdi_write_data(&ftdic, buffer, 1); 

            //Send 
            st += ftdi_set_baudrate(&ftdic, 250000);
            st += ftdi_write_data_set_chunksize(&ftdic, 513);
            if(ftdi_write_data(&ftdic, buffer, 513)==-666)
                st += 1;

            if(st > 0) {
                fprintf(stderr, "Error happened. Resetting.\n");
                st = 0;
                break; //Perform device reset.
            }

            usleep(35000); //DMX Pipe requires frequency less than 30 Hz
        }

        ftdi_usb_close(&ftdic);
        ftdi_free(&ftdic);
    }
    exit(0);
}
