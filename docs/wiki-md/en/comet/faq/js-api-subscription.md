
# How to take a message out of the channel in JavaScript?


<file html index.html> 
<!DOCTYPE HTML>
<html>
<head>
    <!-- Add libs -->
    <meta charset=utf-8>
    <script src="//comet-server.com/CometServerApi.js" type="text/javascript"></script>
    <script src="jquery.min.js" type="text/javascript"></script>
</head>
<body>
    
    <input id="pipeName" type="text" placeholder="Custom channel name" >
    <input type="button" value="Subscribe" onclick="subscription();" >
    <input type="button" value="Clear message list" onclick="$('#msgHolder').html('');" >
    
    <div id="pipeNames"></div>
    
    <div id="msgHolder"></div>
     
    <script type="text/javascript">
              
    $(document).ready(function()
    { 
        /** 
         * Connection to the comet server. For the ability to take commands.
         * dev_id is your developer's public identifier
         */
        CometServer().start({dev_id:15 })
    })
    
    function subscription()
    {
        var pipeName = $("#pipeName").val();
        $("#pipeName").val('');
        $("#pipeNames").html( pipeName + "<br>" + $("#pipeNames").html())
        
        /** 
         * We are subscribed to receive a message from pipe `pipeName`
         */
        CometServer().subscription(pipeName, function(event)
        {
            console.log(["event", event])
            $("#msgHolder").html( JSON.stringify(event) + "<hr>" + $("#msgHolder").html())
        })
    }
        

    </script>
</body>
</html>
</file>