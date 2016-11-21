
var tmpurl = new URL(location.href);
scanner = {};

scanner.api = {
    acquire : function ( device )
        {
            console.log("acquire");
            $.ajax({
                url:    "/api/acquire/" + device ,
                dataType: "json"
            }).done(
                function(resp) { console.log("done", resp); }
            ); 
        }
};

scanner.conexion = {
   historia : [],
   conexion : {
       open     : null,
       id       : null,
       socket   : null
   },
   uri      : "ws://" + ((tmpurl.hostname.trim() == "") ? "localhost" : tmpurl.hostname) + ":8080/ws"
};


scanner.conexion.open = function ( uri ) 
{
    
    if ( uri === undefined || uri === null)
    {
        uri = scanner.conexion.uri;
    }
    
    var u = new URL(uri);
    
    var tmp = function() {
        var websocket = new WebSocket( uri );
        console.log("scanner.conexion.open",uri);
        websocket.onopen = function(evt) { 
            
            scanner.conexion.socket.send('{ "command" : "HOLA" }');
            
        };
        
        websocket.onclose = function(evt) { 
            console.log(evt); 
        };
        
        websocket.onmessage = function(evt) { 
            
            scanner.parse_response(evt.data);
            
            //websocket.close();
        };
        
        websocket.onerror = function(evt) { 
            console.log(evt); 
        };
        
        scanner.conexion.socket = websocket;
    };
    
    tmp();
};

scanner.parse_response = function( resp )
{
    console.log("response", resp)
    var parsed  = {};
    var e       = "";
    try
    {
        parsed = JSON.parse( resp );
    }
    catch( e )
    {
        console.log( "error parsing: ", $e );
    }
    
    if ( parsed.command !== undefined )
    {
        
        if ( parsed.command == "scan" )
        {
            console.log("taking picture from device " + parsed.device);
            scanner.api.acquire( parsed.device );
        }
    }
};
