#include "multi_context_but_c_api.h"
#include "awc2/C/context.h"
#include <awc2/C/awc2.h>
#include <util2/C/marker4.h>
#include <stdio.h>


static void custom_mousebutton_callback(AWC2User_callback_mousebutton_struct const* data);
static void custom_winfocus_callback(AWC2User_callback_winfocus_struct const* data);
static void custom_winsize_callback(AWC2User_callback_winsize_struct const*);
static void custom_cursor_callback(AWC2User_callback_mousecursor_struct const* data);


int awc2_c_api_multi_context()
{
    awc2init();

    u8 id[3] = {
        awc2createContext(),
        awc2createContext(),
        awc2createContext()
    };
    AWC2ContextDescriptor ctxdesc;
    AWC2WindowDescriptor  windesc;


    awc2WindowDescriptorDefault(&windesc);
    for(u8 i = 0; i < 3; ++i) {
        ctxdesc  = (AWC2ContextDescriptor){ 
            .id = id[i], 
            .reserved  = {0},
            .winWidth  = 640, 
            .winHeight = 480,
            .winDesc   = windesc
        };
        awc2initializeContext(&ctxdesc);
        awc2setContextUserCallbackMouseButton  (id[i], &custom_mousebutton_callback);
        awc2setContextUserCallbackWindowFocus  (id[i], &custom_winfocus_callback);
        awc2setContextUserCallbackWindowSize   (id[i], &custom_winsize_callback);
        awc2setContextUserCallbackMousePosition(id[i], &custom_cursor_callback);
    }

    AWC2ContextVector ctxvec;
    awc2getActiveContextList(&ctxvec);
    while(ctxvec.actualSize != 0)
    {
        for(u8 i = 0; i < ctxvec.actualSize; ++i) {
            awc2setCurrentContext(ctxvec.id[i]);
            awc2newframe();
            awc2begin();


            awc2end();
        }
        awc2getActiveContextList(&ctxvec);
    }


    awc2destroy();
    return 1;
}


static void custom_mousebutton_callback(AWC2User_callback_mousebutton_struct const* data)
{
    printf("[%u] called custom_mousebutton_callback!\n", data->context_id);
    return;
}


static void custom_winfocus_callback(AWC2User_callback_winfocus_struct const* data)
{
    printf("[%u] called custom_winfocus_callback!\n", data->context_id);
    return;
}


static void custom_winsize_callback(AWC2User_callback_winsize_struct const* data)
{
    printf("[%u] called custom_winsize_callback!\n", data->context_id);
    return;
}


static void custom_cursor_callback(AWC2User_callback_mousecursor_struct const* data)
{
    printf("[%u] called custom_cursor_callback!\n", data->context_id);
    return;
}

