
#include <stdlib.h>

// Global constants
const char BIT_BOTTOM       = 2;
const char BIT_TOP          = 3;
const char BIT_DIRECTION    = 4;
const char BIT_MOVE         = 5;
const char BIT_FAN          = 6;

const char PLATE_STATE_ERROR    = 0;
const char PLATE_STATE_UP       = 1;
const char PLATE_STATE_DOWN     = 2;
const char PLATE_STATE_MIDDLE   = 3;

const char PLATE_DIRECTION_UP   = 1;
const char PLATE_DIRECTION_DOWN = 2;

const char CYCLE_STATE_ERROR          = 0;
const char CYCLE_STATE_IDLE           = 1;
const char CYCLE_STATE_GOING_HOME     = 2;
const char CYCLE_STATE_PAGE_FIX       = 3;
const char CYCLE_STATE_SCANNING       = 4;
const char CYCLE_STATE_PAGE_TURN      = 5;

// constants for anti-error toggling trick timing during page turn.
const unsigned long PAGE_TURN_FIRST_TOGGLE    = 4000; 
const unsigned long PAGE_TURN_SECOND_TOGGLE   = 7000; 


// Global vars
char            plate_current_direction;
char            cycle_current_status;
unsigned long   page_turn_start_time;
char            page_turn_toggled_times;

void setup( )
{
    Serial.begin(9600);
    
    pinMode(BIT_BOTTOM      , INPUT);
    pinMode(BIT_TOP         , INPUT);
    pinMode(BIT_DIRECTION   , OUTPUT);
    pinMode(BIT_MOVE        , OUTPUT);
    pinMode(BIT_FAN         , OUTPUT);
    
    plate_go_home();
    
    Serial.println('{ action : "setup" }');
}

char plate_check_state()
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
    else
    {
        plate_set_idle();
        cycle_notify( "error", "Plate is both at top and bottom.");
    }
    
    return ret;
}

char plate_is_moving( )
{
    return digitalRead( BIT_MOVE );
}

void plate_stop_moving()
{
    digitalWrite( BIT_MOVE, LOW  );
    delay( 500 );
}

void plate_start_moving()
{
    digitalWrite( BIT_MOVE, HIGH );
}

char plate_toggle( )
{
    char l_toggle = plate_is_moving();
    digitalWrite( BIT_MOVE, l_toggle ? LOW : HIGH );
    return !l_toggle;
}

char plate_toggle_direction( )
{
    char l_dir = plate_current_direction == PLATE_DIRECTION_DOWN ? HIGH : LOW;
    digitalWrite( BIT_DIRECTION, l_dir );
    plate_current_direction = l_dir == HIGH ? PLATE_DIRECTION_UP : PLATE_DIRECTION_DOWN;
    return l_dir;
}

void plate_go_home()
{
    plate_stop_moving();
    plate_current_direction = PLATE_DIRECTION_UP;
    cycle_change_status( CYCLE_STATE_GOING_HOME );
    plate_start_moving();
}

void plate_set_idle()
{
    plate_stop_moving();
    cycle_change_status( CYCLE_STATE_IDLE );
}

void cycle_page_turn()
{
    page_turn_start_time    = millis();
    page_turn_toggled_times = 0;
}

void cycle_change_status ( char status )
{
    cycle_current_status = status;
    cycle_notify( "change_status", (String) status );
} 

void cycle_check_page_turn()
{
    char l_turn1 = millis() < page_turn_start_time + PAGE_TURN_FIRST_TOGGLE  && 1 > page_turn_toggled_times;
    char l_turn2 = millis() < page_turn_start_time + PAGE_TURN_SECOND_TOGGLE && 2 > page_turn_toggled_times;
    
    if ( l_turn1 || l_turn2 )
    {
        page_turn_toggled_times++;
        plate_stop_moving();
        plate_toggle_direction();
        plate_start_moving();
    }
    
}

void cycle_parse_command( String command )
{
    command.trim();
    if ( command.equals("GO_DOWN") )
    {
        plate_stop_moving();
        plate_current_direction = PLATE_DIRECTION_DOWN;
        plate_start_moving();
    }
    else if ( command.equals("PAGE_TURN") )
    {
        plate_stop_moving();
        cycle_page_turn();
    }
    else if ( command.equals("GO_HOME") )
    {
        plate_stop_moving();
        plate_go_home();
    }
    else if ( command.equals("STOP") )
    {
        plate_set_idle();
    }
    else
    {
        plate_set_idle();
        cycle_notify( "error", "Unrecognized command (" + command + ")" );
    }
    
}

void cycle_notify( String stat, String err )
{
    Serial.println('{ action : "' + stat + '", description : "' + err + '" }');
}

void loop( )
{
    // If the scanner is idle, it listens to commands.
    // Otherwise, it checks for different cycle status.
    if ( CYCLE_STATE_IDLE == cycle_current_status )
    {
        cycle_parse_command( Serial.readString() );
    }
    else
    {
        // First check is plate moving or static.
        if ( plate_is_moving() )
        {
            char l_state = plate_check_state();
            
            if ( PLATE_STATE_DOWN == l_state)
            {
                if ( PLATE_DIRECTION_DOWN == plate_current_direction )
                {
                    // Bottom reached.
                    plate_set_idle();
                    plate_toggle_direction();
                }
            }
            else if ( PLATE_STATE_UP == l_state )
            {
                if ( PLATE_DIRECTION_UP == plate_current_direction )
                {
                    // top reached.
                    plate_set_idle();
                    plate_toggle_direction();
                }
            }
            else if ( PLATE_STATE_MIDDLE == l_state)
            {
                if ( CYCLE_STATE_PAGE_TURN == cycle_current_status )
                {
                    cycle_check_page_turn();
                }
            }
        }
        else
        {
            // Dunno yet. ¯\_(ツ)_/¯ 
            // Placeholder.
        }
    }
}
