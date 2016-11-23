
#include <stdlib.h>

// Global constants
const char BIT_BOTTOM       = 2;            // Fin de carrera Inferior IN pin2
const char BIT_TOP          = 3;            // Fin de carrera Superior IN pin3
const char BIT_DIRECTION    = 4;            // Bobina del relay inversor OUT pin4
const char BIT_MOVE         = 5;            // Bobina del releay on/off OUT pin5
const char BIT_FAN          = 6;            // Ventilador sopla hojas OUT pin6
const char BIT_RIGHT_LED    = 7;            // Led iluminacion interna derecha OUT pin7
const char BIT_LEFT_LED     = 8;            // Led iluminacion interna izquiera OUT pin8

const char PLATE_STATE_ERROR    = 0;        // Error en el pratternboxx
const char PLATE_STATE_UP       = 1;        // Pratterboxx arriba
const char PLATE_STATE_DOWN     = 2;        // Pratterboxx abajo
const char PLATE_STATE_MIDDLE   = 3;        // Pratterboxx en el medio

const char PLATE_DIRECTION_UP   = 1;        // Dirección del pratterboxx arriba
const char PLATE_DIRECTION_DOWN = 2;        // Dirección del pratterboxx abajo

const char CYCLE_STATE_ERROR          = 0;  // Error en el ciclo
const char CYCLE_STATE_IDLE           = 1;  // En espera
const char CYCLE_STATE_GOING_HOME     = 2;  // Camino a casa
const char CYCLE_STATE_PAGE_FIX       = 3;  //
const char CYCLE_STATE_SCANNING       = 4;  //
const char CYCLE_STATE_PAGE_TURN      = 5;  //

// constants for anti-error toggling trick timing during page turn.
const unsigned long PAGE_TURN_FIRST_TOGGLE    = 7000; 
const unsigned long PAGE_TURN_SECOND_TOGGLE   = 9000; 


// Global vars
char            plate_current_direction;
char            cycle_current_status;
unsigned long   page_turn_start_time;
char            page_turn_toggled_times;

void setup( )
{
    Serial.begin(9600);
    cycle_notify( "setup", "start");
    
    pinMode(BIT_BOTTOM      , INPUT);      // Fin de carrera Inferior IN pin2
    pinMode(BIT_TOP         , INPUT);      // Fin de carrera Superior IN pin3
    pinMode(BIT_DIRECTION   , OUTPUT);     // Bobina del relay inversor OUT pin4
    pinMode(BIT_MOVE        , OUTPUT);     // Bobina del releay on/off OUT pin5
    pinMode(BIT_FAN         , OUTPUT);     // Ventilador sopla hojas OUT pin6
    pinMode(BIT_RIGHT_LED   , OUTPUT);     // Led iluminacion interna derecha OUT pin7
    pinMode(BIT_LEFT_LED    , OUTPUT);     // Led iluminacion interna izquierda OUT pin8
    
    plate_go_home();                       // you are drunk
    
    cycle_notify( "setup", "end");
}

char plate_check_state()
{
    char bottom = digitalRead( BIT_BOTTOM );
    char top    = digitalRead( BIT_TOP );
    char ret    = PLATE_STATE_ERROR;
    
    if (bottom && !top)
    {
        ret = PLATE_STATE_DOWN;
        cycle_notify( "plate", "state_down");
    }
    else if (!bottom && top)
    {
        ret = PLATE_STATE_UP;
        cycle_notify( "plate", "state_up");
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
    cycle_notify( "plate", "start_moving");
}

char plate_toggle( )
{
    char l_toggle = plate_is_moving();
    digitalWrite( BIT_MOVE, l_toggle ? LOW : HIGH );
    cycle_notify( "plate", "toggle");
    return !l_toggle;
}

char plate_toggle_direction( )
{
    char l_dir = plate_current_direction == PLATE_DIRECTION_DOWN ? HIGH : LOW;
    digitalWrite( BIT_DIRECTION, l_dir );
    plate_current_direction = l_dir == HIGH ? PLATE_DIRECTION_UP : PLATE_DIRECTION_DOWN;
    cycle_notify( "plate", "toggle_direction");
    return l_dir;
}

void plate_go_home()
{
    /*
    // si se lo manda a home y ya está en home avisa y no hace nada 
    char l_state = plate_check_state();
    if ( PLATE_STATE_UP == l_state )
    {
      plate_stop_moving();
      cycle_notify( "plate", "already_homed"); 
      return;
    }
    */
    plate_stop_moving();
    plate_current_direction = PLATE_DIRECTION_DOWN;
    plate_toggle_direction();
    cycle_change_status( CYCLE_STATE_GOING_HOME );
    plate_start_moving();
    cycle_notify( "plate", "go_home");
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
    cycle_change_status( CYCLE_STATE_PAGE_TURN );
    plate_start_moving();
}

void cycle_change_status ( char status )
{
    cycle_current_status = status;
    cycle_notify( "cycle", "change_status: " + String( status, DEC ) );
} 

void cycle_check_page_turn()
{
    char l_turn1 = millis() > (page_turn_start_time + PAGE_TURN_FIRST_TOGGLE)  && 1 > page_turn_toggled_times;
    char l_turn2 = millis() > (page_turn_start_time + PAGE_TURN_SECOND_TOGGLE) && 2 > page_turn_toggled_times;
    
    if ( l_turn1 || l_turn2 )
    {
        cycle_notify( "cycle", "page_turning " +  String( page_turn_toggled_times, DEC ) );
        page_turn_toggled_times++;
        plate_stop_moving();
        plate_toggle_direction();
        plate_start_moving();
        cycle_notify( "cycle", "page_turning ok");
    }
    
}

void cycle_parse_command( String command )
{
    command.trim();
    if ( command == "\1" || command == "\0" || command == "" )
    {
        return;
    }
    
    cycle_notify( "cycle", "ack: " + command );
    
    if ( command.equals("GO_DOWN") )
    {
        cycle_notify( "cycle", "command: GO_DOWN.");
        plate_stop_moving();
        plate_current_direction = PLATE_DIRECTION_DOWN;
        cycle_change_status( CYCLE_STATE_SCANNING );
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
    else if ( command.equals("LED_RIGHT") )
    {
        plate_led( BIT_LEFT_LED , LOW  );
        plate_led( BIT_RIGHT_LED, HIGH );
    }
    else if ( command.equals("LED_LEFT") )
    {
        plate_led( BIT_RIGHT_LED , LOW  );
        plate_led( BIT_LEFT_LED, HIGH );
    }
    else if ( command.equals("LED_OFF") )
    {
        plate_led( BIT_RIGHT_LED , LOW  );
        plate_led( BIT_LEFT_LED, LOW );
    }
    else
    {
        plate_set_idle();
        cycle_notify( "error", "Unrecognized command (" + command + ")" );
    }
    
}

void cycle_notify( String stat, String desc )
{
    Serial.println("{ \"action\" : \"" + stat + "\", \"description\" : \"" + desc + "\" }");
    Serial.flush();
}

void plate_led( char bit_led, char value )
{
    digitalWrite( bit_led, value );
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
        Serial.readString();
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
