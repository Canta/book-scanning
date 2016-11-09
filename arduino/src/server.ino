
#include <stdlib.h>

const char BIT_BOTTOM       = 2;
const char BIT_TOP          = 3;
const char BIT_DIRECTION    = 4;
const char BIT_MOVE         = 5;
const char BIT_FAN          = 6;

const char PLATE_STATE_ERROR    = 0;
const char PLATE_STATE_UP       = 1;
const char PLATE_STATE_DOWN     = 2;
const char PLATE_STATE_MIDDLE   = 3;

const char CYCLE_STATE_ERROR          = 0;
const char CYCLE_STATE_GOING_UP       = 1;
const char CYCLE_STATE_GOING_DOWN     = 2;
const char CYCLE_STATE_PAGE_FIX       = 3;
const char CYCLE_STATE_SCANNING       = 4;
const char CYCLE_STATE_PAGE_TURN      = 5;

void setup( )
{
    pinMode(BIT_BOTTOM      , INPUT);
    pinMode(BIT_TOP         , INPUT);
    pinMode(BIT_DIRECTION   , OUTPUT);
    pinMode(BIT_MOVE        , OUTPUT);
    pinMode(BIT_FAN         , OUTPUT);

}

char check_plate_state()
{
    char bottom = digitalRead( BIT_BOTTOM );
    char top    = digitalRead( BIT_TOP );
    char ret    = PLATE_STATE_ERROR;
    
    if (bottom && !top)
    {
        ret = PLATE_STATE_DOWN;
    }
    else if (!bottom && top)
    {
        ret = PLATE_STATE_UP;
    }
    else if (!bottom && !top)
    {
        ret = PLATE_STATE_MIDDLE;
    }
    
    return ret;
}



void loop( )
{
    
}
