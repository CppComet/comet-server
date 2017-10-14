<!DOCTYPE HTML>
<html>
<head>
        <meta charset=utf-8>
        <!-- add libs -->
        <script language="JavaScript" type="text/javascript" src="//comet-server.com/js/lib/jquery.min.js" ></script>
        <script src="//comet-server.com/CometServerApi.js" type="text/javascript"></script>
</head>
<body>

<style>

hr{
    clear:both;
}

.hide{
    display:none;
}

#log{
    color:#000
}

.status-ok{
    color:#272;
    font-weight:bold;
}

.status-warn{
    color:#772;
    font-weight:bold;
}

.status-error{
    color:#F22;
    font-weight:bold;
}

.note{
    background:#eee;
    padding:1px 10px;
    margin:5px;
    border-radius:5px;
    text-align:justify;
}
</style>

<h3>CppComet tests</h3>
 
<div class="root"> 
    <hr>
    <div><span id="userId"></span> - use this id to send message</div>
    <hr>
    <div class="status">
        We are waiting for authorization on the comet server
    </div>
    <hr>
    <input type="text" placeholder="Message" id="text"> - Personal text message for user<br>
    <input type="text" placeholder="user_id" id="to_user_id"> - Identificator of user who must will receive the message<br>
    <button onclick="send($('#to_user_id').val(), $('#text').val())">Send</button>
    <hr>
    <div id="log"></div>
</div>
 
<script type="text/javascript">

function log()
{
    console.log(arguments);
    $("#log").append(JSON.stringify(arguments)+"<hr>");
}

function status(text, type)
{
    console.log(arguments);
    $(".status").html("<span class='status-"+type+"'>"+text+"</span>");
}

// Authorization on comets server
function runTest(name)
{ 
    return $.when($.ajax({
        url: "https://comet-server.com/doc/CE-api-tests/test.php?count=1&test="+name,
        type: "GET",
        dataType:'json',
    })).always(function(res){
        log(res)
    }).promise();
}


// Authorization on comets server
function auth()
{
    var user_id = 0;
    
    try{
        user_id = localStorage['user_id']
    }catch(e){ 
        user_id = Math.floor(Math.random()*1000000)
    } 
     
    if(!user_id)
    {
        user_id = Math.floor(Math.random()*1000000)
        localStorage['user_id'] = user_id
    }

    $("#userId").text("USER_ID = "+user_id);
    log("Request authorization for user_id="+user_id)
    var user_key = "PassForUser"+user_id

    //  The first step – it is sending user’s ID and a random hash in your system to the comet-server via CometQL.
    //  auth.php ( https://github.com/CppComet/auth-example/blob/master/auth.php )
    return $.when($.ajax({
        url: "https://comet-server.com/doc/CometQL/auth-chat/auth.php?user_id="+user_id+"&user_key="+user_key+"&t="+Math.floor(Math.random()*1000000),
        type: "GET",
        dataType:'json',
    })).always(function(res){
        status("We are waiting for connection to the comet server", "warn");
        
        // The function start accepts connection settings and opens new connection. 
        // https://comet-server.com/wiki/doku.php/en:comet:javascript_api#connection_with_server
        cometApi.start({
            node: "test.comet.su:8087",
            dev_id: 0,
            user_id:user_id,
            user_key:user_key,
        })
    }).promise();
}
 
cometApi.onAuthSuccess(function()
{
    log("Authorization on the comet server is complete")
    status("Authorization on the comet server is complete", "ok");
    cometApi.getTrackPipeUsers("track_authExample", function(){
        log(e)
    })
})

cometApi.onAuthFalill(function()
{
    log("Authorization on the comet server is fail")
    status("Authorization on the comet server is fail", "error");
    cometApi.getTrackPipeUsers("track_authExample", function(){
        log(e)
    })
})

cometApi.subscription("msg.message", function(e){ 
    log(e)
})

cometApi.subscription("track_authExample.subscription", function(msg)
{
    log(msg)
    // Обработка события что кто то зашёл на сайт и подписался на канал track_online
});	
cometApi.subscription("track_authExample.unsubscription", function(msg)
{
    log(msg)
    // Обработка события что кто то покинул сайт и/или отписался от канала track_online
});	

auth();

</script>
</body>
</html>